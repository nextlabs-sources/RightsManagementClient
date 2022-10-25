#include "nxrmfltdef.h"
#include "nxrmfltnxlcachemgr.h"
#include "nxrmfltsetinformation.h"
#include "nxrmfltutils.h"

extern DECLSPEC_CACHEALIGN PFLT_FILTER			gFilterHandle;
extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

static NTSTATUS build_nxlcache_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *ParentDirName, UNICODE_STRING *FinalComponent);
static NTSTATUS build_nxlcache_file_name_from_name_with_nxl_extension(NXL_CACHE_NODE *pNode, UNICODE_STRING *ParentDirName, UNICODE_STRING *FinalComponent);
static NTSTATUS build_nxlcache_file_name_from_name_without_nxl_extension(NXL_CACHE_NODE *pNode, UNICODE_STRING *ParentDirName, UNICODE_STRING *FinalComponent);
static NTSTATUS build_nxlcache_reparse_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *NewReparseName);

static NTSTATUS build_nxlcache_file_name_ex(NXL_CACHE_NODE *pNode, UNICODE_STRING *FullPathFileName);
static NTSTATUS build_nxlcache_file_name_from_name_with_nxl_extension_ex(NXL_CACHE_NODE *pNode, UNICODE_STRING *FullPathFileName);
static NTSTATUS build_nxlcache_file_name_from_name_without_nxl_extension_ex(NXL_CACHE_NODE *pNode, UNICODE_STRING *FullPathFileName);
static NTSTATUS build_nxlcache_source_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *SourceFileName);
static BOOLEAN is_adobe_like_process(void);

extern BOOLEAN IsNXLFile(PUNICODE_STRING Extension);
extern BOOLEAN is_explorer(void);
extern NTSTATUS get_file_id_and_attribute(PFLT_INSTANCE	Instance, UNICODE_STRING *FileName, LARGE_INTEGER *Id, ULONG *FileAttributes);
extern LPSTR PsGetProcessImageFileName(PEPROCESS  Process);

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreSetInformation(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS	CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;

	FILE_INFORMATION_CLASS FileInformationClass = 0;

	PFLT_FILE_NAME_INFORMATION	NameInfo = NULL;
	PFILE_RENAME_INFORMATION	RenameInfo = NULL;

	PNXRMFLT_STREAMHANDLE_CONTEXT Ccb = NULL;
	NXL_CACHE_NODE	*pCacheNode = NULL;
	
	BOOLEAN SourceFileIsNxlFile = FALSE;

	NXRMFLT_STREAM_CONTEXT	*Ctx = NULL;

	NXRMFLT_SETINFORMATION_CONTEXT	*SetFileInfoCtx = NULL;

	NXRMFLT_INSTANCE_CONTEXT	*InstCtx = NULL;

	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;
	
	FileInformationClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
	
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

		if (FileInformationClass != FileAllocationInformation &&
			FileInformationClass != FileBasicInformation &&
			FileInformationClass != FileEndOfFileInformation &&
			FileInformationClass != FileRenameInformation &&
			FileInformationClass != FileValidDataLengthInformation)
		{
			break;
		}
		
		if(FileInformationClass == FileRenameInformation)
		{
			PT_DBG_PRINT(PTDBG_TRACE_RENAME, ("nxrmflt!nxrmfltPreSetInformation: Received a rename request!\n"));

			Status = FltAllocateContext(Global.Filter,
										FLT_STREAMHANDLE_CONTEXT,
										sizeof(NXRMFLT_STREAMHANDLE_CONTEXT),
										NonPagedPool,
										(PFLT_CONTEXT*)&Ccb);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: Failed to allocate stream handle ctx!\n"));
				
				break;
			}

			Ccb->DestinationFileNameInfo	= NULL;
			Ccb->SourceFileNameInfo			= NULL;
			Ccb->SourceFileIsNxlFile		= FALSE;
			Ccb->EncryptDestinationFile		= FALSE;
			Ccb->DeleteOnClose				= FALSE;
			Ccb->Reserved					= 0;

			Status =  FltGetFileNameInformation(Data,
												FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
												&Ccb->SourceFileNameInfo);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: Failed to query file name information!\n"));

				break;
			}

			Status = FltParseFileNameInformation(Ccb->SourceFileNameInfo);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: Failed to parse name information!\n"));

				break;
			}

			FltAcquirePushLockShared(&Global.NxlFileCacheLock);

			pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ccb->SourceFileNameInfo->Name);

			if(pCacheNode)
			{
				Ccb->SourceFileIsNxlFile = SourceFileIsNxlFile = TRUE;
			}
			else
			{
				Ccb->SourceFileIsNxlFile = SourceFileIsNxlFile = FALSE;
			}

			FltReleasePushLock(&Global.NxlFileCacheLock);

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
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: FltSetStreamHandleContext return %x\n", Status));
			}

			RenameInfo = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
	
			//
			// We can use FltInstance instead of OriginalInstance store in Ctx because Rename has to happen on the same Volume
			// otherwise it's a copy and delete operation
			//
			Status = FltGetDestinationFileNameInformation(FltInstance,
														  FileObject,
														  RenameInfo->RootDirectory,
														  RenameInfo->FileName,
														  RenameInfo->FileNameLength,
														  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
														  &NameInfo);

			if(!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: FltGetDestinationFileNameInformation return %x\n", Status));

				break;
			}

			if (RenameInfo->ReplaceIfExists == FALSE && 
				IsNXLFile(&Ccb->SourceFileNameInfo->Extension) &&
				NameInfo->Name.Length >= sizeof(NXRMFLT_NXL_DOTEXT) &&
				memcmp((UCHAR*)NameInfo->Name.Buffer + NameInfo->Name.Length - (sizeof(NXRMFLT_NXL_DOTEXT) - sizeof(WCHAR)), NXRMFLT_NXL_DOTEXT, sizeof(NXRMFLT_NXL_DOTEXT) - sizeof(WCHAR)) == 0)
			{
				UNICODE_STRING DstFileNameWithoutNXLExtension = {0};

				DstFileNameWithoutNXLExtension.Buffer = NameInfo->Name.Buffer;
				DstFileNameWithoutNXLExtension.MaximumLength = NameInfo->Name.MaximumLength - sizeof(NXRMFLT_NXL_DOTEXT) + sizeof(WCHAR);
				DstFileNameWithoutNXLExtension.Length = NameInfo->Name.Length - sizeof(NXRMFLT_NXL_DOTEXT) + sizeof(WCHAR);

				//
				// return STATUS_OBJECT_NAME_COLLISION if destination file without nxl extension does exist
				//
				if (nxrmfltDoesFileExist(FltInstance, &DstFileNameWithoutNXLExtension, TRUE))
				{
					Status = STATUS_OBJECT_NAME_COLLISION;
					Data->IoStatus.Status		= Status;
					Data->IoStatus.Information	= 0;
					CallbackStatus = FLT_PREOP_COMPLETE;
					
					break;
				}
			}

			PT_DBG_PRINT(PTDBG_TRACE_RENAME, 
						 ("nxrmflt!nxrmfltPreSetInformation: Source name is %wZ and destination file is %wZ. %wZ %s an NXL file!\n", 
						 &Ccb->SourceFileNameInfo->Name, 
						 &Ccb->DestinationFileNameInfo->Name,
						 &Ccb->SourceFileNameInfo->Name,
						 SourceFileIsNxlFile?"is":"is NOT"));
		}
		else if (FileInformationClass == FileBasicInformation)
		{
			Status = FltGetFileNameInformation(Data,
											   FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
											   &NameInfo);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: Failed to query file name information!\n"));

				break;
			}

			Status = FltParseFileNameInformation(NameInfo);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPreSetInformation: Failed to parse name information!\n"));

				break;
			}

			// If this is an NXL file, and the corresponding non-NXL file
			// exists, and we are changing read-only attribute, we need to
			// propagate the change to the non-nXL file.  So we invoke our
			// post routine to queue a workitem to check that after this
			// operation succeeds.
			//
			// On the other hand, if this is not an NXL file, we don't need to
			// invoke our post routine.
			if (!IsNXLFile(&NameInfo->Extension))
			{
				FltReleaseFileNameInformation(NameInfo);
				NameInfo = NULL;
			}
		}

		FltGetStreamContext(FltInstance, FileObject, &Ctx);

		if (Ctx == NULL &&
			NameInfo == NULL)
		{
			break;
		}

		SetFileInfoCtx = ExAllocateFromPagedLookasideList(&Global.SetInformationCtxLookaside);

		if (!SetFileInfoCtx)
		{
			break;
		}

		SetFileInfoCtx->Ctx				= Ctx;
		SetFileInfoCtx->NameInfo		= NameInfo;

		*CompletionContext = SetFileInfoCtx;

		//
		// set to NULL to prevent it from being freed
		//
		SetFileInfoCtx = NULL;

		//
		// set to NULL to prevent it from being freed
		//
		NameInfo = NULL;

		CallbackStatus = FLT_PREOP_SYNCHRONIZE;

	} while (FALSE);

	if(Ccb)
	{
		FltReleaseContext(Ccb);
	}

	if (SetFileInfoCtx)
	{
		ExFreeToPagedLookasideList(&Global.SetInformationCtxLookaside, SetFileInfoCtx);
		SetFileInfoCtx = NULL;
	}

	if (NameInfo)
	{
		FltReleaseFileNameInformation(NameInfo);
		NameInfo = NULL;
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return CallbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS
	nxrmfltPostSetInformation(
	_Inout_ PFLT_CALLBACK_DATA		Data,
	_In_ PCFLT_RELATED_OBJECTS		FltObjects,
	_In_opt_ PVOID					CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS	Flags
	)
{
	FLT_POSTOP_CALLBACK_STATUS	CallbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

	NTSTATUS	Status = STATUS_SUCCESS;
	ULONG_PTR	Information = 0;

	FILE_INFORMATION_CLASS FileInformationClass = 0;

	PFLT_INSTANCE			FltInstance = NULL;
	PFILE_OBJECT			FileObject = NULL;

	NXL_CACHE_NODE	*pCacheNode = NULL;

	PFILE_BASIC_INFORMATION FileBasicInfo = NULL;

	BOOLEAN	MediaEjected = FALSE;

	PNXRMFLT_SETINFORMATION_CONTEXT SetFileInfoCtx = (PNXRMFLT_SETINFORMATION_CONTEXT)CompletionContext;

	PFLT_FILE_NAME_INFORMATION	NameInfo		= SetFileInfoCtx->NameInfo;
	PNXRMFLT_STREAM_CONTEXT		Ctx				= SetFileInfoCtx->Ctx;

	PFLT_FILE_NAME_INFORMATION	TunnelNameInfo = NULL;

	BOOLEAN	ReleaseNameInfo = TRUE;

	NXL_CACHE_NODE	*pNode = NULL;

	UNICODE_STRING	OnDiskNXLFileName = { 0 };

	PNXRMFLT_STREAMHANDLE_CONTEXT Ccb = NULL;

	PNXL_RENAME_NODE pRenameNode = NULL;

	LIST_ENTRY	*ite = NULL;

	BOOLEAN AttachStreamCtxToRenamedFile = FALSE;

	BOOLEAN	SkipUpdateNxlCache = FALSE;

	FltInstance = FltObjects->Instance;
	FileObject = FltObjects->FileObject;

	do
	{
		Status = Data->IoStatus.Status;
		Information = Data->IoStatus.Information;
		FileInformationClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;

		if (!NT_SUCCESS(Status))
		{
			if(FileInformationClass == FileRenameInformation)
			{
				PT_DBG_PRINT(PTDBG_TRACE_RENAME, ("nxrmflt!nxrmfltPostSetInformation: Renaming request failed! Status is %x\n",Status));
			}
			
			break;
		}

		if(NameInfo)
		{
			Status = FltGetTunneledName(Data, NameInfo, &TunnelNameInfo);

			if (!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to get tunneled file name information! Status is %x\n", Status));
				break;
			}

			if (TunnelNameInfo)
			{
				PT_DBG_PRINT(PTDBG_TRACE_RENAME, ("nxrmflt!nxrmfltPostSetInformation: Successfully get tunneled name information! New name is %wZ\n", &TunnelNameInfo->Name));

				FltReleaseFileNameInformation(NameInfo);

				NameInfo = TunnelNameInfo;
			}

			Status = FltParseFileNameInformation(NameInfo);

			if(!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to parse file name information! Status is %x\n", Status));
				break;
			}
		}

		Status = FltGetStreamHandleContext(FltInstance, FileObject, &Ccb);

		if(Ccb && NameInfo && (FileInformationClass == FileRenameInformation))
		{
			UNICODE_STRING SrcOnDiskNxlFileName = { 0 };
			UNICODE_STRING DstOnDiskNxlFileName = { 0 };
			UNICODE_STRING SrcOnDiskFileNameWithoutExtension = { 0 };

			Ccb->DestinationFileNameInfo = NameInfo;

			ReleaseNameInfo = FALSE;

			do 
			{
				pRenameNode = ExAllocateFromPagedLookasideList(&Global.RenameCacheLookaside);

				if(!pRenameNode)
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate rename node from lookaside!\n"));

					break;
				}

				memset(pRenameNode, 0, sizeof(NXL_RENAME_NODE));
				
				Status = nxrmfltBuildRenameNodeFromCcb(Ccb, pRenameNode);

				if(!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build rename node from Ccb! Status is %x\n",Status));

					break;
				}

				PT_DBG_PRINT(PTDBG_TRACE_RENAME, 
							 ("nxrmflt!nxrmfltPostSetInformation: Insert rename node into rename list. Source name is %wZ and destination name is %wZ\n", 
							 &pRenameNode->SourceFileName, 
							 &pRenameNode->DestinationFileName));

				FltAcquirePushLockExclusive(&Global.RenameListLock);

				InsertHeadList(&Global.RenameList, &pRenameNode->Link);

				FltReleasePushLock(&Global.RenameListLock);

				//
				// make sure pRenameNode won't get freed
				//
				pRenameNode = NULL;

				FltAcquirePushLockShared(&Global.RenameListLock);

				FOR_EACH_LIST(ite, &Global.RenameList)
				{
					NXL_RENAME_NODE *pTmpNode = CONTAINING_RECORD(ite, NXL_RENAME_NODE, Link);

					if(0 == RtlCompareUnicodeString(&Ccb->DestinationFileNameInfo->Name, &pTmpNode->SourceFileName, TRUE) &&
					   pTmpNode->SourceFileIsNxlFile)
					{
						SrcOnDiskNxlFileName.Buffer			= ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);
						
						if(SrcOnDiskNxlFileName.Buffer)
						{
							SrcOnDiskNxlFileName.MaximumLength	= NXRMFLT_FULLPATH_BUFFER_SIZE;
							SrcOnDiskNxlFileName.Length			= 0;

							RtlUnicodeStringCat(&SrcOnDiskNxlFileName, &pTmpNode->DestinationFileName);
							RtlUnicodeStringCatString(&SrcOnDiskNxlFileName, NXRMFLT_NXL_DOTEXT);

							SrcOnDiskFileNameWithoutExtension.Buffer = SrcOnDiskNxlFileName.Buffer;
							SrcOnDiskFileNameWithoutExtension.MaximumLength = SrcOnDiskNxlFileName.MaximumLength;
							SrcOnDiskFileNameWithoutExtension.Length = pTmpNode->DestinationFileName.Length;
						}
						else
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate source file name from lookaside!\n"));

							break;
						}

						DstOnDiskNxlFileName.Buffer			= ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

						if(DstOnDiskNxlFileName.Buffer)
						{
							DstOnDiskNxlFileName.MaximumLength	= NXRMFLT_FULLPATH_BUFFER_SIZE;
							DstOnDiskNxlFileName.Length			= 0;

							RtlUnicodeStringCat(&DstOnDiskNxlFileName, &Ccb->DestinationFileNameInfo->Name);
							RtlUnicodeStringCatString(&DstOnDiskNxlFileName, NXRMFLT_NXL_DOTEXT);
						}
						else
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate destination file name from lookaside!\n"));

							break;
						}

						AttachStreamCtxToRenamedFile = TRUE;
						break;
					}
				}

				FltReleasePushLock(&Global.RenameListLock);

				if(AttachStreamCtxToRenamedFile)
				{
					NXL_CACHE_NODE	*pNewCacheNode = NULL;
					NXRMFLT_STREAM_CONTEXT	*NewCtx = NULL;
					BOOLEAN FreeNewCacheNode = FALSE;
					
					PT_DBG_PRINT(PTDBG_TRACE_RENAME, 
								 ("nxrmflt!nxrmfltPostSetInformation: Need to attach stream Ctx to file %wZ\n", 
								 &Ccb->DestinationFileNameInfo->Name));

					do 
					{

						Status = FltAllocateContext(Global.Filter, 
													FLT_STREAM_CONTEXT, 
													sizeof(NXRMFLT_STREAM_CONTEXT), 
													NonPagedPool, 
													(PFLT_CONTEXT*)&NewCtx);

						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate stream Ctx!\n"));

							break;
						}

						//
						// Initialize Ctx
						//
						memset(NewCtx, 0, sizeof(NXRMFLT_STREAM_CONTEXT));

						//
						// make sure CtxCleanup won't free NULL point in case there is error when building this Ctx
						//
						NewCtx->ReleaseFileName = FALSE;

						InterlockedIncrement(&Global.TotalContext);

						Status = nxrmfltBuildNamesInStreamContext(NewCtx, &Ccb->DestinationFileNameInfo->Name);

						if(!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build names in stream Ctx! Status is %x\n", Status));

							break;
						}

						NewCtx->OriginalInstance = FltInstance;

						Status = FltGetRequestorSessionId(Data, &NewCtx->RequestorSessionId);

						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: FltGetRequestorSessionId failed! Status is %x\n", Status));

							NewCtx->RequestorSessionId = NXRMFLT_INVALID_SESSION_ID;
						}

						FltInitializePushLock(&NewCtx->CtxLock);

						NewCtx->ContentDirty = 1;

						Status = FltSetStreamContext(FltInstance,
													 FileObject,
													 FLT_SET_CONTEXT_KEEP_IF_EXISTS,
													 NewCtx,
													 NULL);

						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to set stream Ctx! This could be normal. Status is %x\n", Status));

							break;
						}

						FltAcquirePushLockShared(&Global.NxlFileCacheLock);

						pNewCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &NewCtx->FileName);

						if(pNewCacheNode)
						{
							//
							// Chance for code run to here is really low
							//
							SetFlag(pNewCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
						}

						FltReleasePushLock(&Global.NxlFileCacheLock);

						if(!pNewCacheNode)
						{
							
							PT_DBG_PRINT(PTDBG_TRACE_RENAME,
										 ("nxrmflt!nxrmfltPostSetInformation: As expected, couldn't find the newly renamed file %wZ in cache!\n", 
										 &NewCtx->FileName));

							//
							// we need to build CacheNode here
							//

							pNewCacheNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

							if(!pNewCacheNode)
							{
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate cache node from lookaside!\n"));

								Status = STATUS_INSUFFICIENT_RESOURCES;
								break;
							}

							memset(pNewCacheNode, 0, sizeof(NXL_CACHE_NODE));

							ExInitializeRundownProtection(&pNewCacheNode->NodeRundownRef);

							pNewCacheNode->FileAttributes			= 0;
							pNewCacheNode->FileID.QuadPart			= 0;
							pNewCacheNode->Flags					= NXRMFLT_FLAG_CTX_ATTACHED;
							pNewCacheNode->Instance					= FltInstance;
							pNewCacheNode->OnRemoveOrRemovableMedia = FALSE;

							RtlHashUnicodeString(&NewCtx->FullPathParentDir, TRUE, HASH_STRING_ALGORITHM_X65599, &pNewCacheNode->ParentDirectoryHash);

							//
							// build "ReparseFileName" file name field
							//

							Status = build_nxlcache_reparse_name(pNewCacheNode, &Ccb->DestinationFileNameInfo->Name);

							if(!NT_SUCCESS(Status))
							{
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
											 ("nxrmflt!nxrmfltPostSetInformation: Failed to build reparse name for %wZ! Status is %x\n", 
											 &Ccb->DestinationFileNameInfo->Name,
											 Status));
								break;
							}

							//
							// Ignore cases like renaming to different folder
							//

							Status = build_nxlcache_file_name(pNewCacheNode, &(NewCtx->FullPathParentDir), &Ccb->DestinationFileNameInfo->FinalComponent);

							if(!NT_SUCCESS(Status))
							{
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
											 ("nxrmflt!nxrmfltPostSetInformation: Failed to build cache node name for %wZ! Status is %x\n", 
											 &Ccb->DestinationFileNameInfo->Name,
											 Status));

								break;
							}

							//
							// copy source file if there is any
							//

							do 
							{
								NXL_CACHE_NODE *pSrcCacheNodeTmp = NULL;

								FltAcquirePushLockShared(&Global.NxlFileCacheLock);

								pSrcCacheNodeTmp = FindNXLNodeInCache(&Global.NxlFileCache, &SrcOnDiskFileNameWithoutExtension);

								if (pSrcCacheNodeTmp)
								{
									if (!ExAcquireRundownProtection(&pSrcCacheNodeTmp->NodeRundownRef))
									{
										pSrcCacheNodeTmp = NULL;
									}
								}

								FltReleasePushLock(&Global.NxlFileCacheLock);

								if (!pSrcCacheNodeTmp)
								{
									break;
								}

								if (!pSrcCacheNodeTmp->OnRemoveOrRemovableMedia)
								{
									ExReleaseRundownProtection(&pSrcCacheNodeTmp->NodeRundownRef);
									break;
								}

								pNewCacheNode->OnRemoveOrRemovableMedia = pSrcCacheNodeTmp->OnRemoveOrRemovableMedia;

								build_nxlcache_source_file_name(pNewCacheNode, &pSrcCacheNodeTmp->SourceFileName);

								ExReleaseRundownProtection(&pSrcCacheNodeTmp->NodeRundownRef);

							} while (FALSE);


							FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

							//
							// there is no chance this will fail because renaming hold VCB lock which means no two rename on the same volume can happen at the same time
							//
							if(!AddNXLNodeToCache(&Global.NxlFileCache, pNewCacheNode))
							{
								FreeNewCacheNode = TRUE;
							}
	
							FltReleasePushLock(&Global.NxlFileCacheLock);

							if(FreeNewCacheNode)
							{
								FreeNXLCacheNode(pNewCacheNode);
								pNewCacheNode = NULL;
							}

							SkipUpdateNxlCache = TRUE;

							Status = nxrmfltCopyOnDiskNxlFile(FltInstance,
															  &SrcOnDiskNxlFileName,
															  FltInstance,
															  &DstOnDiskNxlFileName);

							if(!NT_SUCCESS(Status))
							{
								PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
											 ("nxrmflt!nxrmfltPostSetInformation: Failed to copy on disk Nxl file from %wZ to %wZ. Status is %x\n",
											 &SrcOnDiskNxlFileName,
											 &DstOnDiskNxlFileName,
											 Status));
								break;
							}
						}
					
					} while (FALSE);

					if(NewCtx)
					{
						FltReleaseContext(NewCtx);
						NewCtx = NULL;
					}

					if(!NT_SUCCESS(Status) && pNewCacheNode)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
									 ("nxrmflt!nxrmfltPostSetInformation: Failed to build new Ctx or build new cache node for %wZ! Status is %x. pNewCacheNode is %p\n", 
									 &Ccb->DestinationFileNameInfo->Name,
									 Status,
									 pNewCacheNode));

						FreeNXLCacheNode(pNewCacheNode);
						pNewCacheNode = NULL;
					}
				}
				else
				{
					PT_DBG_PRINT(PTDBG_TRACE_RENAME,
								 ("nxrmflt!nxrmfltPostSetInformation: No need to track file %wZ\n",
								 &Ccb->DestinationFileNameInfo->Name));
				}

			} while (FALSE);

			if(SrcOnDiskNxlFileName.Buffer)
			{
				ExFreeToPagedLookasideList(&Global.FullPathLookaside, SrcOnDiskNxlFileName.Buffer);

				RtlInitUnicodeString(&SrcOnDiskNxlFileName, NULL);
			}

			if(DstOnDiskNxlFileName.Buffer)
			{
				ExFreeToPagedLookasideList(&Global.FullPathLookaside, DstOnDiskNxlFileName.Buffer);

				RtlInitUnicodeString(&DstOnDiskNxlFileName, NULL);
			}

		}

		if(SkipUpdateNxlCache)
		{
			PT_DBG_PRINT(PTDBG_TRACE_RENAME,
						 ("nxrmflt!nxrmfltPostSetInformation: No need to update cache node for file %wZ\n",
						 &Ccb->DestinationFileNameInfo->Name));

			break;
		}

		if (!Ctx)
		{
			if (Ccb == NULL)
			{
				if (FileInformationClass == FileBasicInformation)
				{
					// This is an NXL file.  If the corresponding non-NXL file
					// exists, and we are changing read-only attribute, we
					// need to propagate the change to the non-nXL file.  So
					// we queue a work item to check if the non-NXL file
					// exists and to propagate the attribute.
					UNICODE_STRING NonNxlFileName;

					NonNxlFileName.Buffer = NameInfo->Name.Buffer;
					NonNxlFileName.MaximumLength = NameInfo->Name.MaximumLength;
					NonNxlFileName.Length = NameInfo->Name.Length - (sizeof(NXRMFLT_NXL_DOTEXT) - sizeof(WCHAR));

					FileBasicInfo = (PFILE_BASIC_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
					BOOLEAN ReadOnlyAttr = BooleanFlagOn(FileBasicInfo->FileAttributes, FILE_ATTRIBUTE_READONLY);

					nxrmfltSetReadOnlyAttrByName(FltInstance, &NonNxlFileName, ReadOnlyAttr);
				}

				break;
			}

			//
			// in the case of explorer or other applications renaming nxl file
			// other applications may download content from network and rename saved temp file to nxl file
			//

			if (IsNXLFile(&(Ccb->SourceFileNameInfo->Extension)) &&
				IsNXLFile(&(Ccb->DestinationFileNameInfo->Extension)))
			{
				UNICODE_STRING SrcFileNameWithoutNXLExtension = {0};
				UNICODE_STRING DstFileNameWithoutNXLExtension = {0};

				SrcFileNameWithoutNXLExtension.Buffer			= Ccb->SourceFileNameInfo->Name.Buffer;
				SrcFileNameWithoutNXLExtension.Length			= Ccb->SourceFileNameInfo->Name.Length - 4 * sizeof(WCHAR);
				SrcFileNameWithoutNXLExtension.MaximumLength	= Ccb->SourceFileNameInfo->Name.MaximumLength;

				//
				// build new cache node, rename on disk nxl file and complete this I/O
				//

				do 
				{
					NXL_CACHE_NODE	*pExistingCacheNode = NULL;
					NXL_CACHE_NODE	*pNewRenamedNode = NULL;

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
						break;
					}

					//
					// following block of code builds pNode
					//
					do 
					{

						pNewRenamedNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

						if(!pNewRenamedNode)
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate cache node from lookaside.\n"));

							break;
						}

						memset(pNewRenamedNode, 0, sizeof(NXL_CACHE_NODE));

						ExInitializeRundownProtection(&pNewRenamedNode->NodeRundownRef);

						pNewRenamedNode->FileAttributes				= pExistingCacheNode->FileAttributes;
						pNewRenamedNode->FileID						= pExistingCacheNode->FileID;
						pNewRenamedNode->Flags						= pExistingCacheNode->Flags;
						pNewRenamedNode->Instance					= pExistingCacheNode->Instance;
						pNewRenamedNode->ParentDirectoryHash		= pExistingCacheNode->ParentDirectoryHash;
						pNewRenamedNode->OnRemoveOrRemovableMedia	= pExistingCacheNode->OnRemoveOrRemovableMedia;

						ClearFlag(pNewRenamedNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
						SetFlag(pNewRenamedNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);

						DstFileNameWithoutNXLExtension.Buffer			= Ccb->DestinationFileNameInfo->Name.Buffer;
						DstFileNameWithoutNXLExtension.Length			= Ccb->DestinationFileNameInfo->Name.Length - 4 * sizeof(WCHAR);
						DstFileNameWithoutNXLExtension.MaximumLength	= Ccb->DestinationFileNameInfo->Name.MaximumLength;

						//
						// build "ReparseFileName" file name field
						//

						Status = build_nxlcache_reparse_name(pNewRenamedNode, &DstFileNameWithoutNXLExtension);

						if(!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build reparse name in cache node. Status is %x\n", Status));

							FreeNXLCacheNode(pNewRenamedNode);
							
							pNewRenamedNode = NULL;

							break;
						}

						//
						// Ignore cases like renaming to different folder
						//

						Status = build_nxlcache_file_name_ex(pNewRenamedNode, &DstFileNameWithoutNXLExtension);

						if(!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build cache name in cache node. Status is %x\n", Status));

							FreeNXLCacheNode(pNewRenamedNode);

							pNewRenamedNode = NULL;

							break;
						}

					} while (FALSE);

					//
					// Let's remove and delete the old Node
					//

					FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

					do 
					{
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

						if (pNewRenamedNode)
						{
							//
							// Add newly created node into the cache
							//
							if(!AddNXLNodeToCache(&Global.NxlFileCache, pNewRenamedNode))
							{
								//
								// Other thread add this new nxl file into cache already
								//

							}
							else
							{
								//
								// don't free pNode
								//
								pNewRenamedNode = NULL;
							}
						}

					} while (FALSE);


					FltReleasePushLock(&Global.NxlFileCacheLock);

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

					if (pNewRenamedNode)
					{
						FreeNXLCacheNode(pNewRenamedNode);
						pNewRenamedNode = NULL;
					}

				} while (FALSE);

			}
			else if (!IsNXLFile(&(Ccb->SourceFileNameInfo->Extension)) &&
					 IsNXLFile(&(Ccb->DestinationFileNameInfo->Extension)) &&
					 RtlEqualUnicodeString(&Ccb->SourceFileNameInfo->ParentDir, &Ccb->DestinationFileNameInfo->ParentDir, TRUE))
			{
				//
				// in the case of explorer (no ctx) or other applications download content from network and rename saved temp file
				//
				NXL_CACHE_NODE	*pNewRenamedNode = NULL;
				UNICODE_STRING	DstFileNameWithoutNXLExtension = {0};

				ULONG DirHash = 0;

				LARGE_INTEGER FileId = {0};

				ULONG FileAttributes = 0;

				//
				// following block of code builds pNode
				//
				do 
				{
					pNewRenamedNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

					if(!pNewRenamedNode)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate cache node from lookaside.\n"));

						break;
					}

					memset(pNewRenamedNode, 0, sizeof(NXL_CACHE_NODE));

					RtlHashUnicodeString(&Ccb->DestinationFileNameInfo->ParentDir, TRUE, HASH_STRING_ALGORITHM_X65599, &DirHash);

					get_file_id_and_attribute(FltInstance, &Ccb->DestinationFileNameInfo->Name, &FileId, &FileAttributes);

					ExInitializeRundownProtection(&pNewRenamedNode->NodeRundownRef);

					pNewRenamedNode->FileAttributes				= FileAttributes;
					pNewRenamedNode->FileID						= FileId;
					pNewRenamedNode->Flags						= 0;
					pNewRenamedNode->Instance					= FltInstance;
					pNewRenamedNode->ParentDirectoryHash		= DirHash;
					pNewRenamedNode->OnRemoveOrRemovableMedia	= FALSE;

					DstFileNameWithoutNXLExtension.Buffer			= Ccb->DestinationFileNameInfo->Name.Buffer;
					DstFileNameWithoutNXLExtension.Length			= Ccb->DestinationFileNameInfo->Name.Length - 4 * sizeof(WCHAR);
					DstFileNameWithoutNXLExtension.MaximumLength	= Ccb->DestinationFileNameInfo->Name.MaximumLength;

					//
					// build "ReparseFileName" file name field
					//

					Status = build_nxlcache_reparse_name(pNewRenamedNode, &DstFileNameWithoutNXLExtension);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build reparse name in cache node. Status is %x\n", Status));

						FreeNXLCacheNode(pNewRenamedNode);

						pNewRenamedNode = NULL;

						break;
					}

					//
					// Ignore cases like renaming to different folder
					//

					Status = build_nxlcache_file_name_ex(pNewRenamedNode, &DstFileNameWithoutNXLExtension);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build cache name in cache node. Status is %x\n", Status));

						FreeNXLCacheNode(pNewRenamedNode);

						pNewRenamedNode = NULL;

						break;
					}

				} while (FALSE);

				FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

				if (pNewRenamedNode)
				{
					//
					// Add newly created node into the cache
					//
					if(!AddNXLNodeToCache(&Global.NxlFileCache, pNewRenamedNode))
					{
						//
						// Other thread add this new nxl file into cache already
						//

					}
					else
					{
						//
						// don't free pNode
						//
						pNewRenamedNode = NULL;
					}
				}

				FltReleasePushLock(&Global.NxlFileCacheLock);

				if (pNewRenamedNode)
				{
					FreeNXLCacheNode(pNewRenamedNode);
					pNewRenamedNode = NULL;
				}
			}
			else if (IsNXLFile(&Ccb->SourceFileNameInfo->Extension) &&
				     !IsNXLFile(&Ccb->DestinationFileNameInfo->Extension))
			{
				//
				// in the case of explorer rename a NXL file to other file
				// we need to update NXL cache node and rename on disk decrypted
				// file
				//
				NXL_CACHE_NODE	*pNewRenamedNode = NULL;
				UNICODE_STRING	DstDecryptedFile = {0};
				UNICODE_STRING	DstNXLFileName = {0};
				UNICODE_STRING	SrcNXLFileName = {0};
				ULONG DirHash = 0;

				LARGE_INTEGER FileId = {0};

				ULONG FileAttributes = 0;

				do 
				{
					SrcNXLFileName.Buffer			= Ccb->SourceFileNameInfo->Name.Buffer;
					SrcNXLFileName.Length			= Ccb->SourceFileNameInfo->Name.Length - 4 * sizeof(WCHAR);
					SrcNXLFileName.MaximumLength	= Ccb->SourceFileNameInfo->Name.MaximumLength;

					//
					// step 1: find the source in cache node
					//
					FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &SrcNXLFileName);

					if (pCacheNode)
					{
						if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
						{
							pCacheNode = NULL;
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);

					if (!pCacheNode)
					{
						break;
					}

					//
					// step 2: build DstNXLFileName
					//
					DstNXLFileName.Buffer = ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

					if(!DstNXLFileName.Buffer)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate file name buffer from lookaside.\n"));
						break;
					}

					memset(DstNXLFileName.Buffer, 0, NXRMFLT_FULLPATH_BUFFER_SIZE);

					DstNXLFileName.MaximumLength	= NXRMFLT_FULLPATH_BUFFER_SIZE;
					DstNXLFileName.Length			= 0;

					RtlUnicodeStringCat(&DstNXLFileName, &Ccb->DestinationFileNameInfo->Name);
					RtlUnicodeStringCatString(&DstNXLFileName, NXRMFLT_NXL_DOTEXT);
					
					//
					// step 3: build DstDecryptedFileName
					//
					DstDecryptedFile.Buffer			= Ccb->DestinationFileNameInfo->Name.Buffer;
					DstDecryptedFile.Length			= Ccb->DestinationFileNameInfo->Name.Length;
					DstDecryptedFile.MaximumLength	= Ccb->DestinationFileNameInfo->Name.MaximumLength;

					//
					// step 4: build pNewRenamedNode
					//

					pNewRenamedNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

					if(!pNewRenamedNode)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate cache node from lookaside.\n"));
						break;
					}

					memset(pNewRenamedNode, 0, sizeof(NXL_CACHE_NODE));

					ExInitializeRundownProtection(&pNewRenamedNode->NodeRundownRef);

					RtlHashUnicodeString(&Ccb->DestinationFileNameInfo->ParentDir, 
										 TRUE, HASH_STRING_ALGORITHM_X65599, 
										 &DirHash);

					pNewRenamedNode->FileAttributes				= pCacheNode->FileAttributes;
					pNewRenamedNode->FileID						= pCacheNode->FileID;
					pNewRenamedNode->Flags						= 0;						// clear flags because we are going to delete decrypted file
					pNewRenamedNode->Instance					= pCacheNode->Instance;
					pNewRenamedNode->ParentDirectoryHash		= DirHash;
					pNewRenamedNode->OnRemoveOrRemovableMedia	= pCacheNode->OnRemoveOrRemovableMedia;

					//
					// step 5: build "ReparseFileName" field
					//

					Status = build_nxlcache_reparse_name(pNewRenamedNode, &DstDecryptedFile);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build reparse name in cache node. Status is %x\n", Status));
						break;
					}

					//
					// step 6: build "FileName" field
					//
					Status = build_nxlcache_file_name_ex(pNewRenamedNode, &DstDecryptedFile);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build cache name in cache node. Status is %x\n", Status));
						break;
					}

					//
					// step 7: delete decrypted file
					//
					if (FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED) ||
						FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX))
					{
						Status = nxrmfltDeleteFileByName(FltInstance, 
														 &pCacheNode->ReparseFileName);

						if(!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Can't delete file %wZ! Status is %x\n", &pCacheNode->ReparseFileName, Status));
							break;
						}
					}

					//
					// step 8: rename NXL file
					//
					Status = nxrmfltRenameOnDiskNXLFile(FltInstance,
														&Ccb->DestinationFileNameInfo->Name,
														&DstNXLFileName);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
									("nxrmflt!nxrmfltPostSetInformation: Failed to rename %wZ to %wZ. Status is %x\n", 
									&Ccb->DestinationFileNameInfo->Name, 
									&DstNXLFileName,
									Status));

						break;
					}

					//
					// update file Id and attribute
					// not all file system keep file ID the same
					//
					get_file_id_and_attribute(FltInstance, &DstNXLFileName, &FileId, &FileAttributes);

					pNewRenamedNode->FileID			= FileId;
					pNewRenamedNode->FileAttributes = FileAttributes;


					//
					// new Node is fully ready. all on disk files have been renamed. Let's remove and delete the old Node and add new node
					//

					FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

					do 
					{
						//
						// Delete old cache node from cache. NOTE: pCacheNode still valid after delete
						// We only delete the node from cache. We don't free resource here
						//
						DeleteNXLNodeInCache(&Global.NxlFileCache, pCacheNode);

						//
						// Add newly created node into the cache
						//
						if(AddNXLNodeToCache(&Global.NxlFileCache, pNewRenamedNode))
						{
							pNewRenamedNode = NULL;
						}

					} while (FALSE);

					FltReleasePushLock(&Global.NxlFileCacheLock);

					//
					// purge rights cache
					//
					nxrmfltPurgeRightsCache(pCacheNode->Instance, pCacheNode->FileNameHash);

					//
					// release old cache node rundown protection
					//
					ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
					//
					// free old cache node
					//
					FreeNXLCacheNode(pCacheNode);
					pCacheNode = NULL;

				} while (FALSE);

				if (DstNXLFileName.Buffer)
				{
					ExFreeToPagedLookasideList(&Global.FullPathLookaside, DstNXLFileName.Buffer);
					RtlInitUnicodeString(&DstNXLFileName, NULL);
				}

				//
				// in the case of error, pCacheNode is not NULL
				//
				if (pCacheNode)
				{
					ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
					pCacheNode = NULL;
				}

				//
				// in the case of other thread add new node to cache, pNewRenamedNode is not NULL
				//
				if (pNewRenamedNode)
				{
					FreeNXLCacheNode(pNewRenamedNode);
					pNewRenamedNode = NULL;
				}
			}
			else if (!IsNXLFile(&(Ccb->SourceFileNameInfo->Extension)) &&
					 !IsNXLFile(&(Ccb->DestinationFileNameInfo->Extension)))
			{
				if (RtlEqualUnicodeString(&Ccb->SourceFileNameInfo->ParentDir, &Ccb->DestinationFileNameInfo->ParentDir, TRUE))
				{
					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ccb->DestinationFileNameInfo->Name);

					if (pCacheNode)
					{
						Ccb->EncryptDestinationFile = TRUE;
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);

					pCacheNode = NULL;
				}

				{
					ADOBE_RENAME_NODE *pAdobeRenameNode = NULL;

					FltAcquirePushLockExclusive(&Global.AdobeRenameExpireTableLock);

					FOR_EACH_LIST(ite, &Global.AdobeRenameExpireTable)
					{
						pAdobeRenameNode = CONTAINING_RECORD(ite, ADOBE_RENAME_NODE, Link);

						if (RtlEqualUnicodeString(&pAdobeRenameNode->SourceFileName, &Ccb->DestinationFileNameInfo->Name, TRUE))
						{
							RemoveEntryList(ite);
							break;
						}
						else
						{
							pAdobeRenameNode = NULL;
						}
					}

					FltReleasePushLock(&Global.AdobeRenameExpireTableLock);

					if (pAdobeRenameNode)
					{
						NTSTATUS LocalStatus = STATUS_SUCCESS;

						ULONG RequestorSessionId = NXRMFLT_INVALID_SESSION_ID;

						//
						// ignore the return status
						// RequestorSessionId is NXRMFLT_INVALID_SESSION_ID if FltGetRequestorSessionId fails
						//
						if (STATUS_UNSUCCESSFUL == FltGetRequestorSessionId(Data, &RequestorSessionId))
						{
							RequestorSessionId = NXRMFLT_INVALID_SESSION_ID;
						}

						LocalStatus = nxrmfltDuplicateNXLFileAndItsRecords(RequestorSessionId,
																		   &pAdobeRenameNode->DestinationFileName,
																		   &Ccb->DestinationFileNameInfo->Name,
																		   FltInstance,
																		   &Ccb->DestinationFileNameInfo->ParentDir);
						
						if (!NT_SUCCESS(LocalStatus))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
										 ("nxrmflt!nxrmfltPostSetInformation: Failed to duplicate %wZ to %wZ. Status is %x\n", 
										 &pAdobeRenameNode->DestinationFileName, 
										 &Ccb->DestinationFileNameInfo->Name,
										 LocalStatus));
						}

						nxrmfltFreeAdobeRenameNode(pAdobeRenameNode);
						pAdobeRenameNode = NULL;
					}
				}
			}

			break;
		}

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

		if (pCacheNode)
		{
			if (FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_STOGRAGE_EJECTED))
			{
				MediaEjected = TRUE;
			}

			if (FileInformationClass == FileBasicInformation)
			{
				FileBasicInfo = (PFILE_BASIC_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;

				if (FileBasicInfo)
				{
					if (FileBasicInfo->FileAttributes != pCacheNode->FileAttributes)
					{
						pCacheNode->FileAttributes = FileBasicInfo->FileAttributes;

						InterlockedExchange(&Ctx->ContentDirty, 1);

						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Set content dirty flag.\n"));
					}
				}
			}
			else if(FileInformationClass == FileRenameInformation)
			{
				NT_ASSERT(NameInfo);

				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Set content dirty flag when renaming. Ctx->FileName is %wZ\n", &Ctx->FileName));

				do 
				{

					//Status = FltGetTunneledName(Data, NameInfo, &TunnelNameInfo);

					//if(!TunnelNameInfo)
					//{
					//	break;
					//}

					//FltReleaseFileNameInformation(NameInfo);
					//NameInfo = TunnelNameInfo;

					//Status = FltParseFileNameInformation(NameInfo);

					//if(!NT_SUCCESS(Status))
					//{
					//	break;
					//}

					OnDiskNXLFileName.Buffer = ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

					if(!OnDiskNXLFileName.Buffer)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate file name buffer from lookaside.\n"));

						Status = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}

					memset(OnDiskNXLFileName.Buffer, 0, NXRMFLT_FULLPATH_BUFFER_SIZE);

					OnDiskNXLFileName.MaximumLength = NXRMFLT_FULLPATH_BUFFER_SIZE;
					OnDiskNXLFileName.Length		= 0;

					RtlUnicodeStringCat(&OnDiskNXLFileName, &NameInfo->Name);
					RtlUnicodeStringCatString(&OnDiskNXLFileName, NXRMFLT_NXL_DOTEXT);

					//
					// issuing rename request to rename the real NXL file. 
					//

					Status = nxrmfltRenameOnDiskNXLFile(Ctx->OriginalInstance,
													    &pCacheNode->OriginalFileName,
													    &OnDiskNXLFileName);


					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
									 ("nxrmflt!nxrmfltPostSetInformation: Failed to rename %wZ to %wZ. Status is %x\n", 
									 &pCacheNode->OriginalFileName, 
									 &OnDiskNXLFileName,
									 Status));

						break;
					}

					InterlockedExchange(&Ctx->ContentDirty, 1); //update content dirty flag after rename

					PT_DBG_PRINT(PTDBG_TRACE_RENAME,
								 ("nxrmflt!nxrmfltPostSetInformation: Renamed on disk file %wZ to %wZ.\n",
								 &pCacheNode->OriginalFileName,
								 &OnDiskNXLFileName));

					pNode = ExAllocateFromPagedLookasideList(&Global.NXLCacheLookaside);

					if(!pNode)
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to allocate cache node from lookaside.\n"));

						Status = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}

					memset(pNode, 0, sizeof(NXL_CACHE_NODE));

					ExInitializeRundownProtection(&pNode->NodeRundownRef);

					pNode->FileAttributes			= pCacheNode->FileAttributes;
					pNode->FileID					= pCacheNode->FileID;
					pNode->Flags					= pCacheNode->Flags;
					pNode->Instance					= pCacheNode->Instance;
					pNode->ParentDirectoryHash		= pCacheNode->ParentDirectoryHash;
					pNode->OnRemoveOrRemovableMedia = pCacheNode->OnRemoveOrRemovableMedia;

					//
					// build "ReparseFileName" file name field
					//

					Status = build_nxlcache_reparse_name(pNode, &NameInfo->Name);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build reparse name in cache node. Status is %x\n", Status));

						break;
					}
					

					Status = build_nxlcache_file_name_ex(pNode, &NameInfo->Name);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build cache name in cache node. Status is %x\n", Status));

						break;
					}

					//
					// copy source file name field if there is any
					//
					if (pCacheNode->OnRemoveOrRemovableMedia)
					{
						Status = build_nxlcache_source_file_name(pNode, &pCacheNode->SourceFileName);

						if (!NT_SUCCESS(Status))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build source file name in cache node. Status is %x\n", Status));
							break;
						}
					}
					else
					{
						RtlInitUnicodeString(&pNode->SourceFileName, NULL);
					}
					
					//
					// update Ctx
					//

					Status = nxrmfltBuildNamesInStreamContext(Ctx, &pNode->FileName);

					if(!NT_SUCCESS(Status))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build names in stream Ctx. Status is %x\n", Status));

						break;
					}

					//
					// new Node is fully ready. Let's remove and delete the old Node
					//

					FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

					do 
					{
						//
						// Delete old cache node from cache. NOTE: pCacheNode still valid after delete
						// We only delete the node from cache. We don't free resource here
						//
						DeleteNXLNodeInCache(&Global.NxlFileCache, pCacheNode);

						//
						// Add newly created node into the cache
						//
						if(!AddNXLNodeToCache(&Global.NxlFileCache, pNode))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: FATAL ERROR!!!!!!!!!!!!!"));
						}

					} while (FALSE);

					FltReleasePushLock(&Global.NxlFileCacheLock);

					//
					// purge rights cache
					//
					nxrmfltPurgeRightsCache(pCacheNode->Instance, pCacheNode->FileNameHash);

					//
					// release old cache node rundown protection
					//
					ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
					//
					// free old cache node
					//
					FreeNXLCacheNode(pCacheNode);
					pCacheNode = NULL;

				} while (FALSE);

				if(!NT_SUCCESS(Status))
				{
					//
					// in this case, OS successfully rename the real file but we failed to update our record
					// THIS IS A NORMAL CASE and we need to recall the rename operation on the real file
					//
					NTSTATUS RevertStatus = STATUS_SUCCESS;

					RevertStatus = nxrmfltRenameFile(FltInstance, &(NameInfo->Name), &(pCacheNode->FileName));

					if (!NT_SUCCESS(RevertStatus))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: FATAL ERROR! Failed to revert rename operation!\n"));
					}

					Data->IoStatus.Status = Status;
					
					if(pNode)
					{
						FreeNXLCacheNode(pNode);
						pNode = NULL;
					}

					break;
				}

				//
				// build Adobe rename node
				//
				if (is_adobe_like_process())
				{
					nxrmfltBuildAdobeRenameNode(&Ccb->SourceFileNameInfo->Name, &Ccb->DestinationFileNameInfo->Name);
				}

			}
			else
			{
				InterlockedExchange(&Ctx->ContentDirty, 1);

				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Set content dirty flag\n"));
			}
		}
		else
		{
			if (FileInformationClass == FileRenameInformation)
			{
				NT_ASSERT(NameInfo);

				Status = nxrmfltBuildNamesInStreamContext(Ctx, &NameInfo->Name);

				if(!NT_SUCCESS(Status))
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation: Failed to build names in stream Ctx. Status is %x\n", Status));
				}
			}
			else
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,("nxrmflt!nxrmfltPostSetInformation: FATAL ERROR!!! Can't find cached record of %wZ.\n", Ctx->FileName));
			}
		}

		if (MediaEjected)
		{
			Data->IoStatus.Status = STATUS_LOST_WRITEBEHIND_DATA;
			Data->IoStatus.Information = 0;
		}

	} while (FALSE);

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	if (Ccb)
	{
		FltReleaseContext(Ccb);
	}

	if (pCacheNode)
	{
		ExReleaseRundownProtection(&pCacheNode->NodeRundownRef);
	}

	if (NameInfo && ReleaseNameInfo)
	{
		FltReleaseFileNameInformation(NameInfo);
	}

	if (OnDiskNXLFileName.Buffer)
	{
		ExFreeToPagedLookasideList(&Global.FullPathLookaside, OnDiskNXLFileName.Buffer);
	}

	if (pRenameNode)
	{
		nxrmfltFreeRenameNode(pRenameNode);

		ExFreeToPagedLookasideList(&Global.RenameCacheLookaside, pRenameNode);

		pRenameNode = NULL;
	}

	if (SetFileInfoCtx)
	{
		ExFreeToPagedLookasideList(&Global.SetInformationCtxLookaside, SetFileInfoCtx);
		
		SetFileInfoCtx = NULL;
	}

	return CallbackStatus;
}

static NTSTATUS build_nxlcache_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *ParentDirName, UNICODE_STRING *FinalComponent)
{
	//
	// FinalComponent does NOT include ".nxl" extension when Global.HideNXLExtension is TRUE
	//
	if (Global.HideNXLExtension)
	{
		return build_nxlcache_file_name_from_name_without_nxl_extension(pNode, ParentDirName, FinalComponent);
	}
	else
	{
		return build_nxlcache_file_name_from_name_with_nxl_extension(pNode, ParentDirName, FinalComponent);
	}

}

static NTSTATUS build_nxlcache_file_name_from_name_with_nxl_extension(NXL_CACHE_NODE *pNode, UNICODE_STRING *ParentDirName, UNICODE_STRING *FinalComponent)
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	USHORT NewFileNameLength = 0;

	do 
	{
		NewFileNameLength = ParentDirName->Length + FinalComponent->Length;

		if(NewFileNameLength > sizeof(pNode->FileNameFastBuffer))
		{
			pNode->FileName.Buffer = pNode->OriginalFileName.Buffer = ExAllocatePoolWithTag(PagedPool, NewFileNameLength, NXRMFLT_NXLCACHE_TAG);

			if(!pNode->FileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pNode->ReleaseFileName = TRUE;

			pNode->FileName.MaximumLength = pNode->OriginalFileName.MaximumLength = NewFileNameLength;
			pNode->FileName.Length = pNode->OriginalFileName.Length = NewFileNameLength;
		}
		else
		{
			pNode->ReleaseFileName = FALSE;

			pNode->FileName.Buffer = pNode->OriginalFileName.Buffer = pNode->FileNameFastBuffer;
			pNode->FileName.MaximumLength = pNode->OriginalFileName.MaximumLength = sizeof(pNode->FileNameFastBuffer);
			pNode->FileName.Length	= pNode->OriginalFileName.Length = NewFileNameLength;

		}

		memcpy(pNode->FileName.Buffer,
			   ParentDirName->Buffer,
			   ParentDirName->Length);

		memcpy(pNode->FileName.Buffer + ParentDirName->Length / sizeof(WCHAR),
			   FinalComponent->Buffer,
			   FinalComponent->Length);

		Status = RtlHashUnicodeString(&pNode->FileName, TRUE, HASH_STRING_ALGORITHM_X65599, &pNode->FileNameHash);
		if (0 == pNode->FileNameHash) {
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation (line: %d): RtlHashUnicodeString failed. Status is %x\n", __LINE__, Status));
			DbgPrint("Error");
		}

	} while (FALSE);

	return Status;
}

static NTSTATUS build_nxlcache_file_name_from_name_without_nxl_extension(NXL_CACHE_NODE *pNode, UNICODE_STRING *ParentDirName, UNICODE_STRING *FinalComponent)
{
	NTSTATUS Status = STATUS_SUCCESS;

	USHORT NewFileNameLength = 0;

	do 
	{
		NewFileNameLength = ParentDirName->Length + FinalComponent->Length + sizeof(NXRMFLT_NXL_DOTEXT) - sizeof(WCHAR);

		if(NewFileNameLength > sizeof(pNode->FileNameFastBuffer))
		{
			pNode->FileName.Buffer = pNode->OriginalFileName.Buffer = ExAllocatePoolWithTag(PagedPool, NewFileNameLength, NXRMFLT_NXLCACHE_TAG);

			if(!pNode->FileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pNode->ReleaseFileName = TRUE;

			pNode->FileName.MaximumLength = pNode->OriginalFileName.MaximumLength = NewFileNameLength;
			pNode->FileName.Length = ParentDirName->Length + FinalComponent->Length;
			pNode->OriginalFileName.Length = NewFileNameLength;
		}
		else
		{
			pNode->ReleaseFileName = FALSE;

			pNode->FileName.Buffer = pNode->OriginalFileName.Buffer = pNode->FileNameFastBuffer;
			pNode->FileName.MaximumLength = pNode->OriginalFileName.MaximumLength = sizeof(pNode->FileNameFastBuffer);
			pNode->FileName.Length	= ParentDirName->Length + FinalComponent->Length;
			pNode->OriginalFileName.Length = NewFileNameLength;
		}

		memcpy(pNode->FileName.Buffer,
			   ParentDirName->Buffer,
			   ParentDirName->Length);

		memcpy(pNode->FileName.Buffer + ParentDirName->Length / sizeof(WCHAR),
			   FinalComponent->Buffer,
			   FinalComponent->Length);

		memcpy(pNode->FileName.Buffer + ((ParentDirName->Length + FinalComponent->Length) / sizeof(WCHAR)), 
			   NXRMFLT_NXL_DOTEXT, 
			   (sizeof(NXRMFLT_NXL_DOTEXT)-sizeof(WCHAR)));

		Status = RtlHashUnicodeString(&pNode->FileName, TRUE, HASH_STRING_ALGORITHM_X65599, &pNode->FileNameHash);
		if (0 == pNode->FileNameHash) {
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation (line: %d): RtlHashUnicodeString failed. Status is %x\n", __LINE__, Status));
		}

	} while (FALSE);

	return Status;
}

static NTSTATUS build_nxlcache_reparse_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *NewReparseName)
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	do 
	{
		if(NewReparseName->Length > sizeof(pNode->ReparseFileNameFastBuffer))
		{
			pNode->ReparseFileName.Buffer = ExAllocatePoolWithTag(PagedPool, NewReparseName->Length, NXRMFLT_NXLCACHE_TAG);

			if(!pNode->ReparseFileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pNode->ReparseFileName.MaximumLength	= NewReparseName->Length;
			pNode->ReparseFileName.Length			= 0;

			pNode->ReleaseReparseName = TRUE;

		}
		else
		{
			pNode->ReparseFileName.Buffer			= pNode->ReparseFileNameFastBuffer;
			pNode->ReparseFileName.MaximumLength	= sizeof(pNode->ReparseFileNameFastBuffer);
			pNode->ReparseFileName.Length			= 0;

			pNode->ReleaseReparseName = FALSE;
		}

		RtlUnicodeStringCat(&pNode->ReparseFileName, NewReparseName);

	} while (FALSE);

	return Status;
}

//////////////////////////////////////////////////////////////////////////
//
// junk code
//
//////////////////////////////////////////////////////////////////////////

static NTSTATUS build_nxlcache_file_name_ex(NXL_CACHE_NODE *pNode, UNICODE_STRING *FullPathFileName)
{
	//
	// FinalComponent does NOT include ".nxl" extension when Global.HideNXLExtension is TRUE
	//
	if (Global.HideNXLExtension)
	{
		return build_nxlcache_file_name_from_name_without_nxl_extension_ex(pNode, FullPathFileName);
	}
	else
	{
		return build_nxlcache_file_name_from_name_with_nxl_extension_ex(pNode, FullPathFileName);
	}

}

static NTSTATUS build_nxlcache_file_name_from_name_with_nxl_extension_ex(NXL_CACHE_NODE *pNode, UNICODE_STRING *FullPathFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	USHORT NewFileNameLength = 0;

	do 
	{
		NewFileNameLength = FullPathFileName->Length;

		if(NewFileNameLength > sizeof(pNode->FileNameFastBuffer))
		{
			pNode->FileName.Buffer = pNode->OriginalFileName.Buffer = ExAllocatePoolWithTag(PagedPool, NewFileNameLength, NXRMFLT_NXLCACHE_TAG);

			if(!pNode->FileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pNode->ReleaseFileName = TRUE;

			pNode->FileName.MaximumLength = pNode->OriginalFileName.MaximumLength = NewFileNameLength;
			pNode->FileName.Length = pNode->OriginalFileName.Length = NewFileNameLength;
		}
		else
		{
			pNode->ReleaseFileName = FALSE;

			pNode->FileName.Buffer			= pNode->OriginalFileName.Buffer = pNode->FileNameFastBuffer;
			pNode->FileName.MaximumLength	= pNode->OriginalFileName.MaximumLength = sizeof(pNode->FileNameFastBuffer);
			pNode->FileName.Length			= pNode->OriginalFileName.Length = NewFileNameLength;

		}

		memcpy(pNode->FileName.Buffer,
			   FullPathFileName->Buffer,
			   FullPathFileName->Length);

		Status = RtlHashUnicodeString(&pNode->FileName, TRUE, HASH_STRING_ALGORITHM_X65599, &pNode->FileNameHash);
		if (0 == pNode->FileNameHash) {
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation (line: %d): RtlHashUnicodeString failed. Status is %x\n", __LINE__, Status));
		}

	} while (FALSE);

	return Status;
}

static NTSTATUS build_nxlcache_file_name_from_name_without_nxl_extension_ex(NXL_CACHE_NODE *pNode, UNICODE_STRING *FullPathFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	USHORT NewFileNameLength = 0;

	do 
	{
		NewFileNameLength = FullPathFileName->Length + sizeof(NXRMFLT_NXL_DOTEXT) - sizeof(WCHAR);

		if(NewFileNameLength > sizeof(pNode->FileNameFastBuffer))
		{
			pNode->FileName.Buffer = pNode->OriginalFileName.Buffer = ExAllocatePoolWithTag(PagedPool, NewFileNameLength, NXRMFLT_NXLCACHE_TAG);

			if(!pNode->FileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pNode->ReleaseFileName = TRUE;

			pNode->FileName.MaximumLength	= pNode->OriginalFileName.MaximumLength = NewFileNameLength;
			pNode->FileName.Length			= FullPathFileName->Length;
			pNode->OriginalFileName.Length	= NewFileNameLength;
		}
		else
		{
			pNode->ReleaseFileName = FALSE;

			pNode->FileName.Buffer			= pNode->OriginalFileName.Buffer = pNode->FileNameFastBuffer;
			pNode->FileName.MaximumLength	= pNode->OriginalFileName.MaximumLength = sizeof(pNode->FileNameFastBuffer);
			pNode->FileName.Length			= FullPathFileName->Length;
			pNode->OriginalFileName.Length	= NewFileNameLength;
		}

		memcpy(pNode->FileName.Buffer,
			   FullPathFileName->Buffer,
			   FullPathFileName->Length);

		memcpy(pNode->FileName.Buffer + ((FullPathFileName->Length) / sizeof(WCHAR)), 
			   NXRMFLT_NXL_DOTEXT, 
			   (sizeof(NXRMFLT_NXL_DOTEXT)-sizeof(WCHAR)));

		Status = RtlHashUnicodeString(&pNode->FileName, TRUE, HASH_STRING_ALGORITHM_X65599, &pNode->FileNameHash);
		if (0 == pNode->FileNameHash) {
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltPostSetInformation (line: %d): RtlHashUnicodeString failed. Status is %x\n", __LINE__, Status));
		}

	} while (FALSE);

	return Status;
}

static BOOLEAN is_adobe_like_process(void)
{
	BOOLEAN bRet = FALSE;

	CHAR *p = NULL;

	do 
	{
		p = PsGetProcessImageFileName(PsGetCurrentProcess());

		if(p)
		{
			if (_stricmp(p, "acrord32.exe") == 0)
			{
				bRet = TRUE;
			}

			if (_stricmp(p, "ugraf.exe") == 0)
			{
				bRet = TRUE;
			}
		}

	} while (FALSE);

	return bRet;

}

static NTSTATUS build_nxlcache_source_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *SourceFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	USHORT SourceFileNameLength = 0;

	do 
	{
		SourceFileNameLength = SourceFileName->Length;

		if (SourceFileNameLength > (USHORT)sizeof(pNode->SourceFileNameFastBuffer))
		{
			pNode->SourceFileName.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool, SourceFileNameLength, NXRMFLT_NXLCACHE_TAG);

			if (!pNode->SourceFileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			memset(pNode->SourceFileName.Buffer, 0, SourceFileNameLength);

			memcpy(pNode->SourceFileName.Buffer, SourceFileName->Buffer, SourceFileNameLength);

			pNode->SourceFileName.Length = SourceFileNameLength;
			pNode->SourceFileName.MaximumLength = SourceFileNameLength;
			pNode->ReleaseSourceFileName = TRUE;
		}
		else
		{
			pNode->SourceFileName.Buffer = pNode->SourceFileNameFastBuffer;

			memset(pNode->SourceFileName.Buffer, 0, SourceFileNameLength);

			memcpy(pNode->SourceFileName.Buffer, SourceFileName->Buffer, SourceFileNameLength);

			pNode->SourceFileName.Length = SourceFileNameLength;
			pNode->SourceFileName.MaximumLength = (USHORT)sizeof(pNode->SourceFileNameFastBuffer);
			pNode->ReleaseSourceFileName = FALSE;
		}

	} while (FALSE);

	return Status;
}
