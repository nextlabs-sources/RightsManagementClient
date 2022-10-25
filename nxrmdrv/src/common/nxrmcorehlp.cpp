
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>

#include <windows.h>
#include <assert.h>
#include <winternl.h>
#include <winioctl.h>
#include <nudf\shared\rbtree.h>
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"


#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#endif

static NXRMRECORD* __stdcall alloc_record(PVOID section);

typedef struct _COREHLP_HANDLE{
	HANDLE			hNxrm;
	NXRMSECTION		Section;
}COREHLP_HANDLE,*PCOREHLP_HANDLE;

PVOID COREAPI init_transporter_client(void)
{
	COREHLP_HANDLE *htransporter = NULL;

	BOOL	bRet = TRUE;
	DWORD	dwbsrt = 0;
	
	NTSTATUS	status = 0;

	UCHAR		io_status[64];

	memset(io_status,0,sizeof(io_status));

	do 
	{
		htransporter = (COREHLP_HANDLE*)malloc(sizeof(COREHLP_HANDLE));

		if(!htransporter)
		{
			break;
		}

		htransporter->hNxrm = CreateFile(NXRMDRV_WIN32_DEVICE_NAME,
									     GENERIC_READ,
									     FILE_SHARE_READ,
									     NULL,
									     OPEN_EXISTING,
									     FILE_ATTRIBUTE_NORMAL,
									     NULL);

		if(htransporter->hNxrm == INVALID_HANDLE_VALUE)
		{
			free(htransporter);
			htransporter = NULL;
			break;	
		}

		bRet = DeviceIoControl(htransporter->hNxrm,
							   IOCTL_NXRMDRV_MAP_RM_CLIENT_SECTION,
							   NULL,
							   0,
							   &htransporter->Section,
							   sizeof(htransporter->Section),
							   &dwbsrt,
							   NULL);

		if(!bRet)
		{
			CloseHandle(htransporter->hNxrm);
			free(htransporter);
			htransporter = NULL;
			break;
		}

	} while(FALSE);

	return htransporter;
}

BOOL COREAPI close_transporter_client(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	if(htransporter)
	{
		if(htransporter->hNxrm)
		{
			CloseHandle(htransporter->hNxrm);
			htransporter->hNxrm = NULL;
		}

		if(htransporter->Section.hsem)
		{
			CloseHandle(htransporter->Section.hsem);
			htransporter->Section.hsem = NULL;
		}

		free(htransporter);

		htransporter = NULL;
	}

	return TRUE;
}

static NXRMRECORD* COREAPI alloc_record(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMRECORD *precord = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	BOOL found = FALSE;
	
	ULONG ulCount = 0;

	ULONG pid = GetCurrentProcessId();

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;	

	do 
	{
		if(!phdr->hdr.ready)
		{
			break;
		}

		do 
		{
			precord = (NXRMRECORD*)(phdr+1);

			do 
			{
				if(InterlockedCompareExchange(&precord->hdr.record_taken,pid,0) == 0)
				{
					found = TRUE;

					precord->hdr.record_owner = pid;
					break;
				}
				else
				{
					precord	+=1;
					found = FALSE;
				}

			} while((BYTE*)precord + sizeof(NXRMRECORD) <= (BYTE*)phdr + phdr->hdr.length);

			if(found)
			{
				break;
			}
			else
			{
				ulCount++;
			}

		} while (ulCount < 3);

		if(!found)
		{
			precord = NULL;
		}

	} while(FALSE);

	return precord;
}

void __stdcall free_record(NXRMRECORD *record)
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

NXCONTEXT COREAPI submit_request(PVOID section, ULONG type, PVOID req, ULONG length)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMRECORD *record = NULL;
	
	ULONG	offset = 0;
	
	LONG	previouscount = 0;

	htransporter = (COREHLP_HANDLE*)section;

	do 
	{
		record = alloc_record(section);

		if(!record)
		{
			break;
		}

		assert(length <= NXRM_SECTION_MAX_DATA_SIZE);

		record->hdr.record_flags	= 0;
		record->hdr.record_type		= type;
		record->length				= length;

		memcpy(record->data,
			   (UCHAR*)req + offset,
			   min(length,NXRM_SECTION_MAX_DATA_SIZE));

		record->hdr.record_state	= NXRM_RECORD_STATE_READY_FOR_ENGINE;

		if(htransporter->Section.hsem)
		{
			ReleaseSemaphore(htransporter->Section.hsem,1,&previouscount);
		}
	
	} while(FALSE);

	return (NXCONTEXT)record;
}

BOOL COREAPI submit_notify(PVOID section, ULONG type, PVOID req, ULONG length)
{
	BOOL bRet = TRUE;
	
	NXCONTEXT context = NULL;

	NXRMRECORD *record = NULL;

	do 
	{
		context = submit_request(section, type, req, length);

		if(!context)
		{
			bRet = FALSE;
			break;
		}
		
		record = (NXRMRECORD*)context;

		InterlockedExchange(&record->hdr.record_cancelled,1);

	} while (FALSE);

	return bRet;
}

BOOL COREAPI wait_for_response(NXCONTEXT context, PVOID section, PVOID resp, ULONG length, ULONG *bytesret)
{
	BOOL bRet = TRUE;
	
	NXRMRECORD *record = NULL;

	COREHLP_HANDLE	*htransporter = (COREHLP_HANDLE*)section;

	DWORD	tick = GetTickCount();

	do 
	{
		record = (NXRMRECORD*)context;
		
		while(record->hdr.record_state != NXRM_RECORD_STATE_READY_FOR_CLIENT) 
		{
			SleepEx(10,TRUE);

			if(GetTickCount() - tick > NX_MAX_TIMEOUT || 
			   !is_transporter_enabled(section))
			{
				break;
			}
		} 

		if(record->hdr.record_state != NXRM_RECORD_STATE_READY_FOR_CLIENT)
		{
			//
			// record is still NOT ready for client
			// 

			InterlockedExchange(&record->hdr.record_cancelled,1);
			bRet = FALSE;
		}
		else
		{
			memcpy(resp,record->data, min(length, record->length));

			*bytesret = min(length, record->length);

			//
			// free record
			//
			free_record(record);
		}
	
	} while(FALSE);

	return bRet;
}

BOOL COREAPI is_transporter_enabled(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR	*phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;	

	if(phdr->hdr.ready)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL COREAPI is_protected_process(PVOID section, DWORD pid)
{
	BOOL bRet = FALSE;

	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR	*phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;	

	do 
	{
		if(!phdr->hdr.ready)
		{
			break;
		}

		if(pid == 0 || pid == 4)
		{
			break;
		}

		if(pid == phdr->hdr.protected_process_id)
		{
			bRet = TRUE;
		}

	} while (FALSE);

	return bRet;
}

BOOL COREAPI is_good_version(PVOID section)
{
	BOOL bRet = FALSE;

	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR	*phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;	

	do 
	{
		if(!phdr->hdr.ready)
		{
			break;
		}

		bRet = (phdr->hdr.version >= NXRM_DRV_VERSION);

	} while (FALSE);

	return bRet;
}

PVOID COREAPI init_transporter_server(void)
{
	COREHLP_HANDLE *htransporter = NULL;

	BOOL	bRet = TRUE;
	DWORD	dwbsrt = 0;

	NXRMSECTIONHDR	*phdr = NULL;

	do 
	{
		htransporter = (COREHLP_HANDLE*)malloc(sizeof(COREHLP_HANDLE));

		if(!htransporter)
		{
			break;
		}

		htransporter->hNxrm = CreateFile(NXRMDRV_WIN32_DEVICE_NAME,
										 GENERIC_READ|GENERIC_WRITE,
										 FILE_SHARE_READ|FILE_SHARE_WRITE,
										 NULL,
										 OPEN_EXISTING,
										 FILE_ATTRIBUTE_NORMAL,
										 NULL);

		if(htransporter->hNxrm == INVALID_HANDLE_VALUE)
		{
			free(htransporter);
			htransporter = NULL;
			break;	
		}

		bRet = DeviceIoControl(htransporter->hNxrm,
							   IOCTL_NXRMDRV_MAP_RM_SERVER_SECTION,
							   NULL,
							   0,
							   &htransporter->Section,
							   sizeof(htransporter->Section),
							   &dwbsrt,
							   NULL);

		if(!bRet)
		{
			CloseHandle(htransporter->hNxrm);
			free(htransporter);
			htransporter = NULL;
			break;
		}

		phdr = (NXRMSECTIONHDR*)htransporter->Section.section;	

		phdr->hdr.version = NXRM_DRV_VERSION;

	} while(FALSE);

	return htransporter;
}

void COREAPI enable_transporter(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR	*phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	InterlockedExchange(&phdr->hdr.ready,1);
}

void COREAPI disable_transporter(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	InterlockedExchange(&phdr->hdr.ready,0);
}

BOOL COREAPI close_transporter_server(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	if(htransporter)
	{
		if(htransporter->hNxrm)
		{
			CloseHandle(htransporter->hNxrm);
			htransporter->hNxrm = NULL;
		}
		
		if(htransporter->Section.hsem)
		{
			CloseHandle(htransporter->Section.hsem);
			htransporter->Section.hsem = NULL;
		}

		if(htransporter->Section.hnotify)
		{
			CloseHandle(htransporter->Section.hnotify);
			htransporter->Section.hnotify = NULL;
		}

		free(htransporter);

		htransporter = NULL;
	}

	return TRUE;
}

NXRMRECORD* COREAPI find_pending_request(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMRECORD *record = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	BOOL	bfound = FALSE;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	record = (NXRMRECORD*)(phdr+1);

	do 
	{
		if(record->hdr.record_state == NXRM_RECORD_STATE_READY_FOR_ENGINE)
		{
			bfound = TRUE;
			break;
		}
		else
		{
			record +=1;
		}
		
	} while((BYTE*)record + sizeof(NXRMRECORD) <= (BYTE*)phdr + phdr->hdr.length);

	if(!bfound)
	{
		record = NULL;
	}
	else
	{
		record->hdr.record_state = NXRM_RECORD_STATE_IN_PROCESSING;
	}

	return record;
}

void COREAPI recycle_cancel_request(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMRECORD *record = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	record = (NXRMRECORD*)(phdr+1);

	do 
	{
		if((record->hdr.record_state == NXRM_RECORD_STATE_READY_FOR_CLIENT || record->hdr.record_state == NXRM_RECORD_STATE_BAD_RECORD) &&
		   record->hdr.record_cancelled)
		{

			//
			// found one record need to recycle
			//
			record->hdr.record_cancelled	= 0;
			record->hdr.record_flags		= 0;
			record->hdr.record_state		= NXRM_RECORD_STATE_FREE;
			record->hdr.record_owner		= 4;
			record->hdr.record_type			= NXRM_RECORD_TYPE_INVALID;

			memset(&record->data,0,NXRM_SECTION_MAX_DATA_SIZE);
			memset(&record->checksum,0,sizeof(record->checksum));

			record->length = 0;

			InterlockedExchange(&record->hdr.record_taken,0);
		}

		record +=1;

	} while((BYTE*)record + sizeof(NXRMRECORD) <= (BYTE*)phdr + phdr->hdr.length);

	return;
}

BOOL COREAPI post_response(PVOID section, NXRMRECORD *record, PVOID resp, ULONG length)
{
	BOOL bRet = TRUE;

	COREHLP_HANDLE	*htransporter = (COREHLP_HANDLE*)section;

	do 
	{
		if(record->hdr.record_cancelled)
		{
			//
			// for better performance on notify request
			//
			record->hdr.record_cancelled	= 0;
			record->hdr.record_flags		= 0;
			record->hdr.record_state		= NXRM_RECORD_STATE_FREE;
			record->hdr.record_owner		= 4;
			record->hdr.record_type			= NXRM_RECORD_TYPE_INVALID;

			memset(&record->data,0,NXRM_SECTION_MAX_DATA_SIZE);
			memset(&record->checksum,0,sizeof(record->checksum));

			record->length = 0;

			InterlockedExchange(&record->hdr.record_taken,0);

			break;

		}

		if(length > NXRM_SECTION_MAX_DATA_SIZE)
		{
			//
			// free this record because error happens
			//
			record->hdr.record_cancelled	= 0;
			record->hdr.record_flags		= 0;
			record->hdr.record_state		= NXRM_RECORD_STATE_FREE;
			record->hdr.record_owner		= 4;
			record->hdr.record_type			= NXRM_RECORD_TYPE_INVALID;

			memset(&record->data,0,NXRM_SECTION_MAX_DATA_SIZE);
			memset(&record->checksum,0,sizeof(record->checksum));

			record->length = 0;

			InterlockedExchange(&record->hdr.record_taken,0);

			bRet = FALSE;
			break;
		}

		record->hdr.record_flags  = 0;

		memcpy(record->data,resp,min(NXRM_SECTION_MAX_DATA_SIZE,length));

		record->length = min(NXRM_SECTION_MAX_DATA_SIZE,length);

		record->hdr.record_state = NXRM_RECORD_STATE_READY_FOR_CLIENT;
	
	} while(FALSE);

	return bRet;
}


HANDLE COREAPI get_transporter_semaphore(PVOID section)
{
	COREHLP_HANDLE *htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	return htransporter->Section.hsem;
}

HANDLE COREAPI get_notify_event(PVOID section)
{
	COREHLP_HANDLE *htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	return htransporter->Section.hnotify;
}


BOOL COREAPI is_xxx_channel_on(PVOID section, const LONG channel)
{
	COREHLP_HANDLE *htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	return (phdr->hdr.channel_mask & channel)?TRUE:FALSE;
}

void COREAPI set_channel_mask(PVOID section, LONG channelmask)
{
	COREHLP_HANDLE *htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	InterlockedExchange(&phdr->hdr.channel_mask,channelmask);
}

DWORD COREAPI get_loader_snapshot(PVOID section, ULONG length, UCHAR* buf, ULONG *outlength)
{
	DWORD dwRet = ERROR_SUCCESS;

	COREHLP_HANDLE *htransporter = NULL;

	htransporter = (COREHLP_HANDLE*)section;
 
	do 
	{
		if(!DeviceIoControl(htransporter->hNxrm,
							IOCTL_NXRMDRV_GET_LOADER_SNAPSHOT,
							NULL,
							0,
							buf,
							length,
							outlength,
							NULL))
		{
			dwRet = GetLastError();
			break;
		}

	} while (FALSE);

	return dwRet;
}

HRESULT COREAPI query_process_information(PVOID section, HANDLE PID, ULONG length, UCHAR* buf, ULONG *outlength)
{
	HRESULT nRet = ERROR_SUCCESS;

	COREHLP_HANDLE *htransporter = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	do 
	{
		if(!DeviceIoControl(htransporter->hNxrm,
							IOCTL_NXRMDRV_QUERY_PROCESS_INFO,
							(LPVOID)&PID,
							sizeof(HANDLE),
							buf,
							length,
							outlength,
							NULL))
		{
			nRet = GetLastError();
			break;
		}

	} while (FALSE);

	return nRet;
}

LONG COREAPI get_rm_policy_sn(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	return phdr->hdr.policy_sn;
}

void COREAPI increase_rm_policy_sn(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	InterlockedIncrement(&phdr->hdr.policy_sn);
}

BOOL COREAPI get_overlay_windows(PVOID section, ULONG *hWnds, ULONG *cbSize)
{
	BOOL bRet = TRUE;

	BOOL bSkip = FALSE;

	ULONG Pid = GetCurrentProcessId();

	do 
	{
		COREHLP_HANDLE	*htransporter = NULL;

		NXRMSECTIONHDR *phdr = NULL;

		htransporter = (COREHLP_HANDLE*)section;

		phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

		while (0 != InterlockedCompareExchange(&phdr->hdr.dwm_wnd_spin_lock, Pid, 0))
		{
			if (!phdr->hdr.ready)
			{
				bSkip = TRUE;
				break;
			}

			Sleep(0);
		}

		if (bSkip)
		{
			*cbSize = 0;
			bRet = FALSE;
			break;
		}

		//
		// now we have the lock
		//
		memcpy(hWnds,
			   (const void*)(phdr->hdr.dwm_wnd),
			   min(*cbSize, NXRM_MAX_DWM_WND * sizeof(ULONG)));

		*cbSize = min(*cbSize, NXRM_MAX_DWM_WND * sizeof(ULONG));

		phdr->hdr.dwm_wnd_spin_lock = 0;

	} while (FALSE);

	return bRet;
}

void COREAPI set_dwm_active_session(PVOID section, ULONG sessionid)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	InterlockedExchange(&phdr->hdr.dwm_active_session_id, sessionid);
}

ULONG COREAPI set_overlay_windows(PVOID section, ULONG *hWnd, ULONG cbSize)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	ULONG Pid = GetCurrentProcessId();

	ULONG cbCopyLength = min(cbSize, NXRM_MAX_DWM_WND * sizeof(ULONG));

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	while ( 0 != InterlockedCompareExchange(&phdr->hdr.dwm_wnd_spin_lock, Pid, 0))
	{
		Sleep(0);
	}

	//
	// now we have the lock
	//

	memset((void *)phdr->hdr.dwm_wnd,
		   0,
		   NXRM_MAX_DWM_WND * sizeof(ULONG));

	memcpy((void *)phdr->hdr.dwm_wnd,
		   hWnd,
		   cbCopyLength);

	increase_rm_policy_sn(section);

	phdr->hdr.dwm_wnd_spin_lock = 0;

	return cbCopyLength;
}

ULONG COREAPI get_dwm_active_session(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	return phdr->hdr.dwm_active_session_id;
}

BOOL COREAPI is_overlay_bitmap_ready(PVOID section)
{
	COREHLP_HANDLE	*htransporter = NULL;

	NXRMSECTIONHDR *phdr = NULL;

	htransporter = (COREHLP_HANDLE*)section;

	phdr = (NXRMSECTIONHDR*)htransporter->Section.section;

	if (phdr->hdr.ready)
		return (phdr->hdr.channel_mask & NX_CHANNEL_MASK_OVERLAY_BITMAP_READY) ? TRUE : FALSE;
	else
		return FALSE;
}