#include "nxrmfltdef.h"
#include "nxrmflt.h"
#include "nxrmfltsetea.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltutils.h"

extern DECLSPEC_CACHEALIGN PFLT_FILTER			gFilterHandle;
extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

extern BOOLEAN is_explorer(void);
extern BOOLEAN IsNXLFile(PUNICODE_STRING FinalName);

NTSTATUS build_nxlcache_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *FileName);
NTSTATUS build_nxlcache_reparse_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *FileName);

NTSTATUS get_file_id_and_attribute(PFLT_INSTANCE	Instance, UNICODE_STRING *FileName, LARGE_INTEGER *Id, ULONG *FileAttributes);

FLT_POSTOP_CALLBACK_STATUS
nxrmfltPostSetEA(
_Inout_ PFLT_CALLBACK_DATA		Data,
_In_ PCFLT_RELATED_OBJECTS		FltObjects,
_In_opt_ PVOID					CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS	Flags
)
{
	FLT_POSTOP_CALLBACK_STATUS	CallbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;

	NTSTATUS Status = STATUS_SUCCESS;

	PNXRMFLT_STREAM_CONTEXT		Ctx = (PNXRMFLT_STREAM_CONTEXT)CompletionContext;

	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;

	do
	{
		Status = Data->IoStatus.Status;

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		if (Ctx)
		{
			InterlockedExchange(&Ctx->ContentDirty, 1);

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetEA: Set content dirty flag.\n"));
		}

	} while (FALSE);

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	return CallbackStatus;
}

FLT_PREOP_CALLBACK_STATUS
	nxrmfltPreSetEA(
	_Inout_ PFLT_CALLBACK_DATA				Data,
	_In_ PCFLT_RELATED_OBJECTS				FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
	)
{
	FLT_PREOP_CALLBACK_STATUS CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;

	NTSTATUS Status = STATUS_SUCCESS;

	PNXRMFLT_STREAM_CONTEXT		Ctx = NULL;

	FILE_FULL_EA_INFORMATION *pFullEAInfo = NULL;
	ULONG FullEAInfoLength = 0;

	STRING	EncryptContentEa = {0};
	STRING	TagEa = {0};
	STRING	SyncHeaderEa = {0};
	STRING	EaName = {0};
	STRING	SetSourceEa = {0};

	FLT_FILE_NAME_INFORMATION *pNameInfo = NULL;

	ULONG DirHash = 0;

	NXL_CACHE_NODE *pNode = NULL;

	LARGE_INTEGER FileId = {0};
	ULONG FileAttributes = 0;

	BOOLEAN RemoveCtx = FALSE;

	NXL_KEKEY_BLOB	PrimaryKey = {0};

	NXRMFLT_INSTANCE_CONTEXT	*InstCtx = NULL;

	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;

	do 
	{
		Status = FltGetStreamContext(FltInstance, FileObject, &Ctx);

		//if (Ctx)
		//{
		//	*CompletionContext = Ctx;

		//	CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
		//}

		if (!Global.ClientPort)
		{
			break;
		}

		Status = FltGetInstanceContext(FltInstance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		if (InstCtx->DisableFiltering)
		{
			break;
		}

		pFullEAInfo = (FILE_FULL_EA_INFORMATION *)Data->Iopb->Parameters.SetEa.EaBuffer;
		FullEAInfoLength = Data->Iopb->Parameters.SetEa.Length;

		if (pFullEAInfo == NULL || FullEAInfoLength == 0)
		{
			break;
		}
		
		if (pFullEAInfo->EaNameLength == 0)
		{
			break;
		}

		EaName.Buffer			= pFullEAInfo->EaName;
		EaName.MaximumLength	= pFullEAInfo->EaNameLength;
		EaName.Length			= pFullEAInfo->EaNameLength;

		RtlInitString(&EncryptContentEa, NXRM_EA_ENCRYPT_CONTENT);
		RtlInitString(&TagEa, NXRM_EA_TAG);
		RtlInitString(&SyncHeaderEa, NXRM_EA_SYNC_HEADER);
		RtlInitString(&SetSourceEa, NXRM_EA_SET_SOURCE);

		if (0 == RtlCompareString(&EaName, &EncryptContentEa, FALSE) && Ctx == NULL)	// can not encrypt encrypted file. Encrypted file has Ctx
		{
			UCHAR EaValue = 0;

			if (pFullEAInfo->EaValueLength != sizeof(UCHAR))
			{
				break;
			}

			EaValue = *((UCHAR*)(pFullEAInfo->EaName + pFullEAInfo->EaNameLength + 1));

			if (EaValue)
			{

				Status = FltGetFileNameInformation(Data,
												   FLT_FILE_NAME_NORMALIZED,
												   &pNameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = FltParseFileNameInformation(pNameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				RtlHashUnicodeString(&pNameInfo->ParentDir, TRUE, HASH_STRING_ALGORITHM_X65599, &DirHash);

				//
				// Calling this function before acquiring the lock because this function generates I/O.
				//
				get_file_id_and_attribute(FltInstance, &pNameInfo->Name, &FileId, &FileAttributes);


				Status = FltAllocateContext(Global.Filter, 
											FLT_STREAM_CONTEXT, 
											sizeof(NXRMFLT_STREAM_CONTEXT), 
											NonPagedPool, 
											(PFLT_CONTEXT*)&Ctx);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				//
				// Initialize Ctx
				//
				memset(Ctx, 0, sizeof(NXRMFLT_STREAM_CONTEXT));

				//
				// make sure CtxCleanup won't free NULL point in case there is error when building this Ctx
				//
				Ctx->ReleaseFileName = FALSE;

				InterlockedIncrement(&Global.TotalContext);

				Status = nxrmfltBuildNamesInStreamContext(Ctx, &pNameInfo->Name);

				if(!NT_SUCCESS(Status))
				{
					break;
				}

				Ctx->OriginalInstance = FltInstance;

				Status = FltGetRequestorSessionId(Data, &Ctx->RequestorSessionId);

				if (!NT_SUCCESS(Status))
				{
					Ctx->RequestorSessionId = NXRMFLT_INVALID_SESSION_ID;
				}

				//
				// forcing create NXL file after closing the handle
				//
				Ctx->ContentDirty = 1;

				FltInitializePushLock(&Ctx->CtxLock);

				Status = FltSetStreamContext(FltInstance,
											 FileObject,
											 FLT_SET_CONTEXT_KEEP_IF_EXISTS,
											 Ctx,
											 NULL);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

				do
				{
					pNode = FindNXLNodeInCache(&Global.NxlFileCache, &pNameInfo->Name);

					if (!pNode)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!FindNXLNode can't find file %wZ in cache\n", &pNameInfo->Name));

						pNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

						if (pNode)
						{
							memset(pNode, 0, sizeof(NXL_CACHE_NODE));

							Status = build_nxlcache_file_name(pNode, &pNameInfo->Name);

							if (!NT_SUCCESS(Status))
							{
								//
								// ERROR case. No memory
								//
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &pNameInfo->Name));

								ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pNode);
								pNode = NULL;
								RemoveCtx = TRUE;
								break;
							}

							pNode->FileAttributes = FileAttributes;
							pNode->FileID.QuadPart = FileId.QuadPart;
							pNode->Flags |= FlagOn(FileAttributes, FILE_ATTRIBUTE_READONLY) ? NXRMFLT_FLAG_READ_ONLY : 0;
							pNode->Flags |= NXRMFLT_FLAG_CTX_ATTACHED;
							pNode->Instance = FltObjects->Instance;
							pNode->ParentDirectoryHash = DirHash;
							pNode->OnRemoveOrRemovableMedia = FALSE;

							ExInitializeRundownProtection(&pNode->NodeRundownRef);

							Status = build_nxlcache_reparse_file_name(pNode, &pNameInfo->Name);

							if (!NT_SUCCESS(Status))
							{
								//
								// ERROR case. No memory
								//
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &pNameInfo->Name));

								if (pNode->ReleaseFileName)
								{
									ExFreePoolWithTag(pNode->FileName.Buffer, NXRMFLT_NXLCACHE_TAG);
									RtlInitUnicodeString(&pNode->FileName, NULL);
								}

								ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pNode);
								pNode = NULL;
								RemoveCtx = TRUE;
								break;
							}

							PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!AddNXLNodeToCache add file %wZ into cache\n", &pNameInfo->Name));

							AddNXLNodeToCache(&Global.NxlFileCache, pNode);

							if (!ExAcquireRundownProtection(&pNode->NodeRundownRef))
							{
								pNode = NULL;
							}
						}
					}
					else
					{
						PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!FindNXLNode found file %wZ in cache\n", &pNameInfo->Name));

						if (FlagOn(pNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED))
						{
							//
							// This means ejected device come back. Files on this device could have been changed
							// We need to clear the NXRMFLT_FLAG_CTX_ATTACHED flag to force a copy when opening
							// There are two possible outcome as clear the NXRMFLT_FLAG_CTX_ATTACHED flag:
							//		A) File was opened and still opened by some process. Copying should fail
							//		B) File is closed by copying. Content is overwritten. Copying should be OK
							//
							ClearFlag(pNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED);
							ClearFlag(pNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
						}
						else
						{
							SetFlag(pNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
							ClearFlag(pNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);
						}

						pNode = NULL;
					}

				} while (FALSE);

				FltReleasePushLock(&Global.NxlFileCacheLock);
			
				if (RemoveCtx)
				{
					//
					// delete already set ctx
					// However, the reference of this ctx still exists. Later it will be released by "FltReleaseContext(Ctx)" call.
					//
					FltDeleteStreamContext(FltInstance, FileObject, NULL);
				}

				if (pNode)
				{
					FltAcquirePushLockShared(&Global.PrimaryKeyLock);

					memcpy(&PrimaryKey, 
						   &Global.PrimaryKey, 
						   min(sizeof(PrimaryKey),sizeof(Global.PrimaryKey)));

					FltReleasePushLock(&Global.PrimaryKeyLock);

					Status = NXLCreateFile(Global.Filter,
										   FltInstance,
										   &pNode->OriginalFileName,
										   &PrimaryKey,
										   NULL,
										   TRUE);

					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't create NXL file %wZ\n", &pNode->OriginalFileName));
					}
				}

				Data->IoStatus.Status		= STATUS_SUCCESS;
				Data->IoStatus.Information	= 0;

				CallbackStatus = FLT_PREOP_COMPLETE;
			}
			else
			{
				UNICODE_STRING SrcFileNameWithoutNXLExtension = {0};

				//
				// TO DO: Removing protection. 
				//
				if (!is_explorer())
				{
					break;
				}

				Status = FltGetFileNameInformation(Data,
												   FLT_FILE_NAME_NORMALIZED,
												   &pNameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = FltParseFileNameInformation(pNameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				if (!IsNXLFile(&(pNameInfo->Extension)))
				{
					break;
				}

				SrcFileNameWithoutNXLExtension.Buffer			= pNameInfo->Name.Buffer;
				SrcFileNameWithoutNXLExtension.Length			= pNameInfo->Name.Length - 4 * sizeof(WCHAR);
				SrcFileNameWithoutNXLExtension.MaximumLength	= pNameInfo->Name.MaximumLength;

				do 
				{
					NXL_CACHE_NODE	*pExistingCacheNode = NULL;
					ULONGLONG		RightsMask = 0;

					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pExistingCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &SrcFileNameWithoutNXLExtension);

					if (pExistingCacheNode)
					{
						if (!ExAcquireRundownProtection(&pExistingCacheNode->NodeRundownRef))
						{
							pExistingCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);

					if (!pExistingCacheNode)
					{
						Status = STATUS_FILE_NOT_ENCRYPTED;
						break;
					}

					Status = nxrmfltCheckRights((HANDLE)FltGetRequestorProcessId(Data),
												Data->Thread?PsGetThreadId(Data->Thread) : PsGetCurrentThreadId(),
												pExistingCacheNode,
												FALSE,
												&RightsMask,
												NULL,
												NULL);

					if (Status != STATUS_SUCCESS)
					{
						break;
					}

					if (!(RightsMask & BUILTIN_RIGHT_DECRYPT))
					{
						Status = STATUS_PRIVILEGE_NOT_HELD;
						break;
					}
					
					if (!FlagOn(pExistingCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED))
					{
						// Set decrypting flag, otherwise IRP_DIR_CONTROL code will delete any decrypted file
						SetFlag(pExistingCacheNode->Flags, NXRMFLT_FLAG_DECRYPTING);
						Status = nxrmfltDecryptFile(Data,
													FltInstance,
													FltInstance,
													&pNameInfo->Name,
													&SrcFileNameWithoutNXLExtension);
						// --> No need to remove this flag because anyway we are going to delete this cached node
					}

					//
					// Let's remove and delete the old Node
					//

					FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

					//
					// ok to call it inside lock because "nxrmfltDeleteFileByName" only queue workitem
					//
					nxrmfltDeleteFileByName(FltInstance,
											&pNameInfo->Name);

					//
					// Delete old cache node from cache. NOTE: pCacheNode still valid after delete
					// We only delete the node from cache. We don't free resource here
					//
					DeleteNXLNodeInCache(&Global.NxlFileCache, pExistingCacheNode);

					FltReleasePushLock(&Global.NxlFileCacheLock);

					//
					// release old cache node rundown protection
					//
					ExReleaseRundownProtection(&pExistingCacheNode->NodeRundownRef);

					//
					// purge rights cache
					//
					nxrmfltPurgeRightsCache(pExistingCacheNode->Instance, pExistingCacheNode->FileNameHash);

					//
					// free old cache node
					//
					FreeNXLCacheNode(pExistingCacheNode);
					pExistingCacheNode = NULL;

				} while (FALSE);

				Data->IoStatus.Status		= Status;
				Data->IoStatus.Information	= 0;

				CallbackStatus = FLT_PREOP_COMPLETE;
			}
		}
		else if (0 == RtlCompareString(&EaName, &EncryptContentEa, FALSE) && Ctx)
		{
			UCHAR EaValue = 0;

			if (pFullEAInfo->EaValueLength != sizeof(UCHAR))
			{
				break;
			}

			EaValue = *((UCHAR*)(pFullEAInfo->EaName + pFullEAInfo->EaNameLength + 1));

			if (EaValue)
			{

				Status = FltGetFileNameInformation(Data,
												   FLT_FILE_NAME_NORMALIZED,
												   &pNameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = FltParseFileNameInformation(pNameInfo);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				RtlHashUnicodeString(&pNameInfo->ParentDir, TRUE, HASH_STRING_ALGORITHM_X65599, &DirHash);

				//
				// Calling this function before acquiring the lock because this function generates I/O.
				//
				get_file_id_and_attribute(FltInstance, &pNameInfo->Name, &FileId, &FileAttributes);

				//
				// forcing create NXL file after closing the handle
				//
				Ctx->ContentDirty = 1;

				FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

				do
				{
					pNode = FindNXLNodeInCache(&Global.NxlFileCache, &pNameInfo->Name);

					if (!pNode)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!FindNXLNode can't find file %wZ in cache\n", &pNameInfo->Name));

						pNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

						if (pNode)
						{
							memset(pNode, 0, sizeof(NXL_CACHE_NODE));

							Status = build_nxlcache_file_name(pNode, &pNameInfo->Name);

							if (!NT_SUCCESS(Status))
							{
								//
								// ERROR case. No memory
								//
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &pNameInfo->Name));

								ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pNode);
								pNode = NULL;
								break;
							}

							pNode->FileAttributes = FileAttributes;
							pNode->FileID.QuadPart = FileId.QuadPart;
							pNode->Flags |= FlagOn(FileAttributes, FILE_ATTRIBUTE_READONLY) ? NXRMFLT_FLAG_READ_ONLY : 0;
							pNode->Flags |= NXRMFLT_FLAG_CTX_ATTACHED;
							pNode->Instance = FltObjects->Instance;
							pNode->ParentDirectoryHash = DirHash;
							pNode->OnRemoveOrRemovableMedia = FALSE;

							ExInitializeRundownProtection(&pNode->NodeRundownRef);

							Status = build_nxlcache_reparse_file_name(pNode, &pNameInfo->Name);

							if (!NT_SUCCESS(Status))
							{
								//
								// ERROR case. No memory
								//
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &pNameInfo->Name));

								if (pNode->ReleaseFileName)
								{
									ExFreePoolWithTag(pNode->FileName.Buffer, NXRMFLT_NXLCACHE_TAG);
									RtlInitUnicodeString(&pNode->FileName, NULL);
								}

								ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pNode);
								pNode = NULL;
								break;
							}

							PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!AddNXLNodeToCache add file %wZ into cache\n", &pNameInfo->Name));

							AddNXLNodeToCache(&Global.NxlFileCache, pNode);

							if (!ExAcquireRundownProtection(&pNode->NodeRundownRef))
							{
								pNode = NULL;
							}
						}
					}
					else
					{
						PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!FindNXLNode found file %wZ in cache\n", &pNameInfo->Name));

						if (FlagOn(pNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED))
						{
							//
							// This means ejected device come back. Files on this device could have been changed
							// We need to clear the NXRMFLT_FLAG_CTX_ATTACHED flag to force a copy when opening
							// There are two possible outcome as clear the NXRMFLT_FLAG_CTX_ATTACHED flag:
							//		A) File was opened and still opened by some process. Copying should fail
							//		B) File is closed by copying. Content is overwritten. Copying should be OK
							//
							ClearFlag(pNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED);
							ClearFlag(pNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
						}
						else
						{
							SetFlag(pNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
							ClearFlag(pNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);
						}

						pNode = NULL;
					}

				} while (FALSE);

				FltReleasePushLock(&Global.NxlFileCacheLock);

				if (pNode)
				{
					FltAcquirePushLockShared(&Global.PrimaryKeyLock);

					memcpy(&PrimaryKey, 
						   &Global.PrimaryKey, 
						   min(sizeof(PrimaryKey),sizeof(Global.PrimaryKey)));

					FltReleasePushLock(&Global.PrimaryKeyLock);

					Status = NXLCreateFile(Global.Filter,
										   FltInstance,
										   &pNode->OriginalFileName,
										   &PrimaryKey,
										   NULL,
										   TRUE);

					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't create NXL file %wZ\n", &pNode->OriginalFileName));
					}
				}

				Data->IoStatus.Status		= STATUS_SUCCESS;
				Data->IoStatus.Information	= 0;

				CallbackStatus = FLT_PREOP_COMPLETE;
			}
			else
			{
				//
				// NOT A VALID CASE. Ask me to decrypt a file with CTX? Only explorer can decrypt and explorer never open NXL file with Ctx
				//
				Data->IoStatus.Status		= STATUS_INVALID_PARAMETER;
				Data->IoStatus.Information	= 0;

				CallbackStatus = FLT_PREOP_COMPLETE;
			}
		}
		else if (0 == RtlCompareString(&EaName, &TagEa, FALSE))
		{
			CHAR *Tag = NULL;
			UCHAR *EaData = NULL;
			ULONG DataLength = 0;

			HANDLE				NXLFileHandle = NULL;
			FILE_OBJECT			*NXLFileObject = NULL;
			OBJECT_ATTRIBUTES	NXLObjectAttribute = {0};
			IO_STATUS_BLOCK		IoStatus = {0};
			NXL_CACHE_NODE		*pCacheNode = NULL;
			UNICODE_STRING		SrcFileNameWithoutNXLExtension = {0};

			BOOLEAN			IsGoodNXLFile = FALSE;
			NXL_HEADER		*NXLHdr = NULL;
			UCHAR			ContentKey[NXRMFLT_CONTENT_KEY_LENGTH] = { 0 };
			LIST_ENTRY		*ite = NULL;
			NXL_KEYCHAIN_NODE *pKeyChainNode = NULL;

			do 
			{
				if ((!Ctx) && (!is_explorer()))
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				if (Ctx)
				{
					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ctx->FileName);

					if (pCacheNode)
					{
						if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
						{
							pCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);
				}
				else
				{
					Status = FltGetFileNameInformation(Data,
													   FLT_FILE_NAME_NORMALIZED,
													   &pNameInfo);
					if (!NT_SUCCESS(Status))
					{
						break;
					}

					Status = FltParseFileNameInformation(pNameInfo);

					if (!NT_SUCCESS(Status))
					{
						break;
					}
					
					SrcFileNameWithoutNXLExtension.Buffer			= pNameInfo->Name.Buffer;
					SrcFileNameWithoutNXLExtension.Length			= pNameInfo->Name.Length - sizeof(WCHAR) * 4;
					SrcFileNameWithoutNXLExtension.MaximumLength	= pNameInfo->Name.MaximumLength;

					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &SrcFileNameWithoutNXLExtension);

					if (pCacheNode)
					{
						if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
						{
							pCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);
				}

				if (!pCacheNode)
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				Tag = (CHAR*)ExAllocatePoolWithTag(PagedPool, pFullEAInfo->EaValueLength, NXRMFLT_TMP_TAG);

				if (!Tag)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				memset(Tag, 0, pFullEAInfo->EaValueLength);

				memcpy(Tag, (pFullEAInfo->EaName + pFullEAInfo->EaNameLength + 1), pFullEAInfo->EaValueLength);

				pFullEAInfo = (FILE_FULL_EA_INFORMATION *)((UCHAR*)pFullEAInfo + pFullEAInfo->NextEntryOffset);

				EaName.Buffer			= pFullEAInfo->EaName;
				EaName.MaximumLength	= pFullEAInfo->EaNameLength;
				EaName.Length			= pFullEAInfo->EaNameLength;

				if (0 != RtlCompareString(&EaName, &SyncHeaderEa, FALSE))
				{
					Status = STATUS_INVALID_PARAMETER;
					break;
				}

				DataLength = pFullEAInfo->EaValueLength;

				EaData = (UCHAR*)ExAllocatePoolWithTag(PagedPool, DataLength, NXRMFLT_TMP_TAG);

				if (!EaData)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				memset(EaData, 0, DataLength);

				memcpy(EaData, (pFullEAInfo->EaName + pFullEAInfo->EaNameLength + 1), DataLength);

				InitializeObjectAttributes(&NXLObjectAttribute,
										   &pCacheNode->OriginalFileName,
										   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
										   NULL,
										   NULL);

				//
				// open the real NXL file to Sync header
				//

				Status = FltCreateFileEx2(FltObjects->Filter,
										  pCacheNode->Instance,
										  &NXLFileHandle,
										  &NXLFileObject,
										  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
										  &NXLObjectAttribute,
										  &IoStatus,
										  NULL,
										  FILE_ATTRIBUTE_NORMAL,
										  FILE_SHARE_VALID_FLAGS,
										  FILE_OPEN,
										  FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
										  NULL,
										  0,
										  IO_IGNORE_SHARE_ACCESS_CHECK,
										  NULL);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = NXLCheck(pCacheNode->Instance, NXLFileObject, &IsGoodNXLFile);

				if(!NT_SUCCESS(Status) || (!IsGoodNXLFile))
				{
					if (!IsGoodNXLFile)
					{
						Status = STATUS_INVALID_PARAMETER;
					}

					break;
				}

				NXLHdr = (NXL_HEADER *)ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), NXRMFLT_TMP_TAG);

				if(!NXLHdr)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				RtlSecureZeroMemory(NXLHdr, sizeof(*NXLHdr));

				Status = NXLReadHeader(pCacheNode->Instance, NXLFileObject, NXLHdr);

				if(!NT_SUCCESS(Status))
				{
					break;
				}

				FltAcquirePushLockShared(&Global.KeyChainLock);

				FOR_EACH_LIST(ite, &Global.KeyChain)
				{
					pKeyChainNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

					if (memcmp(pKeyChainNode->KeyBlob.KeyId.Id, NXLHdr->Crypto.PrimaryKey.KeKeyId.Id, min(pKeyChainNode->KeyBlob.KeyId.IdSize, NXLHdr->Crypto.PrimaryKey.KeKeyId.IdSize)) == 0 &&
						pKeyChainNode->KeyBlob.KeyId.Algorithm	== NXLHdr->Crypto.PrimaryKey.KeKeyId.Algorithm)
					{
						memcpy(&PrimaryKey, 
							   &pKeyChainNode->KeyBlob, 
							   min(sizeof(PrimaryKey),sizeof(pKeyChainNode->KeyBlob)));

						break;
					}
					else
					{
						pKeyChainNode = NULL;
					}
				}

				FltReleasePushLock(&Global.KeyChainLock);

				if (!pKeyChainNode)
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
								 ("nxrmflt!nxrmfltSetTags: Can't find key in current key chain to set tags for file %wZ.", &pCacheNode->OriginalFileName));

					nxrmfltSendKeyChainErrorMsg(FltInstance, &pCacheNode->OriginalFileName, &NXLHdr->Crypto.PrimaryKey.KeKeyId);

					Status = STATUS_DECRYPTION_FAILED;
					break;
				}

				Status = NXLGetContentKey((PCNXL_HEADER)NXLHdr, PrimaryKey.Key, ContentKey);

				if(!NT_SUCCESS(Status))
				{
					break;
				}

				//
				// has to pass the real NXL instance and file object
				//
				Status = NXLWriteSectionData(pCacheNode->Instance, 
											 NXLFileObject, 
											 ContentKey,
											 Tag,
											 EaData,
											 DataLength);

				if (!NT_SUCCESS(Status))
				{
					break;
				}

				////
				//// Adding remote evaluation
				////
				//Status = NXLWriteSectionData(pCacheNode->Instance,
				//							 NXLFileObject,
				//							 ContentKey,
				//							 NXL_SECTION_TEMPLATES,
				//							 "{\"type\": \"remote only\"}",
				//							 sizeof("{\"type\": \"remote only\"}"));

				//if (!NT_SUCCESS(Status))
				//{
				//	break;
				//}


				//
				// remove rights cache on this file because tags changed
				//

				nxrmfltPurgeRightsCache(pCacheNode->Instance, pCacheNode->FileNameHash);

			} while (FALSE);

			if (Tag)
			{
				ExFreePoolWithTag(Tag, NXRMFLT_TMP_TAG);
				Tag = NULL;
			}

			if (EaData)
			{
				ExFreePoolWithTag(EaData, NXRMFLT_TMP_TAG);
				Tag = NULL;
			}

			if (pCacheNode)
			{
				ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
				pCacheNode = NULL;
			}

			if (NXLFileHandle)
			{
				FltClose(NXLFileHandle);
				NXLFileHandle = NULL;
			}

			if (NXLFileObject)
			{
				ObDereferenceObject(NXLFileObject);
				NXLFileObject = NULL;
			}

			if (NXLHdr)
			{
				ExFreePoolWithTag(NXLHdr, NXRMFLT_TMP_TAG);
				NXLHdr = NULL;
			}

			Data->IoStatus.Status		= Status;
			Data->IoStatus.Information	= 0;

			CallbackStatus = FLT_PREOP_COMPLETE;
		}
		else if (0 == RtlCompareString(&EaName, &SetSourceEa, FALSE))
		{
			ULONG SourceFileNameLength = 0;
			UCHAR *EaData = NULL;
			ULONG DataLength = 0;

			NXL_CACHE_NODE		*pCacheNode = NULL;
			UNICODE_STRING		SrcFileNameWithoutNXLExtension = { 0 };

			do
			{
				if (Ctx)
				{
					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ctx->FileName);

					if (pCacheNode)
					{
						if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
						{
							pCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);
				}
				else
				{
					Status = FltGetFileNameInformation(Data,
													   FLT_FILE_NAME_NORMALIZED,
													   &pNameInfo);
					if (!NT_SUCCESS(Status))
					{
						break;
					}

					Status = FltParseFileNameInformation(pNameInfo);

					if (!NT_SUCCESS(Status))
					{
						break;
					}

					SrcFileNameWithoutNXLExtension.Buffer = pNameInfo->Name.Buffer;
					SrcFileNameWithoutNXLExtension.Length = pNameInfo->Name.Length - sizeof(WCHAR) * 4;
					SrcFileNameWithoutNXLExtension.MaximumLength = pNameInfo->Name.MaximumLength;

					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &SrcFileNameWithoutNXLExtension);

					if (pCacheNode)
					{
						if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
						{
							pCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);
				}

				if (!pCacheNode)
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				SourceFileNameLength = pFullEAInfo->EaValueLength;

				if (SourceFileNameLength > sizeof(pCacheNode->SourceFileNameFastBuffer))
				{
					pCacheNode->SourceFileName.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool, SourceFileNameLength, NXRMFLT_NXLCACHE_TAG);

					if (!pCacheNode->SourceFileName.Buffer)
					{
						Status = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}

					memset(pCacheNode->SourceFileName.Buffer, 0, SourceFileNameLength);

					memcpy(pCacheNode->SourceFileName.Buffer, (pFullEAInfo->EaName + pFullEAInfo->EaNameLength + 1), SourceFileNameLength);

					pCacheNode->SourceFileName.Length = (USHORT)SourceFileNameLength;
					pCacheNode->SourceFileName.MaximumLength = (USHORT)SourceFileNameLength;
					pCacheNode->ReleaseSourceFileName = TRUE;
				}
				else
				{
					pCacheNode->SourceFileName.Buffer = pCacheNode->SourceFileNameFastBuffer;

					memset(pCacheNode->SourceFileName.Buffer, 0, SourceFileNameLength);

					memcpy(pCacheNode->SourceFileName.Buffer, (pFullEAInfo->EaName + pFullEAInfo->EaNameLength + 1), SourceFileNameLength);

					pCacheNode->SourceFileName.Length = (USHORT)SourceFileNameLength;
					pCacheNode->SourceFileName.MaximumLength = (USHORT)sizeof(pCacheNode->SourceFileNameFastBuffer);
					pCacheNode->ReleaseSourceFileName = FALSE;
				}

				pCacheNode->OnRemoveOrRemovableMedia = TRUE;


			} while (FALSE);

			if (pCacheNode)
			{
				ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
				pCacheNode = NULL;
			}

			Data->IoStatus.Status = Status;
			Data->IoStatus.Information = 0;

			CallbackStatus = FLT_PREOP_COMPLETE;

		}

	} while (FALSE);

	if (pNode)
	{
		ExReleaseRundownProtection(&pNode->NodeRundownRef);
	}

	if (pNameInfo)
	{
		FltReleaseFileNameInformation(pNameInfo);
	}

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return CallbackStatus;
}

NTSTATUS get_file_id_and_attribute(PFLT_INSTANCE	Instance, UNICODE_STRING *FileName, LARGE_INTEGER *Id, ULONG *FileAttributes)
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	HANDLE			FileHandle = NULL;
	PFILE_OBJECT	FileObject = NULL;

	IO_STATUS_BLOCK IoStatus = { 0 };

	OBJECT_ATTRIBUTES	ObjectAttribute = { 0 };

	FILE_BASIC_INFORMATION BasicInfo = {0};
	FILE_INTERNAL_INFORMATION IdInfo = {0};

	InitializeObjectAttributes(&ObjectAttribute,
							   FileName,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	do 
	{
		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &FileHandle,
								  &FileObject,
								  GENERIC_READ,
								  &ObjectAttribute,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OPEN,
								  FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  IO_IGNORE_SHARE_ACCESS_CHECK,
								  NULL);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltQueryInformationFile(Instance,
										 FileObject,
										 &BasicInfo,
										 sizeof(BasicInfo),
										 FileBasicInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltQueryInformationFile(Instance,
										 FileObject,
										 &IdInfo,
										 sizeof(IdInfo),
										 FileInternalInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Id->QuadPart = IdInfo.IndexNumber.QuadPart;

		*FileAttributes = BasicInfo.FileAttributes;


	} while (FALSE);

	if (FileHandle)
	{
		FltClose(FileHandle);
		FileHandle = NULL;
	}

	if (FileObject)
	{
		ObDereferenceObject(FileObject);
		FileObject = NULL;
	}

	return Status;
}