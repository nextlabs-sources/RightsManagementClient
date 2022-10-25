


#include <Windows.h>
#include <assert.h>
#include <winternl.h>

#include <nudf\shared\rbtree.h>
#include "nxrmdrv.h"

#include "corehlp.hpp"


typedef struct _COREHLP_HANDLE{
	HANDLE			h;
	NXRMSECTION		s;
}COREHLP_HANDLE,*PCOREHLP_HANDLE;

static NXRMRECORD* WINAPI AllocRecord(PCOREHLP_HANDLE phTrans);
static void WINAPI FreeRecord(NXRMRECORD *record);


CCoreObject::CCoreObject()
{
}

CCoreObject::~CCoreObject()
{
    Disconnect();
}

int CCoreObject::Connect()
{
    int nRet = 0;
    PCOREHLP_HANDLE phTrans = NULL;

	BOOL	bRet = TRUE;
	DWORD	dwbsrt = 0;
	
	NTSTATUS	status = 0;
	UCHAR		io_status[64];

	memset(io_status,0,sizeof(io_status));

    phTrans = (COREHLP_HANDLE*)malloc(sizeof(COREHLP_HANDLE));
    if(NULL == phTrans) {
        return ERROR_OUTOFMEMORY;
    }

    phTrans->h = ::CreateFileW( NXRMDRV_WIN32_DEVICE_NAME,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    if(phTrans->h == INVALID_HANDLE_VALUE) {
        nRet = GetLastError();
        free(phTrans);
        phTrans = NULL;
        return nRet;
    }

    bRet = DeviceIoControl( phTrans->h,
                            IOCTL_NXRMDRV_MAP_RM_CLIENT_SECTION,
                            NULL,
                            0,
                            &phTrans->s,
                            sizeof(phTrans->s),
                            &dwbsrt,
                            NULL);
    if(!bRet) {
        nRet = GetLastError();
        CloseHandle(phTrans->h);
        free(phTrans);
        phTrans = NULL;
        return nRet;
    }

    assert(NULL != phTrans);
    assert(NULL != phTrans->h);
    _h = phTrans;
    return 0;
}

void CCoreObject::Disconnect()
{
	if(NULL != _h) {

		if(INVALID_HANDLE_VALUE != ((COREHLP_HANDLE*)_h)->h) {
			CloseHandle(((COREHLP_HANDLE*)_h)->h);
			((COREHLP_HANDLE*)_h)->h = NULL;
		}

        if(((COREHLP_HANDLE*)_h)->s.hsem) {
			CloseHandle(((COREHLP_HANDLE*)_h)->s.hsem);
			((COREHLP_HANDLE*)_h)->s.hsem = NULL;
		}

		free((COREHLP_HANDLE*)_h);
		_h = NULL;
	}
}

NXCONTEXT CCoreObject::SubmitRequest(ULONG type, PVOID req, ULONG length)
{
    PCOREHLP_HANDLE	phTrans = (PCOREHLP_HANDLE)_h;
	NXRMRECORD*     pRecord = NULL;	
	ULONG	        offset = 0;	
	LONG	        previouscount = 0;

    if(NULL == _h) {
        return NULL;
    }

    pRecord = AllocRecord(phTrans);
    if(NULL == pRecord) {
        return NULL;
    }

    assert(length <= NXRM_SECTION_MAX_DATA_SIZE);

    pRecord->hdr.record_flags   = 0;
    pRecord->hdr.record_type    = type;
    pRecord->length             = length;

    memcpy( pRecord->data,
            (UCHAR*)req + offset,
            min(length,NXRM_SECTION_MAX_DATA_SIZE));

    pRecord->hdr.record_state	= NXRM_RECORD_STATE_READY_FOR_ENGINE;

    if(phTrans->s.hsem) {
        ReleaseSemaphore(phTrans->s.hsem, 1, &previouscount);
    }

    return (NXCONTEXT)pRecord;
}

BOOL CCoreObject::SubmitNotify(ULONG type, PVOID req, ULONG length)
{
	NXCONTEXT context = NULL;

    context = SubmitRequest(type, req, length);
    if(NULL == context) {
        return FALSE;
    }

    InterlockedExchange(&(((NXRMRECORD*)context)->hdr.record_cancelled), 1);
    return TRUE;
}

BOOL CCoreObject::WaitForResponse(NXCONTEXT context, PVOID resp, ULONG length, ULONG *bytesret)
{
	NXRMRECORD*     pRecord = NULL;
	DWORD	        tick = GetTickCount();
    
    pRecord = (NXRMRECORD*)context;

    while(pRecord->hdr.record_state != NXRM_RECORD_STATE_READY_FOR_CLIENT)  {

        SleepEx(10,TRUE);
        if((GetTickCount() - tick) > MAX_CORE_TIMEOUT ||  !IsTransportEnabled()) {
            SetLastError(ERROR_TIMEOUT);
            return TRUE;
        }
    } 

    if(pRecord->hdr.record_state != NXRM_RECORD_STATE_READY_FOR_CLIENT) {
        //
        // record is still NOT ready for client
        //
        InterlockedExchange(&pRecord->hdr.record_cancelled, 1);
        return FALSE;
    }
    else {

        memcpy(resp, pRecord->data, min(length, pRecord->length));
        *bytesret = min(length, pRecord->length);
        //
        // free record
        //
        FreeRecord(pRecord);
        pRecord = NULL;
    }

	return TRUE;
}

BOOL CCoreObject::IsTransportEnabled()
{
    if(NULL == _h) {
        return FALSE;
    }
    return ((NXRMSECTIONHDR*)((PCOREHLP_HANDLE)_h)->s.section)->hdr.ready ? TRUE : FALSE;
}

BOOL CCoreObject::IsXxxChannelOn(const LONG channel)
{
    if(NULL == _h) {
        return FALSE;
    }
	return (((NXRMSECTIONHDR*)((PCOREHLP_HANDLE)_h)->s.section)->hdr.channel_mask & channel) ? TRUE : FALSE;
}

BOOL CCoreObject::IsProtectedProcess(DWORD pid)
{
	NXRMSECTIONHDR	*phdr = NULL;
    
    if(NULL == _h) {
        return FALSE;
    }

    phdr = (NXRMSECTIONHDR*)((PCOREHLP_HANDLE)_h)->s.section;
    if(!phdr->hdr.ready) {
        return FALSE;
    }

    if(pid == 0 || pid == 4) {
        return FALSE;
    }

    return (pid == phdr->hdr.protected_process_id) ? TRUE : FALSE;
}

LONG CCoreObject::GetRmPolicySn()
{
    if(NULL == _h) {
        return 0;
    }
    return ((NXRMSECTIONHDR*)((PCOREHLP_HANDLE)_h)->s.section)->hdr.policy_sn;
}



//
//  LOCAL ROUTINES
//

static NXRMRECORD* WINAPI AllocRecord(PCOREHLP_HANDLE phTrans)
{
	NXRMRECORD*     pRecord = NULL;
	NXRMSECTIONHDR* pHdr = NULL;

	BOOL    found = FALSE;	
	ULONG   ulCount = 0;
	ULONG   pid = GetCurrentProcessId();


	pHdr = (NXRMSECTIONHDR*)phTrans->s.section;	
    if(!pHdr->hdr.ready) {
        return NULL;
    }


    do  {

        pRecord = (NXRMRECORD*)(pHdr+1);

        do {

            if(InterlockedCompareExchange(&pRecord->hdr.record_taken,pid,0) == 0) {
                found = TRUE;
                pRecord->hdr.record_owner = pid;
                break;
            }
            else {
                pRecord	+=1;
                found = FALSE;
            }

        } while((BYTE*)pRecord + sizeof(NXRMRECORD) <= (BYTE*)pHdr + pHdr->hdr.length);

        if(found) {
            break;
        }
        else {
            ulCount++;
        }

    } while (ulCount < 3);

    return found ? pRecord : NULL;
}

static void WINAPI FreeRecord(NXRMRECORD *record)
{
	memset(&record->data,0,NXRM_SECTION_MAX_DATA_SIZE);
	memset(&record->checksum,0,sizeof(record->checksum));
	
	record->hdr.record_cancelled	= 0;
	record->hdr.record_flags		= 0;
	record->hdr.record_state		= NXRM_RECORD_STATE_FREE;
	record->hdr.record_owner		= 4;
	record->hdr.record_type			= NXRM_RECORD_TYPE_INVALID;

	record->length = 0;

	InterlockedExchange(&record->hdr.record_taken,0);
}