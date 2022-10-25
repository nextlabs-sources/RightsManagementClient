#include "nxrmfltdef.h"
#include "nxrmfltcreate.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltutils.h"
#include "nxrmflt.h"

extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

extern LPSTR PsGetProcessImageFileName(PEPROCESS  Process);
extern ULONG PsGetCurrentProcessSessionId(void);

BOOLEAN IsNXLFile(PUNICODE_STRING FinalName);
static BOOLEAN build_reparse_name(NXRMFLT_STREAM_CONTEXT *Ctx);
static void ascii2lower(UNICODE_STRING *str);
static BOOLEAN IsReparseEcpPresent(PFLT_CALLBACK_DATA Data);
static BOOLEAN IsBlockingEcpPresent(PFLT_CALLBACK_DATA Data);
static BOOLEAN CompareMupDeviceNames(PUNICODE_STRING FileName, PUNICODE_STRING OpeningMupName);
static BOOLEAN is_process_a_service(PEPROCESS  Process);
NTSTATUS get_file_id_and_attribute_ex(PFLT_INSTANCE	Instance, FILE_OBJECT *FileObject, LARGE_INTEGER *Id, ULONG *FileAttributes);

BOOLEAN is_explorer(void);
BOOLEAN is_app_in_real_name_access_list(PEPROCESS  Process);

extern NTSTATUS build_nxlcache_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *FileName);
extern NTSTATUS build_nxlcache_reparse_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *FileName);

FLT_PREOP_CALLBACK_STATUS nxrmfltPreCreate(
	_Inout_ PFLT_CALLBACK_DATA				Data,
	_In_ PCFLT_RELATED_OBJECTS				FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext)
{
	PFLT_FILE_NAME_INFORMATION	NameInfo = NULL;
	NTSTATUS					Status = STATUS_SUCCESS;
	FLT_PREOP_CALLBACK_STATUS	CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	BOOLEAN						ReleaseNameInfo = TRUE;

	NXRMFLT_INSTANCE_CONTEXT	*InstCtx = NULL;

	NXL_CACHE_NODE	*pCacheNode = NULL;

	BOOLEAN	CtxAttached = FALSE;

	ULONG Flags = 0;
	
	BOOLEAN	IgnoreCase = TRUE;

	UNICODE_STRING ReparseFileName = { 0 };

	WCHAR	*ReparseFileNameBuffer = NULL;

	ULONG ParentDirectoryHash = 0;

	ULONGLONG RightsMask = 0;

	BOOLEAN SkipProcessingAfterCheckingRights = FALSE;
	
	LIST_ENTRY *ite = NULL;

	NXL_SAVEAS_NODE *pSaveAsNode = NULL;

	NXL_CACHE_NODE	*pSaveAsSourceCacheNode = NULL;

	ULONG CreateDisposition = 0;
	ULONG CreateOptions = 0;

	BOOLEAN IsExplorerDeleteFile = FALSE;
	BOOLEAN IsExplorer = FALSE;
	BOOLEAN IsExplorerRequiringOpLock = FALSE;

	DECLARE_CONST_UNICODE_STRING(LegacyThemesFolder, NXRMFLT_LEGACY_THEMES_FOLDER);

	do 
	{

		if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE))
		{
			break;
		}

		if (FlagOn(Data->Iopb->TargetFileObject->Flags, FO_VOLUME_OPEN))
		{
			break;
		}

		if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID))
		{
			break;
		}

		if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DIRECTORY_FILE))
		{
			break;
		}

		if (Global.DriverUnloading)
		{
			break;
		}

		IgnoreCase = !(BooleanFlagOn(Data->Iopb->OperationFlags, SL_CASE_SENSITIVE));

		CreateDisposition = (Data->Iopb->Parameters.Create.Options & 0xff000000) >> 24;
		CreateOptions = (Data->Iopb->Parameters.Create.Options & 0x00ffffff);

		if (IsReparseEcpPresent(Data))
		{
			*CompletionContext = NULL;

			CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

			break;
		}

		if (IsBlockingEcpPresent(Data))
		{
			*CompletionContext = NULL;

			if (FlagOn(CreateOptions, FILE_DELETE_ON_CLOSE))
				CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
			else
				CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

			break;
		}

		//
		//  Get the name information.
		//

		if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY)) 
		{

			//
			//  The SL_OPEN_TARGET_DIRECTORY flag indicates the caller is attempting
			//  to open the target of a rename or hard link creation operation. We
			//  must clear this flag when asking fltmgr for the name or the result
			//  will not include the final component. We need the full path in order
			//  to compare the name to our mapping.
			//

			ClearFlag(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY);

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						 ("nxrmflt!nxrmfltPreCreate: -> Clearing SL_OPEN_TARGET_DIRECTORY for %wZ (Cbd = %p, FileObject = %p)\n",
						 &NameInfo->Name,
						 Data,
						 FltObjects->FileObject));

			//
			//  Get the filename as it appears below this filter. Note that we use 
			//  FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY when querying the filename
			//  so that the filename as it appears below this filter does not end up
			//  in filter manager's name cache.
			//

			Status = FltGetFileNameInformation(Data,
											   FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY,
											   &NameInfo);

			//
			//  Restore the SL_OPEN_TARGET_DIRECTORY flag so the create will proceed 
			//  for the target. The file systems depend on this flag being set in 
			//  the target create in order for the subsequent SET_INFORMATION 
			//  operation to proceed correctly.
			//

			SetFlag(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY);


		}
		else
		{

			//
			//  Note that we use FLT_FILE_NAME_QUERY_DEFAULT when querying the 
			//  filename. In the precreate the filename should not be in filter
			//  manager's name cache so there is no point looking there.
			//

			Status = FltGetFileNameInformation(Data,
											   FLT_FILE_NAME_OPENED |
											   FLT_FILE_NAME_QUERY_DEFAULT,
											   &NameInfo);
		}

		if (!NT_SUCCESS(Status)) 
		{

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						 ("nxrmflt!nxrmfltPreCreate: -> Failed to get name information (Cbd = %p, FileObject = %p)\n",
						 Data,
						 FltObjects->FileObject));

			break;
		}


		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
					 ("nxrmflt!nxrmfltPreCreate -> Processing create for file %wZ (Cbd = %p, FileObject = %p)\n",
					 &NameInfo->Name,
					 Data,
					 FltObjects->FileObject));

		//
		//  Parse the filename information
		//

		Status = FltParseFileNameInformation(NameInfo);
		
		if (!NT_SUCCESS(Status)) 
		{

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						 ("nxrmflt!nxrmfltPreCreate -> Failed to parse name information for file %wZ (Cbd = %p, FileObject = %p)\n",
						 &NameInfo->Name,
						 Data,
						 FltObjects->FileObject));

			break;
		}

		if ((RtlCompareUnicodeString(&LegacyThemesFolder, &NameInfo->ParentDir, TRUE) == 0) && is_explorer())
		{
			Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
			Data->IoStatus.Information	= 0;
			CallbackStatus				= FLT_PREOP_COMPLETE;
			
			break;
		}

		FltAcquirePushLockShared(&Global.NxlFileCacheLock);

		pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &NameInfo->Name);

		if (NULL == pCacheNode) {
			BOOLEAN bShortFormat = FALSE;

			for (int i = 0; i < NameInfo->Name.Length / 2; i++) {
				if (NameInfo->Name.Buffer[i] == '~') {
					bShortFormat = TRUE;
					break;
				}
			}

			if(bShortFormat){
				FltReleasePushLock(&Global.NxlFileCacheLock);

				PFLT_FILE_NAME_INFORMATION LongNameInfo = NULL;
				Status = FltGetFileNameInformation(Data,
					FLT_FILE_NAME_NORMALIZED,
					&LongNameInfo);

				if (NT_SUCCESS(Status)) {
					Status = FltParseFileNameInformation(LongNameInfo);
				}

				FltAcquirePushLockShared(&Global.NxlFileCacheLock);
				if (NT_SUCCESS(Status)) {
					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &LongNameInfo->Name);
					if (pCacheNode) {
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: convert file name %wZ -> %wZ! \n", &NameInfo->Name, &LongNameInfo->Name));
					}
					FltReleaseFileNameInformation(NameInfo);
					NameInfo = LongNameInfo;
				}
				else if (NULL != LongNameInfo) {
					FltReleaseFileNameInformation(LongNameInfo);
				}
			}
		}

		if (pCacheNode)
		{
			if (ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!nxrmfltPreCreate found file %wZ in cache\n", &NameInfo->Name));

				Flags = pCacheNode->Flags;

				CtxAttached = BooleanFlagOn(Flags, NXRMFLT_FLAG_CTX_ATTACHED);
			}
			else
			{
				pCacheNode = NULL;
			}
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

		//
		// It's OK to use pCacheNode because we acquired rundown protection
		//
		if (pCacheNode && CtxAttached == FALSE)
		{
			do 
			{
				IsExplorer = is_explorer();

				if (Global.ClientPort)
				{
					Status = nxrmfltCheckRights((HANDLE)FltGetRequestorProcessId(Data),
												Data->Thread?PsGetThreadId(Data->Thread) : PsGetCurrentThreadId(),
												pCacheNode,
												FALSE,
												&RightsMask,
												NULL,
												NULL);
				}
				else
				{
					Status = STATUS_SUCCESS;
					RightsMask = 0;
				}

				if ((Status == STATUS_SUCCESS && (!(RightsMask & BUILTIN_RIGHT_VIEW))) || IsExplorer)	// always gives explorer NXL file instead of decrypted file even explorer has rights
				{
					Status = nxrmfltBlockPreCreate(Data, 
												   &pCacheNode->OriginalFileName, 
												   NameInfo);

					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to give encrypted content %wZ to PID %d! Status is %x\n", &pCacheNode->OriginalFileName, FltGetRequestorProcessId(Data), Status));

						Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
						Data->IoStatus.Information	= 0;
					}

					CallbackStatus = FLT_PREOP_COMPLETE;
					SkipProcessingAfterCheckingRights = TRUE;
					break;
				}

				if (Status == STATUS_SUCCESS && (!(RightsMask & BUILTIN_RIGHT_EDIT)) && (RightsMask != 0))
				{
					if (FlagOn(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess, FILE_WRITE_DATA) ||
						(CreateDisposition == FILE_SUPERSEDE) ||
						(CreateDisposition == FILE_OVERWRITE) ||
						(CreateDisposition == FILE_OVERWRITE_IF))
					{
						Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
						Data->IoStatus.Information	= 0;
						CallbackStatus				= FLT_PREOP_COMPLETE;

						nxrmfltSendBlockNotificationMsg(Data, &NameInfo->Name, nxrmfltDeniedWritesOpen);

						SkipProcessingAfterCheckingRights = TRUE;
						break;
					}
				}

			} while (FALSE);

			if (SkipProcessingAfterCheckingRights)
			{
				break;
			}

			do 
			{
				//
				// file exists but no ctx attached to it
				// must be new open
				//
				SetFlag(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);

				Status = nxrmfltDecryptFile(Data,
											FltObjects->Instance,
											FltObjects->Instance,
											&pCacheNode->OriginalFileName,
											&pCacheNode->ReparseFileName);

				if (!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,("nxrmflt!nxrmfltPreCreate: Failed to decrypt file %wZ! Status is %x\n", &pCacheNode->OriginalFileName, Status));
					break;
				}

				Status = nxrmfltAddReparseECP(Data, NameInfo, FltObjects->Instance, NULL);

				if (!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,("nxrmflt!nxrmfltPreCreate: Failed to add ECP to %wZ! Status is %x\n", &NameInfo->Name, Status));
					break;
				}

				//
				// Only return STATUS_REPARSE when the opening file is NOT the same as re-parsed file
				//
				if (RtlEqualUnicodeString(&NameInfo->Name, &pCacheNode->ReparseFileName, TRUE))
				{
					CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
				}
				else
				{
					Status = IoReplaceFileObjectName(Data->Iopb->TargetFileObject,
													 pCacheNode->ReparseFileName.Buffer,
													 pCacheNode->ReparseFileName.Length);
					if (!NT_SUCCESS(Status))
					{
						//
						// remove ECP in this case
						//
						nxrmfltDeleteAddedReparseECP(Data);

						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to reparse %wZ! Status is %x\n", &NameInfo->Name, Status));
						break;
					}

					Data->IoStatus.Status = STATUS_REPARSE;
					Data->IoStatus.Information = IO_REPARSE;
					CallbackStatus = FLT_PREOP_COMPLETE;
				}

			} while (FALSE);

			if (!NT_SUCCESS(Status))
			{
				//
				// clear flag in the case of error
				//
				ClearFlag(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);

				Status = nxrmfltBlockPreCreate(Data,
											   &pCacheNode->OriginalFileName,
											   NameInfo);

				if (!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to give encrypted content %wZ to PID %d! Status is %x\n", &pCacheNode->OriginalFileName, FltGetRequestorProcessId(Data), Status));
					
					Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
					Data->IoStatus.Information	= 0;
				}

				CallbackStatus	= FLT_PREOP_COMPLETE;
				
				break;
			}

			ReleaseNameInfo = FALSE;

			break;
		}
		else if (pCacheNode)
		{
			do 
			{
				IsExplorer = is_explorer();

				IsExplorerDeleteFile = (IsExplorer && (CreateDisposition == FILE_OPEN && FlagOn(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess, DELETE)));

				IsExplorerRequiringOpLock = (IsExplorer && (CreateDisposition == FILE_OPEN && FlagOn(CreateOptions, FILE_OPEN_REQUIRING_OPLOCK)));

				if (Global.ClientPort)
				{
					Status = nxrmfltCheckRights((HANDLE)FltGetRequestorProcessId(Data),
												Data->Thread?PsGetThreadId(Data->Thread) : PsGetCurrentThreadId(),
												pCacheNode,
												FALSE,
												&RightsMask,
												NULL,
												NULL);
				}
				else
				{
					Status = STATUS_SUCCESS;
					RightsMask = 0;
				}

				if (IsExplorer && IsExplorerRequiringOpLock && Data->Iopb->Parameters.Create.ShareAccess != FILE_SHARE_VALID_FLAGS)
				{
					Data->IoStatus.Status		= STATUS_SHARING_VIOLATION;
					Data->IoStatus.Information	= 0;
					CallbackStatus				= FLT_PREOP_COMPLETE;

					SkipProcessingAfterCheckingRights = TRUE;
					break;
				}

				if ((Status == STATUS_SUCCESS && (!(RightsMask & BUILTIN_RIGHT_VIEW))) || ((IsExplorer) && (!IsExplorerDeleteFile)))
				{
					Status = nxrmfltBlockPreCreate(Data,
												   &pCacheNode->OriginalFileName,
												   NameInfo);
					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to give encrypted content %wZ to PID %d! Status is %x\n", &pCacheNode->OriginalFileName, FltGetRequestorProcessId(Data), Status));

						Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
						Data->IoStatus.Information	= 0;
					}

					CallbackStatus = FLT_PREOP_COMPLETE;
					SkipProcessingAfterCheckingRights = TRUE;
					break;
				}
				else if (IsExplorer)
				{
					//
					// explorer delete NXL file or explorer requiring OpLock
					//
					Status = nxrmfltForceAccessCheck(FltObjects->Instance,
													 &NameInfo->Name,
													 Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess,
													 Data->Iopb->Parameters.Create.FileAttributes,
													 Data->Iopb->Parameters.Create.ShareAccess,
													 CreateOptions);

					if (!NT_SUCCESS(Status) && (Status != STATUS_OBJECT_NAME_NOT_FOUND))
					{
						Data->IoStatus.Status		= Status;
						Data->IoStatus.Information	= 0;
						CallbackStatus				= FLT_PREOP_COMPLETE;

						SkipProcessingAfterCheckingRights = TRUE;
						break;
					}
					else
					{
						Status = nxrmfltBlockPreCreate(Data,
													   &pCacheNode->OriginalFileName,
													   NameInfo);
						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to give encrypted content %wZ to PID %d! Status is %x\n", &pCacheNode->OriginalFileName, FltGetRequestorProcessId(Data), Status));

							Data->IoStatus.Status = STATUS_ACCESS_DENIED;
							Data->IoStatus.Information = 0;
						}

						CallbackStatus = FLT_PREOP_COMPLETE;
						SkipProcessingAfterCheckingRights = TRUE;
						break;
					}
				}

				if (Status == STATUS_SUCCESS && (!(RightsMask & BUILTIN_RIGHT_EDIT)) && (RightsMask != 0))
				{
					if (FlagOn(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess, FILE_WRITE_DATA) ||
						(CreateDisposition == FILE_SUPERSEDE) ||
						(CreateDisposition == FILE_OVERWRITE) ||
						(CreateDisposition == FILE_OVERWRITE_IF))
					{
						Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
						Data->IoStatus.Information	= 0;
						CallbackStatus				= FLT_PREOP_COMPLETE;

						nxrmfltSendBlockNotificationMsg(Data, &NameInfo->Name, nxrmfltDeniedWritesOpen);

						SkipProcessingAfterCheckingRights = TRUE;
						break;
					}
				}

			} while (FALSE);

			if (SkipProcessingAfterCheckingRights)
			{
				break;
			}
			//
			// do whatever we need to do
			//
			
			//////////////////////////////////////////////////////////////////////////
			//
			// at this point, we need to see PostCreate if CreateDisposition is to
			// "Overwrite" the file. We only do it here where Ctx has been attached
			// to this file.
			// 
			//////////////////////////////////////////////////////////////////////////

			if ((CreateDisposition == FILE_SUPERSEDE) ||
				(CreateDisposition == FILE_OVERWRITE) ||
				(CreateDisposition == FILE_OVERWRITE_IF))
			{
				CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
			}

		}
		else if (IsNXLFile(&NameInfo->Extension))
		{
			UNICODE_STRING FileNameWithoutNXLExtension = { 0 };

			BOOLEAN FoundInCache = FALSE;

			NXLFILE_CREATE_CONTEXT *CreateCtx = NULL;

			//
			// ignore opening NXL file second stream if NXL file is in cache 
			//
			if (NameInfo->Stream.Length)
			{
				break;
			}

			FileNameWithoutNXLExtension.Buffer = NameInfo->Name.Buffer;
			FileNameWithoutNXLExtension.Length = NameInfo->Name.Length - 4 * sizeof(WCHAR);
			FileNameWithoutNXLExtension.MaximumLength = NameInfo->Name.MaximumLength;

			FltAcquirePushLockShared(&Global.NxlFileCacheLock);

			do 
			{
				if (FindNXLNodeInCache(&Global.NxlFileCache, &FileNameWithoutNXLExtension))
				{
					FoundInCache = TRUE;
				}
			} while (FALSE);

			FltReleasePushLock(&Global.NxlFileCacheLock);

			//
			// ignore opening NXL file in cache if it's not truncate content
			//
			if (FoundInCache)
			{
				if ((CreateDisposition != FILE_SUPERSEDE) &&
					(CreateDisposition != FILE_OVERWRITE) &&
					(CreateDisposition != FILE_OVERWRITE_IF))
				{
					Status = IoReplaceFileObjectName(Data->Iopb->TargetFileObject,
													 FileNameWithoutNXLExtension.Buffer,
													 FileNameWithoutNXLExtension.Length);
					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to force application with PID %d to open %wZ! Status is %x\n", FltGetRequestorProcessId(Data), &FileNameWithoutNXLExtension, Status));

						Data->IoStatus.Status = STATUS_ACCESS_DENIED;
						Data->IoStatus.Information = 0;
						CallbackStatus = FLT_PREOP_COMPLETE;

						break;
					}

					Data->IoStatus.Status = STATUS_REPARSE;
					Data->IoStatus.Information = IO_REPARSE;
					CallbackStatus = FLT_PREOP_COMPLETE;

					break;
				}

				Status = nxrmfltForceAccessCheck(FltObjects->Instance,
												 &FileNameWithoutNXLExtension,
												 Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess,
												 Data->Iopb->Parameters.Create.FileAttributes,
												 Data->Iopb->Parameters.Create.ShareAccess,
												 CreateOptions);

				if (!NT_SUCCESS(Status) && (Status != STATUS_OBJECT_NAME_NOT_FOUND))
				{
					Data->IoStatus.Status		= Status;
					Data->IoStatus.Information	= 0;
					CallbackStatus = FLT_PREOP_COMPLETE;

					break;
				}

				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: Failed to force check with PID %d Status is %x\n", FltGetRequestorProcessId(Data), Status));
			}

			
			// 
			// this means application is opening a NXL file that not in our cache
			// we need to exam this file in PostCreate
			//
			CreateCtx = ExAllocateFromPagedLookasideList(&Global.NXLFileCreateCtxLookaside);

			if (!CreateCtx)
			{
				break;
			}

			CreateCtx->Flags |= FoundInCache ? NXRMFLT_NXLFILE_CREATE_CTX_FLAG_FILE_IS_IN_CACHE : 0;
			CreateCtx->NameInfo = NameInfo;
			CreateCtx->FileNameWithoutNXLExtension = FileNameWithoutNXLExtension;

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreCreate: check PostCreate file %wZ! \n", &FileNameWithoutNXLExtension));

			//
			// prevent releasing NameInfo
			//
			ReleaseNameInfo = FALSE;

			*CompletionContext = CreateCtx;
			
			CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

			break;
		}
		else
		{
			//
			// in this case, we need to check SaveAs forecast to see if this "Create" in SaveAs forecast
			//
			FltAcquirePushLockShared(&Global.ExpireTableLock);

			FOR_EACH_LIST(ite, &Global.ExpireTable)
			{
				pSaveAsNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

				if (0 == RtlCompareUnicodeString(&pSaveAsNode->SaveAsFileName, &NameInfo->Name, TRUE) && 
					pSaveAsNode->ProcessId == (HANDLE)FltGetRequestorProcessId(Data))
				{
					if (!ExAcquireRundownProtection(&pSaveAsNode->NodeRundownRef))
					{
						pSaveAsNode = NULL;
					}

					break;
				}
				else if (NameInfo->Share.Length &&
						 pSaveAsNode->ProcessId == (HANDLE)FltGetRequestorProcessId(Data))
				{
					UNICODE_STRING MupDevice = {0};

					RtlInitUnicodeString(&MupDevice, NXRMFLT_MUP_PREFIX);

					if (!RtlPrefixUnicodeString(&MupDevice, &NameInfo->Name, TRUE))
					{
						pSaveAsNode = NULL;
						continue;
					}

					if (!CompareMupDeviceNames(&pSaveAsNode->SaveAsFileName, &NameInfo->Name))
					{
						pSaveAsNode = NULL;
						continue;
					}
					
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

			if (!pSaveAsNode)
			{
				break;
			}

			//
			// Check source file
			// Output of following block of code is "pSaveAsSourceCacheNode" which represents source file
			//
			do 
			{
				if (pSaveAsNode->SourceFileName.Length)
				{
					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pSaveAsSourceCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &pSaveAsNode->SourceFileName);

					if (pSaveAsSourceCacheNode)
					{
						if (!ExAcquireRundownProtection(&pSaveAsSourceCacheNode->NodeRundownRef))
						{
							pSaveAsSourceCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);

					break;
				}
				else
				{
					//
					// comment out following temporarily to avoid encrypting file if we don't know the soruce file
					//
					//if (STATUS_NOT_FOUND == nxrmfltGuessSourceFileFromProcessCache((HANDLE)FltGetRequestorProcessId(Data), &pSaveAsNode->SourceFileName))
					//{
					//	pSaveAsSourceCacheNode = NULL;
					//	break;
					//}

					//FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					//pSaveAsSourceCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &pSaveAsNode->SourceFileName);

					//if (pSaveAsSourceCacheNode)
					//{
					//	if (!ExAcquireRundownProtection(&pSaveAsSourceCacheNode->NodeRundownRef))
					//	{
					//		pSaveAsSourceCacheNode = NULL;
					//	}
					//}
					//
					//FltReleasePushLock(&Global.NxlFileCacheLock);
				}

			} while (FALSE);

			if (!pSaveAsSourceCacheNode)
			{
				//
				// Source file is NOT a NXL file or this process is not dirty
				// Let it go
				//
				break;
			}

			//
			// we need to check rights for this SaveAs operation
			//
			if (!Global.ClientPort)
			{
				//
				// service stopped?
				//
				break;
			}

			Status = nxrmfltCheckRights((HANDLE)FltGetRequestorProcessId(Data),
										Data->Thread?PsGetThreadId(Data->Thread) : PsGetCurrentThreadId(),
										pSaveAsSourceCacheNode,
										FALSE,
										&RightsMask,
										NULL,
										NULL);

			if (Status == STATUS_SUCCESS && RightsMask == 0)
			{
				Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
				Data->IoStatus.Information	= 0;
				CallbackStatus				= FLT_PREOP_COMPLETE;

				break;
			}

			if (Status == STATUS_SUCCESS && (!(RightsMask & BUILTIN_RIGHT_SAVEAS)) && (RightsMask != 0))
			{
				if (FlagOn(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess, FILE_WRITE_DATA) ||
					(CreateDisposition == FILE_SUPERSEDE) ||
					(CreateDisposition == FILE_OVERWRITE) ||
					(CreateDisposition == FILE_OVERWRITE_IF))
				{
					Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
					Data->IoStatus.Information	= 0;
					CallbackStatus				= FLT_PREOP_COMPLETE;

					nxrmfltSendBlockNotificationMsg(Data, &NameInfo->Name, nxrmfltDeniedSaveAsOpen);

					break;
				}
			}

			Status = FltGetInstanceContext(FltObjects->Instance, &InstCtx);

			if (!NT_SUCCESS(Status))
			{
				//
				// Source file is NXL file or Process is dirty but we can't get instance ctx
				// deny access
				//
				Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
				Data->IoStatus.Information	= 0;
				CallbackStatus				= FLT_PREOP_COMPLETE;

				nxrmfltSendBlockNotificationMsg(Data, &NameInfo->Name, nxrmfltSaveAsToUnprotectedVolume);

				break;
			}

			if (InstCtx->DisableFiltering)
			{
				//
				// Source file is NXL file or Process is dirty but we don't filtering this volume
				// deny access
				//
				Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
				Data->IoStatus.Information	= 0;
				CallbackStatus				= FLT_PREOP_COMPLETE;

				nxrmfltSendBlockNotificationMsg(Data, &NameInfo->Name, nxrmfltSaveAsToUnprotectedVolume);

				break;
			}

			//
			// Adding ECP
			//
			Status = nxrmfltAddReparseECP(Data, NameInfo, FltObjects->Instance, &pSaveAsSourceCacheNode->FileName);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,("nxrmflt!nxrmfltPreCreate: Failed to add ECP to %wZ! Status is %x\n", &NameInfo->Name, Status));

				Data->IoStatus.Status		= STATUS_ACCESS_DENIED;
				Data->IoStatus.Information	= 0;
				CallbackStatus				= FLT_PREOP_COMPLETE;

				break;
			}

			ReleaseNameInfo = FALSE;

			*CompletionContext = NULL;

			CallbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

		}

	} while (FALSE);

	if (ReleaseNameInfo && NameInfo)
	{
		FltReleaseFileNameInformation(NameInfo);
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	if (pCacheNode)
	{
		ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
	}
	
	if (pSaveAsSourceCacheNode)
	{
		ExReleaseRundownProtection(&pSaveAsSourceCacheNode->NodeRundownRef);
	}

	if(ReparseFileNameBuffer)
	{
		ExFreeToPagedLookasideList(&Global.FullPathLookaside, ReparseFileNameBuffer);
	}

	if (pSaveAsNode)
	{
		ExReleaseRundownProtection(&pSaveAsNode->NodeRundownRef);
	}

	return CallbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS nxrmfltPostCreate(
	_Inout_ PFLT_CALLBACK_DATA		Data,
	_In_ PCFLT_RELATED_OBJECTS		FltObjects,
	_In_opt_ PVOID					CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS	Flags)
{
	NTSTATUS Status = STATUS_SUCCESS;
	FLT_POSTOP_CALLBACK_STATUS	CallbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

	NXRMFLT_STREAM_CONTEXT *Ctx = NULL;
	NXRMFLT_STREAMHANDLE_CONTEXT *Ccb = NULL;

	PFLT_INSTANCE	FltInstance = NULL;
	PFILE_OBJECT	FileObject = NULL;
	NXRMFLT_REPARSE_ECP_CONTEXT *EcpCtx = NULL;

	NXL_CACHE_NODE *pCacheNode = NULL;

	ULONG DirHash = 0;

	LARGE_INTEGER FileId = {0};
	ULONG FileAttributes = 0;

	NXL_KEKEY_BLOB	PrimaryKey = {0};

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	LIST_ENTRY	ExpiredNodeList = {0};

	ULONG CreateDisposition = 0;
	ULONG CreateOptions = 0;

	NXLFILE_CREATE_CONTEXT *CreateCtx = NULL;

	do 
	{

		FltInstance = FltObjects->Instance;
		FileObject = FltObjects->FileObject;

		CreateDisposition = (Data->Iopb->Parameters.Create.Options & 0xff000000) >> 24;
		CreateOptions = (Data->Iopb->Parameters.Create.Options & 0x00ffffff);

		if (CompletionContext)
		{
			CreateCtx = (NXLFILE_CREATE_CONTEXT *)CompletionContext;

			do 
			{
				if (FlagOn(CreateCtx->Flags, NXRMFLT_NXLFILE_CREATE_CTX_FLAG_FILE_IS_IN_CACHE))
				{
					if (((CreateDisposition == FILE_SUPERSEDE) ||
						(CreateDisposition == FILE_OVERWRITE) ||
						(CreateDisposition == FILE_OVERWRITE_IF)) && Data->IoStatus.Status == STATUS_SUCCESS)
					{
						ULONG FileNameHash = 0;

						//
						// NXL file has been truncated
						//
						//
						// purge rights cache
						//

						RtlHashUnicodeString(&CreateCtx->FileNameWithoutNXLExtension, TRUE, HASH_STRING_ALGORITHM_X65599, &FileNameHash);

						nxrmfltPurgeRightsCache(FltInstance, FileNameHash);

						//
						// remove ctx attached flags
						//
						FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

						do
						{
							NXL_CACHE_NODE *pTmpNode = NULL;

							pTmpNode = FindNXLNodeInCache(&Global.NxlFileCache, &CreateCtx->FileNameWithoutNXLExtension);

							if(pTmpNode)
							{
								ClearFlag(pTmpNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
							}

						} while (FALSE);

						FltReleasePushLock(&Global.NxlFileCacheLock);

						//
						// delete the decrypted copy
						//
						nxrmfltDeleteFileByName(FltInstance, &CreateCtx->FileNameWithoutNXLExtension);
					}

					break;
				}

				if (Data->IoStatus.Status != STATUS_SUCCESS || 
					Data->IoStatus.Information != FILE_OPENED)
				{
					//
					// if it's not successfully open an existing one ...
					//
					break;
				}

				RtlHashUnicodeString(&(CreateCtx->NameInfo->ParentDir), TRUE, HASH_STRING_ALGORITHM_X65599, &DirHash);

				//
				// Calling this function before acquiring the lock because this function generates I/O.
				//
				get_file_id_and_attribute_ex(FltInstance, FileObject, &FileId, &FileAttributes);

				FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

				do
				{
					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &(CreateCtx->FileNameWithoutNXLExtension));

					if (pCacheNode)
					{
						//
						// This is NOT expected
						//
						pCacheNode = NULL;
						break;
					}

					//
					// This is expected
					//
					pCacheNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

					if (!pCacheNode)
					{
						//
						// very unlikely, out of memory?
						//
						break;
					}

					memset(pCacheNode, 0, sizeof(NXL_CACHE_NODE));

					Status = build_nxlcache_file_name(pCacheNode, &(CreateCtx->FileNameWithoutNXLExtension));

					if (!NT_SUCCESS(Status))
					{
						//
						// ERROR case. No memory
						//
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &(CreateCtx->FileNameWithoutNXLExtension)));

						ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pCacheNode);
						pCacheNode = NULL;
						break;
					}

					pCacheNode->FileAttributes = FileAttributes;
					pCacheNode->FileID.QuadPart = FileId.QuadPart;
					pCacheNode->Flags |= FlagOn(FileAttributes, FILE_ATTRIBUTE_READONLY) ? NXRMFLT_FLAG_READ_ONLY : 0;
					pCacheNode->Flags |= NXRMFLT_FLAG_ATTACHING_CTX;
					pCacheNode->Instance = FltObjects->Instance;
					pCacheNode->ParentDirectoryHash = DirHash;
					pCacheNode->OnRemoveOrRemovableMedia = FALSE;

					ExInitializeRundownProtection(&pCacheNode->NodeRundownRef);

					Status = build_nxlcache_reparse_file_name(pCacheNode, &(CreateCtx->FileNameWithoutNXLExtension));

					if (!NT_SUCCESS(Status))
					{
						//
						// ERROR case. No memory
						//
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &(CreateCtx->FileNameWithoutNXLExtension)));

						if (pCacheNode->ReleaseFileName)
						{
							ExFreePoolWithTag(pCacheNode->FileName.Buffer, NXRMFLT_NXLCACHE_TAG);
							RtlInitUnicodeString(&pCacheNode->FileName, NULL);
						}

						ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pCacheNode);
						pCacheNode = NULL;
						break;
					}

					PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!AddNXLNodeToCache add file %wZ into cache\n", &(CreateCtx->FileNameWithoutNXLExtension)));

					AddNXLNodeToCache(&Global.NxlFileCache, pCacheNode);

					pCacheNode = NULL;

				} while (FALSE);

				FltReleasePushLock(&Global.NxlFileCacheLock);

			} while (FALSE);

			//
			// let's break for now. it does not make sense to continue at this time
			//
			break;
		}

		Status = FltGetStreamContext(FltInstance, FileObject, &Ctx);

		if (Ctx)
		{
			if (((CreateDisposition == FILE_SUPERSEDE) ||
				 (CreateDisposition == FILE_OVERWRITE) ||
				 (CreateDisposition == FILE_OVERWRITE_IF)) && Data->IoStatus.Status == STATUS_SUCCESS)
			{
				Ctx->ContentDirty = TRUE;
			}

			EcpCtx = (NXRMFLT_REPARSE_ECP_CONTEXT *)nxrmfltGetReparseECP(Data);

			if (EcpCtx)
			{
				FltAcknowledgeEcp(Global.Filter, EcpCtx);
			}

			break;
		}

		if (FlagOn(CreateOptions, FILE_DELETE_ON_CLOSE))
		{
			//
			// add stream handle context to track delete on close
			//
			Status = FltAllocateContext(Global.Filter,
										FLT_STREAMHANDLE_CONTEXT,
										sizeof(NXRMFLT_STREAMHANDLE_CONTEXT),
										NonPagedPool,
										(PFLT_CONTEXT*)&Ccb);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostCreate: Failed to allocate stream handle ctx!\n"));

				break;
			}

			Ccb->DestinationFileNameInfo = NULL;
			Ccb->SourceFileNameInfo = NULL;
			Ccb->SourceFileIsNxlFile = FALSE;
			Ccb->EncryptDestinationFile = FALSE;
			Ccb->DeleteOnClose = TRUE;
			Ccb->Reserved = 0;
			
			Status = FltSetStreamHandleContext(FltInstance,
											   FileObject,
											   FLT_SET_CONTEXT_KEEP_IF_EXISTS,
											   Ccb,
											   NULL);

			if (!NT_SUCCESS(Status))
			{
				//
				// ignore the error and continue
				//
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostCreate: FltSetStreamHandleContext return %x\n", Status));
			}

		}

		EcpCtx = (NXRMFLT_REPARSE_ECP_CONTEXT *)nxrmfltGetReparseECP(Data);

		if (!EcpCtx)
		{
			break;
		}

		FltAcknowledgeEcp(Global.Filter, EcpCtx);

		if ((Data->IoStatus.Status != STATUS_SUCCESS && Data->IoStatus.Status != STATUS_OBJECT_NAME_COLLISION))
		{
			break;
		}

		//
		// Create NXL cache node if it's a SaveAs Open
		//

		if (EcpCtx->SourceFileName.Length)
		{
			RtlHashUnicodeString(&(EcpCtx->NameInfo->ParentDir), TRUE, HASH_STRING_ALGORITHM_X65599, &DirHash);

			//
			// Calling this function before acquiring the lock because this function generates I/O.
			//
			get_file_id_and_attribute_ex(FltInstance, FileObject, &FileId, &FileAttributes);

			FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

			do
			{
				pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &(EcpCtx->NameInfo->Name));

				if (pCacheNode)
				{
					//
					// This is NOT expected
					//
					pCacheNode = NULL;
					break;
				}

				//
				// This is expected
				//
				pCacheNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

				if (!pCacheNode)
				{
					//
					// very unlikely, out of memory?
					//
					break;
				}

				memset(pCacheNode, 0, sizeof(NXL_CACHE_NODE));

				Status = build_nxlcache_file_name(pCacheNode, &(EcpCtx->NameInfo->Name));

				if (!NT_SUCCESS(Status))
				{
					//
					// ERROR case. No memory
					//
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &(EcpCtx->NameInfo->Name)));

					ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pCacheNode);
					pCacheNode = NULL;
					break;
				}

				pCacheNode->FileAttributes = FileAttributes;
				pCacheNode->FileID.QuadPart = FileId.QuadPart;
				pCacheNode->Flags |= FlagOn(FileAttributes, FILE_ATTRIBUTE_READONLY) ? NXRMFLT_FLAG_READ_ONLY : 0;
				pCacheNode->Flags |= NXRMFLT_FLAG_ATTACHING_CTX;
				pCacheNode->Instance = FltObjects->Instance;
				pCacheNode->ParentDirectoryHash = DirHash;
				pCacheNode->OnRemoveOrRemovableMedia = FALSE;

				ExInitializeRundownProtection(&pCacheNode->NodeRundownRef);

				Status = build_nxlcache_reparse_file_name(pCacheNode, &(EcpCtx->NameInfo->Name));

				if (!NT_SUCCESS(Status))
				{
					//
					// ERROR case. No memory
					//
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", &(EcpCtx->NameInfo->Name)));

					if (pCacheNode->ReleaseFileName)
					{
						ExFreePoolWithTag(pCacheNode->FileName.Buffer, NXRMFLT_NXLCACHE_TAG);
						RtlInitUnicodeString(&pCacheNode->FileName, NULL);
					}

					ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pCacheNode);
					pCacheNode = NULL;
					break;
				}

				PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!AddNXLNodeToCache add file %wZ into cache\n", &(EcpCtx->NameInfo->Name)));

				AddNXLNodeToCache(&Global.NxlFileCache, pCacheNode);

				if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
				{
					pCacheNode = NULL;
				}

			} while (FALSE);

			FltReleasePushLock(&Global.NxlFileCacheLock);

			if (pCacheNode)
			{
				NXL_CACHE_NODE *pSrcCacheNode = NULL;

				FltAcquirePushLockShared(&Global.PrimaryKeyLock);

				memcpy(&PrimaryKey, 
					   &Global.PrimaryKey, 
					   min(sizeof(PrimaryKey),sizeof(Global.PrimaryKey)));

				FltReleasePushLock(&Global.PrimaryKeyLock);

				Status = NXLCreateFile(Global.Filter,
									   FltInstance,
									   &pCacheNode->OriginalFileName,
									   &PrimaryKey,
									   NULL,
									   TRUE);

				if (!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't create NXL file %wZ\n", &pCacheNode->OriginalFileName));
				}

				FltAcquirePushLockShared(&Global.NxlFileCacheLock);

				pSrcCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &EcpCtx->SourceFileName);

				if (pSrcCacheNode)
				{
					if (!ExAcquireRundownProtection(&pSrcCacheNode->NodeRundownRef))
					{
						pSrcCacheNode = NULL;
					}
				}

				FltReleasePushLock(&Global.NxlFileCacheLock);

				if (pSrcCacheNode)
				{
					Status = nxrmfltCopyTags(pSrcCacheNode->Instance,
											 &pSrcCacheNode->OriginalFileName,
											 pCacheNode->Instance,
											 &pCacheNode->OriginalFileName);

					if (!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't copy tags from file %wZ to %wZ\n", &pSrcCacheNode->OriginalFileName, &pCacheNode->OriginalFileName));
					}

					ExReleaseRundownProtection(&pSrcCacheNode->NodeRundownRef);

					pSrcCacheNode = NULL;
				}

				ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);

				pCacheNode = NULL;
			}
		}

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

		Status = nxrmfltBuildNamesInStreamContext(Ctx, &EcpCtx->NameInfo->Name);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		Ctx->OriginalInstance			= EcpCtx->OriginalInstance;

		Status = FltGetRequestorSessionId(Data, &Ctx->RequestorSessionId);

		if (!NT_SUCCESS(Status))
		{
			Ctx->RequestorSessionId = NXRMFLT_INVALID_SESSION_ID;
		}

		FltInitializePushLock(&Ctx->CtxLock);

		if (((CreateDisposition == FILE_SUPERSEDE) ||
			 (CreateDisposition == FILE_OVERWRITE) ||
			 (CreateDisposition == FILE_OVERWRITE_IF)) && Data->IoStatus.Status == STATUS_SUCCESS)
		{
			Ctx->ContentDirty = TRUE;
		}

		//
		// force sync when SaveAs
		//
		if (EcpCtx->SourceFileName.Length)
		{
			Ctx->ContentDirty = TRUE;
		}

		Status = FltSetStreamContext(FltInstance,
									 FileObject,
									 FLT_SET_CONTEXT_KEEP_IF_EXISTS,
									 Ctx,
									 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		FltAcquirePushLockShared(&Global.NxlFileCacheLock);

		pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ctx->FileName);

		if(pCacheNode)
		{
			SetFlag(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
			ClearFlag(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);
		}
		
		FltReleasePushLock(&Global.NxlFileCacheLock);

		//
		// Remove SaveAsNode from expire table
		//

		//InitializeListHead(&ExpiredNodeList);

		//if (EcpCtx->SourceFileName.Length)
		//{
		//	FltAcquirePushLockExclusive(&Global.ExpireTableLock);

		//	FOR_EACH_LIST_SAFE(ite, tmp, &Global.ExpireTable)
		//	{
		//		NXL_SAVEAS_NODE *pSaveAsNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

		//		if (0 == RtlCompareUnicodeString(&pSaveAsNode->SaveAsFileName, &(EcpCtx->NameInfo->Name), TRUE) &&
		//			pSaveAsNode->ProcessId == (HANDLE)FltGetRequestorProcessId(Data))
		//		{
		//			RemoveEntryList(ite);

		//			InsertHeadList(&ExpiredNodeList, &pSaveAsNode->Link);
		//		}
		//	}

		//	FltReleasePushLock(&Global.ExpireTableLock);

		//	FOR_EACH_LIST_SAFE(ite, tmp, &ExpiredNodeList)
		//	{
		//		NXL_SAVEAS_NODE *pSaveAsNode = CONTAINING_RECORD(ite, NXL_SAVEAS_NODE, Link);

		//		RemoveEntryList(ite);

		//		//
		//		// Wait for all other threads rundown
		//		//
		//		ExWaitForRundownProtectionRelease(&pSaveAsNode->NodeRundownRef);

		//		ExRundownCompleted(&pSaveAsNode->NodeRundownRef);

		//		memset(pSaveAsNode, 0, sizeof(NXL_SAVEAS_NODE));

		//		ExFreeToPagedLookasideList(&Global.SaveAsExpireLookaside, pSaveAsNode);
		//	}
		//}

	} while (FALSE);

	//
	// It's possible that we need to release ECP nameinfo and release Ctx
	// This happens when two creates happen at the same time
	//
	if(EcpCtx && EcpCtx->NameInfo)
	{
		FltReleaseFileNameInformation(EcpCtx->NameInfo);
	}

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	if (Ccb)
	{
		FltReleaseContext(Ccb);
	}

	if (CreateCtx)
	{
		FltReleaseFileNameInformation(CreateCtx->NameInfo);

		ExFreeToPagedLookasideList(&Global.NXLFileCreateCtxLookaside, CreateCtx);
	}

	return CallbackStatus;
}

BOOLEAN IsNXLFile(PUNICODE_STRING Extension)
{
	BOOLEAN bRet = FALSE;

	do 
	{

		bRet = (RtlCompareUnicodeString(&Global.NXLFileExtsion, Extension, TRUE) == 0);

	} while (FALSE);

	return bRet;
}

static void ascii2lower(UNICODE_STRING *str)
{
	WCHAR *p = str->Buffer;

	while (*p && (ULONG_PTR)p < (ULONG_PTR)((UCHAR*)str->Buffer + str->Length))
	{
		if (*p >= L'A' && *p <= L'Z')
		{
			*p += L'a' - L'A';
		}

		p++;
	}
}

static BOOLEAN IsReparseEcpPresent(PFLT_CALLBACK_DATA Data)
{
	NTSTATUS status = STATUS_SUCCESS;
	PECP_LIST ecpList = NULL;

	status = FltGetEcpListFromCallbackData(Global.Filter, Data, &ecpList);

	if (NT_SUCCESS(status) && (ecpList != NULL))
	{

		status = FltFindExtraCreateParameter(Global.Filter,
											 ecpList,
											 &GUID_ECP_NXRMFLT_REPARSE,
											 NULL,
											 NULL);

		if (NT_SUCCESS(status))
		{
			return TRUE;
		}
	}

	return FALSE;
}

static BOOLEAN IsBlockingEcpPresent(PFLT_CALLBACK_DATA Data)
{
	NTSTATUS status = STATUS_SUCCESS;
	PECP_LIST ecpList = NULL;

	status = FltGetEcpListFromCallbackData(Global.Filter, Data, &ecpList);

	if (NT_SUCCESS(status) && (ecpList != NULL))
	{

		status = FltFindExtraCreateParameter(Global.Filter,
											 ecpList,
											 &GUID_ECP_NXRMFLT_BLOCKING,
											 NULL,
											 NULL);

		if (NT_SUCCESS(status))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN is_explorer(void)
{
	BOOLEAN bRet = FALSE;

	CHAR *p = NULL;

	do 
	{
		p = PsGetProcessImageFileName(PsGetCurrentProcess());

		if(p)
		{
			if(_stricmp(p,"explorer.exe") == 0)
			{
				bRet = TRUE;
			}
		}

	} while (FALSE);

	return bRet;
}

static BOOLEAN CompareMupDeviceNames(PUNICODE_STRING FileName, PUNICODE_STRING OpeningMupName)
{
	BOOLEAN bRet = FALSE;
	
	UNICODE_STRING FileNameWithoutDevice = {0};

	UNICODE_STRING DevicePrefix = {0};

	UNICODE_STRING OpeningMupNameWithoutDeviceMup = { 0 };

	do 
	{
		DevicePrefix.Buffer			= NXRMFLT_DEVICE_PREFIX;
		DevicePrefix.Length			= sizeof(NXRMFLT_DEVICE_PREFIX) - sizeof(WCHAR);
		DevicePrefix.MaximumLength	= sizeof(NXRMFLT_DEVICE_PREFIX) - sizeof(WCHAR);

		if (RtlPrefixUnicodeString(&DevicePrefix, FileName, TRUE))
		{
			FileNameWithoutDevice.Buffer		= FileName->Buffer + (sizeof(NXRMFLT_DEVICE_PREFIX) - sizeof(WCHAR))/sizeof(WCHAR);
			FileNameWithoutDevice.Length		= FileName->Length - (sizeof(NXRMFLT_DEVICE_PREFIX) - sizeof(WCHAR));
			FileNameWithoutDevice.MaximumLength = FileName->MaximumLength - (sizeof(NXRMFLT_DEVICE_PREFIX) - sizeof(WCHAR));

			OpeningMupNameWithoutDeviceMup.Buffer			= OpeningMupName->Buffer + (sizeof(NXRMFLT_MUP_PREFIX) - sizeof(WCHAR)) / sizeof(WCHAR);
			OpeningMupNameWithoutDeviceMup.Length			= OpeningMupName->Length - (sizeof(NXRMFLT_MUP_PREFIX) - sizeof(WCHAR));
			OpeningMupNameWithoutDeviceMup.MaximumLength	= OpeningMupName->MaximumLength - (sizeof(NXRMFLT_MUP_PREFIX) - sizeof(WCHAR));

			bRet = NkEndsWithUnicodeString(OpeningMupName, &FileNameWithoutDevice, TRUE);

			if (!bRet)
			{
				bRet = NkEndsWithUnicodeString(FileName, &OpeningMupNameWithoutDeviceMup, TRUE);
			}
		}
		else
		{
			bRet = NkEndsWithUnicodeString(OpeningMupName, FileName, TRUE);

		}

	} while (FALSE);

	return bRet;
}

BOOLEAN is_app_in_real_name_access_list(PEPROCESS  Process)
{
	BOOLEAN bRet = FALSE;

	CHAR *p = NULL;

	do 
	{
		p = PsGetProcessImageFileName(Process);

		if (p)
		{
			if (_stricmp(p,"iexplore.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "chrome.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "firefox.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "System") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "outlook.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "saplogon.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "sapguiserver.e") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "MicrosoftEdgeC") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "PickerHost.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "javaw.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

		}

		if (is_process_a_service(Process))
		{
			bRet = TRUE;
			break;
		}

	} while (FALSE);

	return bRet;
}

BOOLEAN is_app_in_graphic_integration_list(PEPROCESS  Process)
{
	BOOLEAN bRet = FALSE;

	CHAR *p = NULL;

	do
	{
		p = PsGetProcessImageFileName(Process);

		if (p)
		{
			if (_stricmp(p, "winword.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "excel.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "powerpnt.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			if (_stricmp(p, "acrord32.exe") == 0)
			{
				bRet = TRUE;
				break;
			}

			//if (_stricmp(p, "veviewer.exe") == 0)
			//{
			//	bRet = TRUE;
			//	break;
			//}

			//if (_stricmp(p, "visview.exe") == 0)
			//{
			//	bRet = TRUE;
			//	break;
			//}
		}

	} while (FALSE);

	return bRet;
}

static BOOLEAN is_process_a_service(PEPROCESS  Process)
{
	BOOLEAN bRet = FALSE;

	TOKEN_USER	user;

	PACCESS_TOKEN	pPrimaryToken = NULL;

	NTSTATUS status = STATUS_SUCCESS;

	PISID	pUserSid = NULL;

	do
	{
		memset(&user, 0, sizeof(user));

		pPrimaryToken = PsReferencePrimaryToken(Process);

		if (!pPrimaryToken)
		{
			break;
		}

		status = SeQueryInformationToken(pPrimaryToken, TokenUser, (PVOID)&user);

		if (!NT_SUCCESS(status))
		{
			break;
		}

		pUserSid = *(PISID*)user.User.Sid;

		switch (pUserSid->SubAuthority[0])
		{
		case SECURITY_LOCAL_SYSTEM_RID:
		case SECURITY_LOCAL_SERVICE_RID:
		case SECURITY_NETWORK_SERVICE_RID:
			bRet = TRUE;
			break;
		default:
			bRet = FALSE;
			break;
		}

	} while (FALSE);

	if (pPrimaryToken)
	{
		PsDereferencePrimaryToken(pPrimaryToken);
		pPrimaryToken = NULL;
	}

	return bRet;
}

NTSTATUS get_file_id_and_attribute_ex(PFLT_INSTANCE	Instance, FILE_OBJECT *FileObject, LARGE_INTEGER *Id, ULONG *FileAttributes)
{
	NTSTATUS Status = STATUS_SUCCESS;

	IO_STATUS_BLOCK IoStatus = { 0 };

	FILE_BASIC_INFORMATION BasicInfo = { 0 };
	FILE_INTERNAL_INFORMATION IdInfo = { 0 };

	do
	{
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

	return Status;
}
