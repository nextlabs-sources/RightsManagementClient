#include "nxrmfltdef.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltutils.h"
#include "nxrmfltcleanup.h"

extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

extern LPSTR PsGetProcessImageFileName(PEPROCESS  Process);

extern BOOLEAN IsNXLFile(PUNICODE_STRING Extension);

extern BOOLEAN is_explorer(void);

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreCleanup(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BOOLEAN DeleteFile = FALSE;
	BOOLEAN ContentDirty = FALSE;
	BOOLEAN	KeepRecordAndOnDiskNXLFile = FALSE;
	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;
	PNXRMFLT_STREAM_CONTEXT	Ctx = NULL;

	NXL_CACHE_NODE	*pCacheNode = NULL;

	PNXRMFLT_STREAMHANDLE_CONTEXT Ccb = NULL;
	PNXL_RENAME_NODE pRenameNode = NULL;

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	LIST_ENTRY *ite = NULL;

	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;

	do 
	{
		Status = FltGetInstanceContext(FltInstance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		if (InstCtx->DisableFiltering)
		{
			break;
		}

		Status = FltGetStreamHandleContext(FltInstance, FileObject, &Ccb);

		//
		// In the case of failing to get DestinationFileNameInfo in the rename request, DestinationFileNameInfo is NULL
		//
		if (Ccb && Ccb->DestinationFileNameInfo)
		{
			FltAcquirePushLockExclusive(&Global.RenameListLock);

			pRenameNode = nxrmfltFindRenameNodeFromCcb(Ccb);

			if(pRenameNode)
			{
				RemoveEntryList(&(pRenameNode->Link));
			}
			else
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,("nxrmflt!Failed to find rename item for Ccb %p which has src: %wZ and dst: %wZ\n", Ccb, &Ccb->SourceFileNameInfo->Name, &Ccb->DestinationFileNameInfo->Name));
			}

			FltReleasePushLock(&Global.RenameListLock);

			if(pRenameNode)
			{
				nxrmfltFreeRenameNode(pRenameNode);

				ExFreeToPagedLookasideList(&Global.RenameCacheLookaside, pRenameNode);

				pRenameNode = NULL;
			}

			if (Ccb->EncryptDestinationFile)
			{
				FltAcquirePushLockShared(&Global.NxlFileCacheLock);

				pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ccb->DestinationFileNameInfo->Name);

				if (pCacheNode)
				{
					if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
					{
						pCacheNode = NULL;
					}
				}

				FltReleasePushLock(&Global.NxlFileCacheLock);

				if (pCacheNode)
				{
					ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
					ExWaitForRundownProtectionRelease(&pCacheNode->NodeRundownRef);
					Status = nxrmfltEncryptFile(Data,
												FltInstance,
												FltInstance,			// I can use this because it's a rename which means it's in the same volume
												&pCacheNode->ReparseFileName,
												&pCacheNode->OriginalFileName);

					ExReInitializeRundownProtection(&pCacheNode->NodeRundownRef);
					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCleanup: Failed to encrypt file %wZ! Status is %x\n", &pCacheNode->OriginalFileName, Status));
					}
					else
					{
						Ccb->EncryptDestinationFile = FALSE;
					}

					pCacheNode = NULL;
				}
			}
		}

		if (Ccb && Ccb->DeleteOnClose)
		{
			DeleteFile = TRUE;
		}

		Status = FltGetStreamContext(FltInstance, FileObject, &Ctx);

		if (!Ctx)
		{
			PFLT_FILE_NAME_INFORMATION	NameInfo = NULL;
		
			do 
			{
				DeleteFile = (DeleteFile || (FileObject->DeletePending != 0));

				if (!DeleteFile)
				{
					break;
				}

				Status = FltGetFileNameInformation(Data,
												   FLT_FILE_NAME_NORMALIZED,
												   &NameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = FltParseFileNameInformation(NameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				//
				// in the case of explorer delete nxl file
				//
			
				
				{
					UNICODE_STRING SrcFileNameWithoutNXLExtension = {0};

					if (IsNXLFile(&(NameInfo->Extension)))
					{
						SrcFileNameWithoutNXLExtension.Buffer = NameInfo->Name.Buffer;
						SrcFileNameWithoutNXLExtension.Length = NameInfo->Name.Length - 4 * sizeof(WCHAR);
						SrcFileNameWithoutNXLExtension.MaximumLength = NameInfo->Name.MaximumLength;
					}
					else
					{
						SrcFileNameWithoutNXLExtension.Buffer = NameInfo->Name.Buffer;
						SrcFileNameWithoutNXLExtension.Length = NameInfo->Name.Length;
						SrcFileNameWithoutNXLExtension.MaximumLength = NameInfo->Name.MaximumLength;
					}

					//
					// build new cache node, rename on disk nxl file and complete this I/O
					//

					do 
					{
						NXL_CACHE_NODE	*pExistingCacheNode = NULL;
						FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

						pExistingCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &SrcFileNameWithoutNXLExtension);

						if (pExistingCacheNode)
						{
							if (!ExAcquireRundownProtection(&pExistingCacheNode->NodeRundownRef))
							{
								pExistingCacheNode = NULL;
							}
							else {
								//
								// Delete old cache node from cache. NOTE: pCacheNode still valid after delete
								// We only delete the node from cache. We don't free resource here
								//
								DeleteNXLNodeInCache(&Global.NxlFileCache, pExistingCacheNode);

								//
								// ok to call it inside lock because "nxrmfltDeleteFileByName" only queue workitem
								//
								nxrmfltDeleteFileByName(FltInstance,
									&SrcFileNameWithoutNXLExtension);

							}
						}

						FltReleasePushLock(&Global.NxlFileCacheLock);

						if (!pExistingCacheNode)
						{
							break;
						}

						//
						// purge rights cache
						//
						nxrmfltPurgeRightsCache(pExistingCacheNode->Instance, pExistingCacheNode->FileNameHash);

						//
						// release old cache node rundown protection
						//
						ExReleaseRundownProtection(&pExistingCacheNode->NodeRundownRef);
						//
						// free old cache node
						//
						FreeNXLCacheNode(pExistingCacheNode);
						pExistingCacheNode = NULL;

					} while (FALSE);

				}

			} while (FALSE);

			if (NameInfo)
			{
				FltReleaseFileNameInformation(NameInfo);
				NameInfo = NULL;
			}

			break;
		}

		FltAcquirePushLockShared(&Global.ExpireTableLock);

		FOR_EACH_LIST(ite, &Global.ExpireTable)
		{
			NXL_SAVEAS_NODE *pSaveAsNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

			if (RtlEqualUnicodeString(&Ctx->FileName, &pSaveAsNode->SourceFileName, TRUE) &&
				RtlEqualUnicodeString(&pSaveAsNode->SourceFileName, &pSaveAsNode->SaveAsFileName, TRUE))
			{
				KeepRecordAndOnDiskNXLFile = TRUE;
				break;
			}
		}

		FltReleasePushLock(&Global.ExpireTableLock);

		ContentDirty = Ctx->ContentDirty?TRUE:FALSE;

		DeleteFile = (FileObject->DeletePending != 0 || DeleteFile) && (!KeepRecordAndOnDiskNXLFile);

		if (DeleteFile)
		{
			FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);
		}
		else
		{
			FltAcquirePushLockShared(&Global.NxlFileCacheLock);
		}

		pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ctx->FileName);

		if (pCacheNode)
		{
			if (DeleteFile)
			{
				//
				//
				// Remove Cache node from the Cache. We don't free resource here because:
				//		A. There could be other threads are using this record
				//		B. We don't want to wait other threads rundown while holding a push lock
				//
				DeleteNXLNodeInCache(&Global.NxlFileCache, pCacheNode);
				//
				// Delete original file
				//
				Status = nxrmfltDeleteFileByName(Ctx->OriginalInstance, &pCacheNode->OriginalFileName);

				if (!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCleanup: Failed to delete file %wZ! Status is %x\n", &pCacheNode->OriginalFileName, Status));
				}
			}
			else
			{
				if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
				{
					pCacheNode = NULL;
				}
			}
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

		if (!pCacheNode)
		{
			//
			// in the case of protection has been moved
			//
			break;
		}

		if (DeleteFile)
		{
			//
			// purge rights cache
			//
			nxrmfltPurgeRightsCache(pCacheNode->Instance, pCacheNode->FileNameHash);

			FreeNXLCacheNode(pCacheNode);

			pCacheNode = NULL;
		}
		else
		{
			if (ContentDirty)
			{
				NXL_SAVEAS_NODE *pSaveAsNode = NULL;
				NXL_CACHE_NODE *pSrcCacheNode = NULL;

				InterlockedExchange(&Ctx->ContentDirty, 0);

				ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
				ExWaitForRundownProtectionRelease(&pCacheNode->NodeRundownRef);
				Status = nxrmfltEncryptFile(Data,
											FltInstance,
											Ctx->OriginalInstance,
											&pCacheNode->ReparseFileName,
											&pCacheNode->OriginalFileName);
				ExReInitializeRundownProtection(&pCacheNode->NodeRundownRef);
				ExAcquireRundownProtection(&pCacheNode->NodeRundownRef);
				if (!NT_SUCCESS(Status))
				{
					//
					// restore Flag if encryption failed
					//
					InterlockedExchange(&Ctx->ContentDirty, 1);

					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCleanup: Failed to encrypt file %wZ! Status is %x\n", &pCacheNode->OriginalFileName, Status));
				}

				FltAcquirePushLockShared(&Global.ExpireTableLock);

				FOR_EACH_LIST(ite, &Global.ExpireTable)
				{
					pSaveAsNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

					if (RtlEqualUnicodeString(&Ctx->FileName, &pSaveAsNode->SaveAsFileName, TRUE))
					{
						if (!ExAcquireRundownProtection(&pSaveAsNode->NodeRundownRef))
						{
							pSaveAsNode = NULL;
						}

						break;
					}
					else
					{
						pSaveAsNode = NULL;
					}
				}

				FltReleasePushLock(&Global.ExpireTableLock);

				if (pSaveAsNode)
				{
					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pSrcCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &pSaveAsNode->SourceFileName);

					if (pSrcCacheNode)
					{
						if (!ExAcquireRundownProtection(&pSrcCacheNode->NodeRundownRef))
						{
							pSrcCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);

					ExReleaseRundownProtection(&pSaveAsNode->NodeRundownRef);

					pSaveAsNode = NULL;

					if (pSrcCacheNode)
					{
						//
						// Update Tags here
						//
						Status = nxrmfltCopyTags(pSrcCacheNode->Instance,
												 &pSrcCacheNode->OriginalFileName,
												 pCacheNode->Instance,
												 &pCacheNode->OriginalFileName);

						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't copy tags from file %wZ to %wZ\n", &pSrcCacheNode->OriginalFileName, &pCacheNode->OriginalFileName));
						}
						else
						{
							nxrmfltPurgeRightsCache(pCacheNode->Instance, pCacheNode->FileNameHash);
						}

						ExReleaseRundownProtection(&pSrcCacheNode->NodeRundownRef);

						pSrcCacheNode = NULL;
					}
				}

				//
				// copy saved NXL file is the file is on remote or removable media
				//
				if (pCacheNode->OnRemoveOrRemovableMedia && pCacheNode->SourceFileName.Length)
				{
					//
					// still need to make sure the final component in pCacheNode->FileName match SourceFileName because we ONLY deal with "Save"
					//
					if (0 == nxrmfltCompareFinalComponent(&pCacheNode->OriginalFileName,
														  &pCacheNode->SourceFileName,
														  TRUE))
					{
						Status = nxrmfltCopyOnDiskNxlFile(pCacheNode->Instance,
														  &pCacheNode->OriginalFileName,
														  nxrmfltFindInstanceByFileName(&pCacheNode->SourceFileName),
														  &pCacheNode->SourceFileName);

						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't save back file %wZ to %wZ\n", &pSrcCacheNode->OriginalFileName, &pCacheNode->SourceFileName));
						}
					}
				}
			}

			if (KeepRecordAndOnDiskNXLFile && (FileObject->DeletePending != 0))
			{
				SetFlag(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);
				ClearFlag(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
			}
		}

	} while (FALSE);

	if (pCacheNode)
	{
		ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
	}

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	if (Ccb)
	{
		FltReleaseContext(Ccb);
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}