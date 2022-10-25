#include "nxrmfltdef.h"
#include "nxrmflt.h"
#include "nxrmfltexpire.h"
#include "nxrmfltutils.h"

extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

void nxrmfltExpireThreadProc(PVOID ThreadCtx)
{
	NTSTATUS Status = STATUS_SUCCESS;

	LARGE_INTEGER Timeout = {0};

	LIST_ENTRY	ExpiredNodeList = {0};
	LIST_ENTRY	AdobeExpiredNodeList = {0};

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	Timeout.QuadPart = NXRMFLT_SAVEAS_EXPIRE_TIMEOUT_1S * 5;

	while(TRUE)
	{
		Status = KeWaitForSingleObject(&Global.ExpireStopEvent, Executive, KernelMode, FALSE, &Timeout);

		if (Status == STATUS_SUCCESS)
		{
			break;
		}
		else if (Status == STATUS_TIMEOUT)
		{
			InitializeListHead(&ExpiredNodeList);

			FltAcquirePushLockExclusive(&Global.ExpireTableLock);

			FOR_EACH_LIST_SAFE(ite, tmp, &Global.ExpireTable)
			{
				NXL_SAVEAS_NODE *pNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

				NT_ASSERT(ite->Flink);

				if (pNode->ExpireTick >= 12)	// expire in 60 seconds
				{
					RemoveEntryList(ite);

					InsertHeadList(&ExpiredNodeList, &pNode->Link);
				}
				else
				{
					pNode->ExpireTick++;
				}
			}

			FltReleasePushLock(&Global.ExpireTableLock);

			InitializeListHead(&AdobeExpiredNodeList);

			FltAcquirePushLockExclusive(&Global.AdobeRenameExpireTableLock);

			FOR_EACH_LIST_SAFE(ite, tmp, &Global.AdobeRenameExpireTable)
			{
				ADOBE_RENAME_NODE *pNode = CONTAINING_RECORD(ite, ADOBE_RENAME_NODE, Link);

				NT_ASSERT(ite->Flink);

				if (pNode->ExpireTick >= 6)		// expire in 30 seconds
				{
					RemoveEntryList(ite);

					InsertHeadList(&AdobeExpiredNodeList, &pNode->Link);
				}
				else
				{
					pNode->ExpireTick++;
				}
			}

			FltReleasePushLock(&Global.AdobeRenameExpireTableLock);

			NT_ASSERT(ExpiredNodeList.Flink);

			//
			// free expire table
			//
			FOR_EACH_LIST_SAFE(ite, tmp, &ExpiredNodeList)
			{
				NXL_SAVEAS_NODE *pNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

				NT_ASSERT(ite->Flink);

				RemoveEntryList(ite);

				//
				// Wait for all other threads rundown
				//
				ExWaitForRundownProtectionRelease(&pNode->NodeRundownRef);

				ExRundownCompleted(&pNode->NodeRundownRef);

				memset(pNode, 0, sizeof(NXL_SAVEAS_NODE));

				ExFreeToPagedLookasideList(&Global.SaveAsExpireLookaside, pNode);
			}
			
			//
			// free Adobe rename expire table
			//
			FOR_EACH_LIST_SAFE(ite, tmp, &AdobeExpiredNodeList)
			{
				ADOBE_RENAME_NODE *pNode = CONTAINING_RECORD(ite, ADOBE_RENAME_NODE, Link);

				RemoveEntryList(ite);

				nxrmfltFreeAdobeRenameNode(pNode);
			}
		}
		else
		{
			ASSERT(!"THIS SHOULD NEVER HAPPEN!!!");
		}
	}

	PsTerminateSystemThread(Status);
}