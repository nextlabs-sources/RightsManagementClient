#include "nxrmfltdef.h"
#include "nxrmfltutils.h"
#include "nxrmflt.h"
#include "nxrmfltnxlcachemgr.h"

extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

extern ULONG PsGetCurrentProcessSessionId(void);
extern BOOLEAN is_app_in_real_name_access_list(PEPROCESS  Process);
extern BOOLEAN is_app_in_graphic_integration_list(PEPROCESS  Process);
extern NTSTATUS get_file_id_and_attribute(PFLT_INSTANCE	Instance, UNICODE_STRING *FileName, LARGE_INTEGER *Id, ULONG *FileAttributes);
extern NTSTATUS build_nxlcache_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *FileName);
extern NTSTATUS build_nxlcache_reparse_file_name(NXL_CACHE_NODE *pNode, UNICODE_STRING *FileName);

static NXL_RIGHTS_CACHE_NODE *
	FindRightsNodeInCache(
	rb_root			*Cache,
	ULONG			FileNameHash
	);

static BOOLEAN
	AddRightsNodeToCache(
	rb_root					*CacheMap,
	NXL_RIGHTS_CACHE_NODE	*pRightsNode
	);

static void make_device_string_user_friendly(char *device_string, ULONG length);
static BOOLEAN myisalnum(int c);
static NTSTATUS send_purge_cache_notification(PFLT_INSTANCE Instance, PUNICODE_STRING FileName);

NTSTATUS nxrmfltDecryptFile(
	__in PFLT_CALLBACK_DATA	Data,
	__in PFLT_INSTANCE		SrcInstance,
	__in PFLT_INSTANCE		DstInstance,
	__in PUNICODE_STRING		SrcFileName,
	__in PUNICODE_STRING		DstFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_FILTER		Filter = NULL;
	BOOLEAN			IgnoreCase = TRUE;
	HANDLE			SourceFileHandle = NULL;
	HANDLE			DestFileHandle = NULL;
	FILE_OBJECT		*SourceFileObject = NULL;
	FILE_OBJECT		*DestFileObject = NULL;
	OBJECT_ATTRIBUTES	SourceObjectAttribute;
	OBJECT_ATTRIBUTES	DestObjectAttribute;

	IO_STATUS_BLOCK		IoStatus = { 0 };

	FILE_STANDARD_INFORMATION FileStandardInfo = { 0 };
	FILE_POSITION_INFORMATION FilePositionInfo = { 0 };
	FILE_BASIC_INFORMATION	FileBasicInfo = { 0 };
	FILE_BASIC_INFORMATION	DestFileBasicInfo = { 0 };

	UCHAR	*ReadBuffer = NULL;
	ULONG	ReadBufferLength = 0;
	ULONG	BytesRead = 0;
	ULONG	ValidDataLength = 0;
	ULONG	BytesWrite = 0;

	LARGE_INTEGER	ReadPosition = { 0 };
	LARGE_INTEGER	WritePosition = { 0 };

	ULONG LengthNeeded = 0;
	ULONG SecurityDescriptorLength = 0;
	SECURITY_DESCRIPTOR *pSecurityDescriptor = NULL;

	HANDLE			AesObject = NULL;
	BOOLEAN			IsGoodNXLFile = FALSE;
	NXL_HEADER		*NXLHdr = NULL;
	NXL_KEKEY_BLOB	PrimaryKey = {0};
	UCHAR			ContentKey[NXRMFLT_CONTENT_KEY_LENGTH];
	ULONG			ContentOffset = 0;
	LARGE_INTEGER	ContentPosition = {0};
	LARGE_INTEGER	FileSize = {0};

	LIST_ENTRY	*ite = NULL;
	NXL_KEYCHAIN_NODE *pNode = NULL;

	do
	{
		Filter = Global.Filter;

		IgnoreCase = !(BooleanFlagOn(Data->Iopb->OperationFlags, SL_CASE_SENSITIVE));

		//if (!Global.ReparseInstance)
		//{
		//	Status = STATUS_INVALID_DEVICE_OBJECT_PARAMETER;

		//	break;
		//}

		InitializeObjectAttributes(&SourceObjectAttribute,
								   SrcFileName,
								   OBJ_KERNEL_HANDLE | (IgnoreCase ? OBJ_CASE_INSENSITIVE : 0),
								   NULL,
								   NULL);

		InitializeObjectAttributes(&DestObjectAttribute,
								   DstFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		//FltAcquirePushLockShared(&Global.PrimaryKeyLock);

		//memcpy(&PrimaryKey, 
		//	   &Global.PrimaryKey, 
		//	   min(sizeof(PrimaryKey),sizeof(Global.PrimaryKey)));

		//FltReleasePushLock(&Global.PrimaryKeyLock);

		Status = FltCreateFileEx2(Filter,
								  SrcInstance,
								  &SourceFileHandle,
								  &SourceFileObject,
								  FILE_GENERIC_READ,
								  &SourceObjectAttribute,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OPEN,
								  FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  0,
								  NULL);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FltCreateFileEx2 return %x for %wZ\n",
						 Status,
						 SrcFileName));
			break;
		}

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FileStandardInfo,
										 sizeof(FileStandardInfo),
										 FileStandardInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FltQueryInformationFile -> FileStandardInformation return %x for %wZ\n",
						 Status,
						 SrcFileName));

			break;
		}

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FilePositionInfo,
										 sizeof(FilePositionInfo),
										 FilePositionInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FltQueryInformationFile -> FilePositionInformation return %x for %wZ\n",
						 Status,
						 SrcFileName));

			break;
		}

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FileBasicInfo,
										 sizeof(FileBasicInfo),
										 FileBasicInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FltQueryInformationFile -> FileBasicInformation return %x for %wZ\n",
						 Status,
						 SrcFileName));

			break;
		}

		if(FileStandardInfo.EndOfFile.QuadPart < sizeof(NXL_HEADER))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: File %wZ is not a valid Nxl file. File size %I64u is too small.\n",
						 SrcFileName,
						 FileStandardInfo.EndOfFile.QuadPart));

			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		ReadBufferLength = (max(NXRMFLT_READFILE_BUFFER_SIZE, sizeof(NXL_HEADER)) + (PAGE_SIZE - 1)) & (~(PAGE_SIZE - 1));

		ReadBuffer = ExAllocatePoolWithTag(PagedPool,
										   ReadBufferLength,
										   NXRMFLT_READFILE_TAG);

		if (!ReadBuffer)
		{
			break;
		}

		RtlSecureZeroMemory(ReadBuffer, ReadBufferLength);

		Status = NXLCheck(SrcInstance, SourceFileObject, &IsGoodNXLFile);

		if(!NT_SUCCESS(Status) || (!IsGoodNXLFile))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: File %wZ is not a valid Nxl file. Status is %x\n",
						 SrcFileName,
						 Status));

			break;
		}

		NXLHdr = (NXL_HEADER *)ReadBuffer;

		Status = NXLReadHeader(SrcInstance, SourceFileObject, NXLHdr);

		if(!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: Failed to read Nxl file header!File is %wZ and Status is %x\n",
						 SrcFileName,
						 Status));

			break;
		}

		FltAcquirePushLockShared(&Global.KeyChainLock);

		FOR_EACH_LIST(ite, &Global.KeyChain)
		{
			pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

			if (memcmp(pNode->KeyBlob.KeyId.Id, NXLHdr->Crypto.PrimaryKey.KeKeyId.Id, min(pNode->KeyBlob.KeyId.IdSize, NXLHdr->Crypto.PrimaryKey.KeKeyId.IdSize)) == 0 &&
				pNode->KeyBlob.KeyId.Algorithm	== NXLHdr->Crypto.PrimaryKey.KeKeyId.Algorithm)
			{
				memcpy(&PrimaryKey, 
					   &pNode->KeyBlob, 
					   min(sizeof(PrimaryKey),sizeof(pNode->KeyBlob)));

				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		FltReleasePushLock(&Global.KeyChainLock);

		if (!pNode)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						 ("nxrmflt!nxrmfltDecryptFile: Can't find key in current key chain to decrypt file %wZ.",SrcFileName));

			nxrmfltSendKeyChainErrorMsg(SrcInstance, SrcFileName, &NXLHdr->Crypto.PrimaryKey.KeKeyId);

			Status = STATUS_DECRYPTION_FAILED;
			break;
		}

		Status = NXLGetContentKey((PCNXL_HEADER)NXLHdr, PrimaryKey.Key, ContentKey);

		if(!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FATAL ERROR A. Status is %x\n",
						 Status));

			break;
		}

		ContentOffset			= NXLHdr->Basic.PointerOfContent;
		FileSize.QuadPart		= NXLHdr->Crypto.ContentLength;

		//
		// The buffer NXLHdr point to is going to be reused for other purpose. Assign NXLHdr to NULL
		//
		NXLHdr = NULL;

		Status = NkCreateAesObject(ContentKey, 32, NXL_CBC_SIZE, &AesObject);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FATAL ERROR B. Status is %x\n",
						 Status));

			break;
		}

		Status = FltCreateFileEx2(Filter,
								  DstInstance,
								  &DestFileHandle,
								  &DestFileObject,
								  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
								  &DestObjectAttribute,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OVERWRITE_IF,
								  FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  IO_IGNORE_SHARE_ACCESS_CHECK,
								  NULL);

		if (!NT_SUCCESS(Status))
		{
			if(Status != STATUS_ACCESS_DENIED)
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
							 ("nxrmflt!nxrmfltDecryptFile: FATAL ERROR C. Status is %x\n",
							 Status));

				break;
			}

			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
						 ("nxrmflt!nxrmfltDecryptFile: FltCreateFileEx2 return access denied when opening file %wZ.\n",
						 DstFileName));

			Status = FltCreateFileEx2(Filter,
									  DstInstance,
									  &DestFileHandle,
									  &DestFileObject,
									  GENERIC_READ | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
									  &DestObjectAttribute,
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
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
							 ("nxrmflt!nxrmfltDecryptFile: FATAL ERROR D. Status is %x\n",
							 Status));

				break;
			}

			Status = FltQueryInformationFile(DstInstance,
											 DestFileObject,
											 &DestFileBasicInfo,
											 sizeof(DestFileBasicInfo),
											 FileBasicInformation,
											 NULL);

			if(!NT_SUCCESS(Status))
			{
				//
				// DestFileObject will be close at the end
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
							 ("nxrmflt!nxrmfltDecryptFile: FltQueryInformationFile -> FileBasicInformation return %x for %wZ\n",
							 Status,
							 DstFileName));

				break;
			}

			if(!FlagOn(DestFileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY))
			{
				//
				// STATUS_ACCESS_DENIED happened NOT because of read only
				//
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
							 ("nxrmflt!nxrmfltDecryptFile: FATAL ERROR E. Attributes are %x\n",
							 DestFileBasicInfo.FileAttributes));

				break;
			}

			ClearFlag(DestFileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY);

			Status = FltSetInformationFile(DstInstance,
										   DestFileObject,
										   &DestFileBasicInfo,
										   sizeof(DestFileBasicInfo),
										   FileBasicInformation);

			if(!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
							 ("nxrmflt!nxrmfltDecryptFile: FltSetInformationFile -> FileBasicInformation return %x for %wZ\n",
							 Status,
							 DstFileName));

				break;
			}

			//
			// Close the destination file
			//
			if (DestFileHandle)
			{
				FltClose(DestFileHandle);
				DestFileHandle = NULL;
			}

			if (DestFileObject)
			{
				ObDereferenceObject(DestFileObject);
			}

			//
			// try again after clear the read only flag
			//
			Status = FltCreateFileEx2(Filter,
									  DstInstance,
									  &DestFileHandle,
									  &DestFileObject,
									  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
									  &DestObjectAttribute,
									  &IoStatus,
									  NULL,
									  FILE_ATTRIBUTE_NORMAL,
									  FILE_SHARE_VALID_FLAGS,
									  FILE_OVERWRITE_IF,
									  FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
									  NULL,
									  0,
									  IO_IGNORE_SHARE_ACCESS_CHECK,
									  NULL);

			if(!NT_SUCCESS(Status))
			{
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, 
							 ("nxrmflt!nxrmfltDecryptFile: (2nd chance) FltCreateFileEx2 return %x when opening file %wZ.\n",
							 Status,
							 DstFileName));

				break;
			}
		}

		ReadPosition.QuadPart		= ContentOffset;
		WritePosition.QuadPart		= 0;
		ContentPosition.QuadPart	= 0;

		while (TRUE)
		{
			Status = FltReadFile(SrcInstance,
								 SourceFileObject,
								 &ReadPosition,
								 ReadBufferLength,
								 ReadBuffer,
								 0,
								 &BytesRead,
								 NULL,
								 NULL);

			if (Status == STATUS_END_OF_FILE)
			{
				Status = STATUS_SUCCESS;
				break;
			}

			if (!NT_SUCCESS(Status) || !BytesRead)
			{
				break;
			}

			//
			// ReadBuffer and a valid NXL file are always CBC alien. So, no adjusting here
			//
			Status = NkAesDecrypt(AesObject,
								  ContentPosition.QuadPart,
								  ReadBuffer,
								  BytesRead);

			if(!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// handle the last trunk of data which maybe less that CBC size
			//
			if(ContentPosition.QuadPart + BytesRead >= FileSize.QuadPart)
			{
				ValidDataLength = (ULONG)(FileSize.QuadPart - ContentPosition.QuadPart);
			}
			else
			{
				ValidDataLength = BytesRead;
			}

			Status = FltWriteFile(DstInstance,
								  DestFileObject,
								  &WritePosition,
								  ValidDataLength,
								  ReadBuffer,
								  0,
								  &BytesWrite,
								  NULL,
								  NULL);

			if (!NT_SUCCESS(Status) || ValidDataLength != BytesWrite)
			{
				break;
			}

			WritePosition.QuadPart += ValidDataLength;
			ReadPosition.QuadPart += BytesRead;
			ContentPosition.QuadPart += ValidDataLength;
		}

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltDecryptFile: Something is wrong when reading/writing data. Status is %x", Status));

			break;
		}

		//
		// TO DO: copy security object from source to destination
		//
		do
		{

			//
			// OWNER_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											OWNER_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);
		
			if (!pSecurityDescriptor)
			{
				break;
			}
	
			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											OWNER_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  OWNER_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			memset(pSecurityDescriptor, 0, SecurityDescriptorLength);

			//
			// GROUP_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											GROUP_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											GROUP_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  GROUP_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// DACL_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											DACL_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											DACL_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  DACL_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// DACL_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											SACL_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											SACL_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  SACL_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

		} while (FALSE);

		if (pSecurityDescriptor)
		{
			ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);
			pSecurityDescriptor = NULL;
		}

		//
		// restore source file attributes
		//
		//Status = FltSetInformationFile(SrcInstance,
		//							   SourceFileObject,
		//							   &FileStandardInfo,
		//							   sizeof(FileStandardInfo),
		//							   FileStandardInformation);

		Status = FltSetInformationFile(SrcInstance,
									   SourceFileObject,
									   &FilePositionInfo,
									   sizeof(FilePositionInfo),
									   FilePositionInformation);

		//
		// set destination file attributes
		//
		Status = FltSetInformationFile(DstInstance,
									   DestFileObject,
									   &FileBasicInfo,
									   sizeof(FileBasicInfo),
									   FileBasicInformation);

	} while (FALSE);

	if (SourceFileHandle)
	{
		FltClose(SourceFileHandle);
		SourceFileHandle = NULL;
	}

	if (DestFileHandle)
	{
		FltClose(DestFileHandle);
		DestFileHandle = NULL;
	}

	if (SourceFileObject)
	{
		ObDereferenceObject(SourceFileObject);
	}

	if (DestFileObject)
	{
		ObDereferenceObject(DestFileObject);
	}

	if (ReadBuffer)
	{
		ExFreePoolWithTag(ReadBuffer, NXRMFLT_READFILE_TAG);
		ReadBuffer = NULL;
	}

	if(AesObject)
	{
		NkDestroyAesObject(AesObject);
		AesObject = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltDeleteFileByNameSync(
	__in	PFLT_INSTANCE		Instance,
	__in	PUNICODE_STRING		FileName)
{
	NTSTATUS				Status = STATUS_SUCCESS;

	HANDLE					DestFileHandle = NULL;
	OBJECT_ATTRIBUTES		DestObjectAttribute = { 0 };
	IO_STATUS_BLOCK			IoStatus = {0};

	FILE_OBJECT				*DestFileObject = NULL;
	FILE_BASIC_INFORMATION	DestFileBasicInfo = { 0 };

	InitializeObjectAttributes(&DestObjectAttribute,
							   FileName,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
							   NULL, 
							   NULL);

	do 
	{

		Status = FltCreateFile(Global.Filter,
							   Instance,
							   &DestFileHandle,
							   DELETE,
							   &DestObjectAttribute,
							   &IoStatus,
							   NULL,
							   FILE_ATTRIBUTE_NORMAL,
							   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
							   FILE_OPEN,
							   FILE_DELETE_ON_CLOSE,
							   NULL, 
							   0,
							   IO_IGNORE_SHARE_ACCESS_CHECK);

		if(Status != STATUS_CANNOT_DELETE)
		{
			break;
		}

		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &DestFileHandle,
								  &DestFileObject,
								  GENERIC_READ | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
								  &DestObjectAttribute,
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
										 DestFileObject,
										 &DestFileBasicInfo,
										 sizeof(DestFileBasicInfo),
										 FileBasicInformation,
										 NULL);

		if(!NT_SUCCESS(Status))
		{
			//
			// DestFileObject will be close at the end
			break;
		}

		if(!FlagOn(DestFileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY))
		{
			//
			// STATUS_ACCESS_DENIED happened NOT because of read only
			//
			break;
		}

		ClearFlag(DestFileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY);

		Status = FltSetInformationFile(Instance,
									   DestFileObject,
									   &DestFileBasicInfo,
									   sizeof(DestFileBasicInfo),
									   FileBasicInformation);

		if(!NT_SUCCESS(Status))
		{
			break;
		}
		
		//
		// Close the destination file
		//
		if (DestFileHandle)
		{
			FltClose(DestFileHandle);
			DestFileHandle = NULL;
		}

		if (DestFileObject)
		{
			ObDereferenceObject(DestFileObject);
			DestFileObject = NULL;
		}

		Status = FltCreateFile(Global.Filter,
							   Instance,
							   &DestFileHandle,
							   DELETE,
							   &DestObjectAttribute,
							   &IoStatus,
							   NULL,
							   FILE_ATTRIBUTE_NORMAL,
							   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
							   FILE_OPEN,
							   FILE_DELETE_ON_CLOSE,
							   NULL, 
							   0,
							   IO_IGNORE_SHARE_ACCESS_CHECK);

	} while (FALSE);

	if (DestFileHandle)
	{
		FltClose(DestFileHandle);
		DestFileHandle = NULL;
	}

	if (DestFileObject)
	{
		ObDereferenceObject(DestFileObject);
		DestFileObject = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltEncryptFile(
	__in	PFLT_CALLBACK_DATA			Data,
	__in	PFLT_INSTANCE				SrcInstance,
	__in	PFLT_INSTANCE				DstInstance,
	__in	PUNICODE_STRING				SrcFileName,
	__in	PUNICODE_STRING				DstFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_FILTER		Filter = NULL;
	BOOLEAN			IgnoreCase = TRUE;
	HANDLE			SourceFileHandle = NULL;
	HANDLE			DestFileHandle = NULL;
	FILE_OBJECT		*SourceFileObject = NULL;
	FILE_OBJECT		*DestFileObject = NULL;
	OBJECT_ATTRIBUTES	SourceObjectAttribute;
	OBJECT_ATTRIBUTES	DestObjectAttribute;

	IO_STATUS_BLOCK		IoStatus = { 0 };

	FILE_STANDARD_INFORMATION FileStandardInfo = { 0 };
	FILE_POSITION_INFORMATION FilePositionInfo = { 0 };
	FILE_BASIC_INFORMATION	FileBasicInfo = { 0 };
	FILE_BASIC_INFORMATION	DestFileBasicInfo = { 0 };
	FILE_END_OF_FILE_INFORMATION	EndOfFileInfo = { 0 };

	UCHAR	*ReadBuffer = NULL;
	ULONG	ReadBufferLength = 0;
	ULONG	BytesRead = 0;
	LONG	DataLength = 0;
	ULONG	BytesWrite = 0;

	LARGE_INTEGER	ReadPosition = { 0 };
	LARGE_INTEGER	WritePosition = { 0 };

	ULONG LengthNeeded = 0;
	ULONG SecurityDescriptorLength = 0;
	SECURITY_DESCRIPTOR *pSecurityDescriptor = NULL;

	HANDLE			AesObject = NULL;
	BOOLEAN			IsGoodNXLFile = FALSE;
	NXL_HEADER		*NXLHdr = NULL;
	NXL_KEKEY_BLOB	PrimaryKey = {0};
	UCHAR			ContentKey[NXRMFLT_CONTENT_KEY_LENGTH];
	ULONG			ContentOffset = 0;
	LARGE_INTEGER	AllocateSize = {0};
	LARGE_INTEGER	FileSize = {0};
	

	do
	{
		Filter = Global.Filter;

		IgnoreCase = !(BooleanFlagOn(Data->Iopb->OperationFlags, SL_CASE_SENSITIVE));

		//if (!Global.ReparseInstance)
		//{
		//	Status = STATUS_INVALID_DEVICE_OBJECT_PARAMETER;

		//	break;
		//}

		InitializeObjectAttributes(&SourceObjectAttribute,
								   SrcFileName,
								   OBJ_KERNEL_HANDLE | (IgnoreCase ? OBJ_CASE_INSENSITIVE : 0),
								   NULL,
								   NULL);

		InitializeObjectAttributes(&DestObjectAttribute,
								   DstFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		FltAcquirePushLockShared(&Global.PrimaryKeyLock);

		memcpy(&PrimaryKey, 
			   &Global.PrimaryKey, 
			   min(sizeof(PrimaryKey),sizeof(Global.PrimaryKey)));

		FltReleasePushLock(&Global.PrimaryKeyLock);

		Status = FltCreateFileEx2(Filter,
								  DstInstance,
								  &DestFileHandle,
								  &DestFileObject,
								  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
								  &DestObjectAttribute,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OPEN,
								  FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  IO_IGNORE_SHARE_ACCESS_CHECK,
								  NULL);

		if (!NT_SUCCESS(Status))
		{
			if(Status != STATUS_ACCESS_DENIED)
			{
				break;
			}

			Status = FltCreateFileEx2(Filter,
									  DstInstance,
									  &DestFileHandle,
									  &DestFileObject,
									  GENERIC_READ | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
									  &DestObjectAttribute,
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

			Status = FltQueryInformationFile(DstInstance,
											 DestFileObject,
											 &DestFileBasicInfo,
											 sizeof(DestFileBasicInfo),
											 FileBasicInformation,
											 NULL);

			if(!NT_SUCCESS(Status))
			{
				//
				// DestFileObject will be close at the end
				break;
			}

			if(!FlagOn(DestFileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY))
			{
				//
				// STATUS_ACCESS_DENIED happened NOT because of read only
				//
				break;
			}

			ClearFlag(DestFileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY);

			Status = FltSetInformationFile(DstInstance,
										   DestFileObject,
										   &DestFileBasicInfo,
										   sizeof(DestFileBasicInfo),
										   FileBasicInformation);

			if(!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// Close the destination file
			//
			if (DestFileHandle)
			{
				FltClose(DestFileHandle);
				DestFileHandle = NULL;
			}

			if (DestFileObject)
			{
				ObDereferenceObject(DestFileObject);
			}

			//
			// try again after clear the read only flag
			//
			Status = FltCreateFileEx2(Filter,
									  DstInstance,
									  &DestFileHandle,
									  &DestFileObject,
									  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
									  &DestObjectAttribute,
									  &IoStatus,
									  NULL,
									  FILE_ATTRIBUTE_NORMAL,
									  FILE_SHARE_VALID_FLAGS,
									  FILE_OPEN,
									  FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
									  NULL,
									  0,
									  IO_IGNORE_SHARE_ACCESS_CHECK,
									  NULL);

			if(!NT_SUCCESS(Status))
			{
				break;
			}

			break;
		}

		Status = FltQueryInformationFile(DstInstance,
										 DestFileObject,
										 &FileStandardInfo,
										 sizeof(FileStandardInfo),
										 FileStandardInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		if(FileStandardInfo.EndOfFile.QuadPart < sizeof(NXL_HEADER))
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		NXLHdr = (NXL_HEADER *)ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), NXRMFLT_TMP_TAG);

		if(!NXLHdr)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		RtlSecureZeroMemory(NXLHdr, sizeof(*NXLHdr));

		Status = NXLCheck(DstInstance, DestFileObject, &IsGoodNXLFile);

		if(!NT_SUCCESS(Status) || (!IsGoodNXLFile))
		{
			break;
		}

		Status = NXLReadHeader(DstInstance, DestFileObject, NXLHdr);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		Status = NXLGetContentKey((PCNXL_HEADER)NXLHdr, PrimaryKey.Key, ContentKey);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		ContentOffset = NXLHdr->Basic.PointerOfContent;

		Status = NkCreateAesObject(ContentKey, 32, NXL_CBC_SIZE, &AesObject);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltCreateFileEx2(Filter,
								  SrcInstance,
								  &SourceFileHandle,
								  &SourceFileObject,
								  FILE_GENERIC_READ,
								  &SourceObjectAttribute,
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

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FileBasicInfo,
										 sizeof(FileBasicInfo),
										 FileBasicInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		////
		//// reuse FileStandardInfo for source file
		////
		//memset(&FileStandardInfo, 0, sizeof(FileStandardInfo));

		//Status = FltQueryInformationFile(SrcInstance,
		//								 SourceFileObject,
		//								 &FileStandardInfo,
		//								 sizeof(FileStandardInfo),
		//								 FileStandardInformation,
		//								 NULL);

		//if (!NT_SUCCESS(Status))
		//{
		//	break;
		//}

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FilePositionInfo,
										 sizeof(FilePositionInfo),
										 FilePositionInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		ReadBufferLength = NXRMFLT_READFILE_BUFFER_SIZE;

		ReadBuffer = ExAllocatePoolWithTag(PagedPool,
										   ReadBufferLength,
										   NXRMFLT_READFILE_TAG);

		if (!ReadBuffer)
		{
			break;
		}


		ReadPosition.QuadPart = 0;
		WritePosition.QuadPart = ContentOffset;

		memset(ReadBuffer, 0xcc, ReadBufferLength);

		AllocateSize.QuadPart	= 0;
		FileSize.QuadPart		= 0;
		DataLength				= 0;

		while (TRUE)
		{
			Status = FltReadFile(SrcInstance,
								 SourceFileObject,
								 &ReadPosition,
								 (ReadBufferLength - PAGE_SIZE),
								 ReadBuffer,
								 0,
								 &BytesRead,
								 NULL,
								 NULL);

			if (Status == STATUS_END_OF_FILE)
			{
				Status = STATUS_SUCCESS;
				break;
			}

			if (!NT_SUCCESS(Status) || !BytesRead)
			{
				break;
			}

			//
			// this should ONLY happen at the last block
			//
			if(BytesRead % PAGE_SIZE)
			{
				DataLength = ((BytesRead + (PAGE_SIZE - 1)) & (~(PAGE_SIZE - 1)));
			}
			else
			{
				DataLength = BytesRead;
			}

			//
			// update allocation size and file size
			//
			FileSize.QuadPart		+= BytesRead;
			AllocateSize.QuadPart	+= DataLength;

			Status = NkAesEncrypt(AesObject,
								  ReadPosition.QuadPart,
								  ReadBuffer,
								  DataLength);

			if(!NT_SUCCESS(Status))
			{
				break;
			}


			Status = FltWriteFile(DstInstance,
								  DestFileObject,
								  &WritePosition,
								  DataLength,
								  ReadBuffer,
								  0,
								  &BytesWrite,
								  NULL,
								  NULL);

			if (!NT_SUCCESS(Status) || DataLength != BytesWrite)
			{
				break;
			}

			WritePosition.QuadPart += DataLength;
			ReadPosition.QuadPart += BytesRead;

			memset(ReadBuffer, 0xcc, ReadBufferLength);
		}

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		//
		// Set EOF to truncate file if new content is less
		//
		EndOfFileInfo.EndOfFile = WritePosition;

		Status = FltSetInformationFile(DstInstance,
									   DestFileObject,
									   (PVOID)&EndOfFileInfo,
									   sizeof(EndOfFileInfo),
									   FileEndOfFileInformation);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		NXLHdr->Crypto.AllocateLength	= AllocateSize.QuadPart;
		NXLHdr->Crypto.ContentLength	= FileSize.QuadPart;

		Status = NXLWriteHeader(DstInstance,
								DestFileObject,
								NXLHdr);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		//
		// TO DO: copy security object from source to destination
		//
		do
		{

			//
			// OWNER_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											OWNER_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			if (!pSecurityDescriptor)
			{
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											OWNER_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  OWNER_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			memset(pSecurityDescriptor, 0, SecurityDescriptorLength);

			//
			// GROUP_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											GROUP_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											GROUP_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  GROUP_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// DACL_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											DACL_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											DACL_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  DACL_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// DACL_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											SACL_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											SACL_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  SACL_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

		} while (FALSE);

		if (pSecurityDescriptor)
		{
			ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);
			pSecurityDescriptor = NULL;
		}

		////
		//// restore source file attributes
		////
		//Status = FltSetInformationFile(SrcInstance,
		//							   SourceFileObject,
		//							   &FileStandardInfo,
		//							   sizeof(FileStandardInfo),
		//							   FileStandardInformation);

		Status = FltSetInformationFile(SrcInstance,
									   SourceFileObject,
									   &FilePositionInfo,
									   sizeof(FilePositionInfo),
									   FilePositionInformation);

		//
		// set destination file attributes
		//
		Status = FltSetInformationFile(DstInstance,
									   DestFileObject,
									   &FileBasicInfo,
									   sizeof(FileBasicInfo),
									   FileBasicInformation);

	} while (FALSE);

	if (SourceFileHandle)
	{
		FltClose(SourceFileHandle);
		SourceFileHandle = NULL;
	}

	if (DestFileHandle)
	{
		FltClose(DestFileHandle);
		DestFileHandle = NULL;
	}

	if (SourceFileObject)
	{
		ObDereferenceObject(SourceFileObject);
	}

	if (DestFileObject)
	{
		ObDereferenceObject(DestFileObject);
	}

	if (ReadBuffer)
	{
		ExFreePoolWithTag(ReadBuffer, NXRMFLT_READFILE_TAG);
		ReadBuffer = NULL;
	}

	if(AesObject)
	{
		NkDestroyAesObject(AesObject);
		AesObject = NULL;
	}

	if(NXLHdr)
	{
		ExFreePoolWithTag(NXLHdr, NXRMFLT_TMP_TAG);
		NXLHdr = NULL;
	}

	return Status;

}

ULONG nxrmfltExceptionFilter(__in NXRMFLT_STREAM_CONTEXT *Ctx, __in PEXCEPTION_POINTERS ExceptionPointer)
{
	NTSTATUS ExceptionCode = STATUS_SUCCESS;

	ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

	if (ExceptionCode == STATUS_IN_PAGE_ERROR)
	{
		if (ExceptionPointer->ExceptionRecord->NumberParameters >= 3)
		{
			ExceptionCode = (NTSTATUS)ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
		}
	}

	PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!got exception %x", ExceptionCode));

	return EXCEPTION_EXECUTE_HANDLER;
}

//NTSTATUS nxrmfltBuildRightsManagmentInfo(PFLT_INSTANCE Instance, PUNICODE_STRING FileName, NXL_KEKEY_BLOB *PrimaryKey, NXL_RM_INFO *RightsManagmentInfo)
//{
//	NTSTATUS Status = STATUS_SUCCESS;
//
//	PFLT_FILTER			Filter = NULL;
//	HANDLE				FileHandle = NULL;
//	FILE_OBJECT			*FileObject = NULL;
//	OBJECT_ATTRIBUTES	FileObjectAttribute;
//
//	IO_STATUS_BLOCK		IoStatus = { 0 };
//
//	FILE_STANDARD_INFORMATION FileStandardInfo = { 0 };
//
//	UCHAR	*ReadBuffer = NULL;
//	ULONG	ReadBufferLength = 0;
//	ULONG	BytesRead = 0;
//
//	BOOLEAN IsGoodNXLFile = FALSE;
//
//	UNICODE_STRING FileExtension = {0};
//
//	ULONG i = 0;
//
//	NXL_SECTION *nxlSection = NULL;
//	NXL_HEADER	*nxlHeader = NULL;
//
//	ULONG DataOffset = 0;
//	
//	do 
//	{
//		Filter = Global.Filter;
//
//		InitializeObjectAttributes(&FileObjectAttribute,
//								   FileName,
//								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
//								   NULL,
//								   NULL);
//
//		Status = FltCreateFileEx2(Filter,
//								  Instance,
//								  &FileHandle,
//								  &FileObject,
//								  FILE_GENERIC_READ,
//								  &FileObjectAttribute,
//								  &IoStatus,
//								  NULL,
//								  FILE_ATTRIBUTE_NORMAL,
//								  0,
//								  FILE_OPEN,
//								  FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
//								  NULL,
//								  0,
//								  IO_IGNORE_SHARE_ACCESS_CHECK,
//								  NULL);
//
//		if (!NT_SUCCESS(Status))
//		{
//			break;
//		}
//
//		Status = FltQueryInformationFile(Instance,
//										 FileObject,
//										 &FileStandardInfo,
//										 sizeof(FileStandardInfo),
//										 FileStandardInformation,
//										 NULL);
//
//		if (!NT_SUCCESS(Status))
//		{
//			break;
//		}
//
//		if(FileStandardInfo.EndOfFile.QuadPart < sizeof(NXL_HEADER))
//		{
//			Status = STATUS_INVALID_PARAMETER;
//			break;
//		}
//
//		ReadBufferLength = sizeof(NXL_HEADER);
//
//		ReadBuffer = ExAllocatePoolWithTag(PagedPool, ReadBufferLength, NXRMFLT_TMP_TAG);
//
//		if(!ReadBuffer)
//		{
//			Status = STATUS_INSUFFICIENT_RESOURCES;
//			break;
//		}
//
//		RtlSecureZeroMemory(ReadBuffer, ReadBufferLength);
//
//		Status = NXLCheck(Instance, FileObject, &IsGoodNXLFile);
//
//		if(!NT_SUCCESS(Status) || (!IsGoodNXLFile))
//		{
//			break;
//		}
//
//		nxlHeader = (NXL_HEADER *)ReadBuffer;
//
//		Status = NXLReadHeader(Instance, FileObject, nxlHeader);
//
//		if(!NT_SUCCESS(Status))
//		{
//			break;
//		}
//	
//		Status = NXLGetContentKey((PCNXL_HEADER)nxlHeader, PrimaryKey->Key, RightsManagmentInfo->ContentKey);
//
//		if(!NT_SUCCESS(Status))
//		{
//			break;
//		}
//
//		RightsManagmentInfo->DataOffset = nxlHeader->Basic.PointerOfContent;
//
//		RightsManagmentInfo->FileSize.QuadPart			= nxlHeader->Crypto.ContentLength;
//		RightsManagmentInfo->AllocationSize.QuadPart	= nxlHeader->Crypto.AllocateLength;
//
//		NXLGetOrignalFileExtension(FileName, &FileExtension);
//
//		RightsManagmentInfo->Extension.Buffer			= RightsManagmentInfo->ExtensionFastBuffer;
//		RightsManagmentInfo->Extension.MaximumLength	= sizeof(RightsManagmentInfo->ExtensionFastBuffer);
//		RightsManagmentInfo->Extension.Length			= FileExtension.Length;
//
//		memcpy(RightsManagmentInfo->ExtensionFastBuffer,
//			   FileExtension.Buffer,
//			   min(sizeof(RightsManagmentInfo->ExtensionFastBuffer), FileExtension.Length));
//
//	} while (FALSE);
//	
//	if (FileHandle)
//	{
//		FltClose(FileHandle);
//		FileHandle = NULL;
//	}
//
//	if (FileObject)
//	{
//		ObDereferenceObject(FileObject);
//	}
//
//	if(ReadBuffer)
//	{
//		ExFreePoolWithTag(ReadBuffer,NXRMFLT_TMP_TAG);
//	}
//
//	return Status;
//}

PVOID nxrmfltGetReparseECP(__in PFLT_CALLBACK_DATA Data)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PECP_LIST ecpList = NULL;

	NXRMFLT_REPARSE_ECP_CONTEXT *Ctx = NULL;

	do 
	{
		Status = FltGetEcpListFromCallbackData(Global.Filter, Data, &ecpList);

		if (NT_SUCCESS(Status) && (ecpList != NULL))
		{

			Status = FltFindExtraCreateParameter(Global.Filter,
												 ecpList,
												 &GUID_ECP_NXRMFLT_REPARSE,
												 &Ctx,
												 NULL);

			if (NT_SUCCESS(Status))
			{
				break;
			}
		}

	} while (FALSE);

	return Ctx;
}

NTSTATUS nxrmfltDeleteAddedReparseECP(
	__in PFLT_CALLBACK_DATA Data
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PECP_LIST ecpList = NULL;

	NXRMFLT_REPARSE_ECP_CONTEXT *Ctx = NULL;

	do 
	{
		Status = FltGetEcpListFromCallbackData(Global.Filter, Data, &ecpList);

		if (NT_SUCCESS(Status) && (ecpList != NULL))
		{

			Status = FltFindExtraCreateParameter(Global.Filter,
												 ecpList,
												 &GUID_ECP_NXRMFLT_REPARSE,
												 &Ctx,
												 NULL);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			FltFreeExtraCreateParameter(Global.Filter, Ctx);

		}
		else if (NT_SUCCESS(Status) && (ecpList == NULL))
		{
			Status = STATUS_NOT_FOUND;
		}
		else
		{

		}

	} while (FALSE);

	return Status;
}

NTSTATUS
	nxrmfltAddReparseECP(
	_Inout_		PFLT_CALLBACK_DATA			Data,
	__in		PFLT_FILE_NAME_INFORMATION	NameInfo,
	__in		PFLT_INSTANCE				Instance,
	_In_opt_	PUNICODE_STRING				SourceFileName
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PECP_LIST  ecpList = NULL;
	PNXRMFLT_REPARSE_ECP_CONTEXT ecpContext;

	do 
	{
		Status = FltGetEcpListFromCallbackData(Global.Filter,
											   Data,
											   &ecpList);
		if (!NT_SUCCESS(Status)) 
		{
			break;
		}

		if (ecpList == NULL) 
		{
			//
			// Create a new ecplist.
			//
			Status = FltAllocateExtraCreateParameterList(Global.Filter, 0, &ecpList);

			if (!NT_SUCCESS(Status)) 
			{
				break;
			}

			//
			// Set it into CBD.
			//
			Status = FltSetEcpListIntoCallbackData(Global.Filter, Data, ecpList);

			if (!NT_SUCCESS(Status))
			{
				FltFreeExtraCreateParameterList(Global.Filter, ecpList);
				break;
			}

		}
		else 
		{
			//
			// See if the ECP has already been added to the ECP list
			// already.
			//
			Status = FltFindExtraCreateParameter(Global.Filter,
												 ecpList,
												 &GUID_ECP_NXRMFLT_REPARSE,
												 NULL,
												 NULL);

			if (Status != STATUS_NOT_FOUND) 
			{
				break;
			}

		}

		Status = FltAllocateExtraCreateParameter(Global.Filter,
												 &GUID_ECP_NXRMFLT_REPARSE,
												 sizeof(NXRMFLT_REPARSE_ECP_CONTEXT),
												 0,
												 nxrmfltECPCleanupCallback,
												 NXRMFLT_ECP_CTX_TAG,
												 &ecpContext);

		if (!NT_SUCCESS(Status)) 
		{
			break;
		}

		memset(ecpContext, 0, sizeof(NXRMFLT_REPARSE_ECP_CONTEXT));

		FltReferenceFileNameInformation(NameInfo);

		ecpContext->NameInfo			= NameInfo;
		ecpContext->OriginalInstance	= Instance;

		if (SourceFileName)
		{
			if (SourceFileName->Length > sizeof(ecpContext->SourceFileNameBuffer))
			{
				ecpContext->SourceFileName.Buffer = ExAllocatePoolWithTag(PagedPool, SourceFileName->Length, NXRMFLT_ECPSRCFILENAME_TAG);

				if (!ecpContext->SourceFileName.Buffer)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
					FltFreeExtraCreateParameter(Global.Filter, ecpContext);
					break;
				}

				ecpContext->ReleaseSourceFileName = TRUE;

				ecpContext->SourceFileName.Length = 0;
				ecpContext->SourceFileName.MaximumLength = SourceFileName->Length;

			}
			else
			{
				ecpContext->ReleaseSourceFileName = FALSE;

				ecpContext->SourceFileName.Buffer = ecpContext->SourceFileNameBuffer;
				ecpContext->SourceFileName.Length = 0;
				ecpContext->SourceFileName.MaximumLength = sizeof(ecpContext->SourceFileNameBuffer);
			}

			RtlCopyUnicodeString(&ecpContext->SourceFileName, SourceFileName);
		}
		else
		{
			RtlInitUnicodeString(&ecpContext->SourceFileName, NULL);
		}

		Status = FltInsertExtraCreateParameter(Global.Filter,
											   ecpList,
											   ecpContext);
		if (!NT_SUCCESS(Status)) 
		{
			FltFreeExtraCreateParameter(Global.Filter, ecpContext);
			break;
		}

	} while (FALSE);

	return Status;
}


VOID nxrmfltECPCleanupCallback(
	_Inout_  PVOID EcpContext,
	_In_     LPCGUID EcpType
	)
{
	if (IsEqualGUID(EcpType, &GUID_ECP_NXRMFLT_REPARSE))
	{
		NXRMFLT_REPARSE_ECP_CONTEXT *Ctx = (NXRMFLT_REPARSE_ECP_CONTEXT*)EcpContext;

		if (Ctx->NameInfo)
		{
			FltReleaseFileNameInformation(Ctx->NameInfo);
			Ctx->NameInfo = NULL;
		}

		if (Ctx->ReleaseSourceFileName)
		{
			ExFreePoolWithTag(Ctx->SourceFileName.Buffer, NXRMFLT_ECPSRCFILENAME_TAG);
			
			RtlInitUnicodeString(&Ctx->SourceFileName, NULL);
		}
	}
	else if (IsEqualGUID(EcpType, &GUID_ECP_NXRMFLT_BLOCKING))
	{
		NXRMFLT_BLOCKING_ECP_CONTEXT *Ctx = (NXRMFLT_BLOCKING_ECP_CONTEXT*)EcpContext;

		if (Ctx->NameInfo)
		{
			FltReleaseFileNameInformation(Ctx->NameInfo);
			Ctx->NameInfo = NULL;
		}
	}
}

NTSTATUS nxrmfltAddBlockingECP(
	_Inout_		PFLT_CALLBACK_DATA			Data,
	__in		PFLT_FILE_NAME_INFORMATION	NameInfo
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PECP_LIST  ecpList = NULL;
	PNXRMFLT_BLOCKING_ECP_CONTEXT ecpContext;

	do
	{
		Status = FltGetEcpListFromCallbackData(Global.Filter,
											   Data,
											   &ecpList);
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		if (ecpList == NULL)
		{
			//
			// Create a new ecplist.
			//
			Status = FltAllocateExtraCreateParameterList(Global.Filter, 0, &ecpList);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// Set it into CBD.
			//
			Status = FltSetEcpListIntoCallbackData(Global.Filter, Data, ecpList);

			if (!NT_SUCCESS(Status))
			{
				FltFreeExtraCreateParameterList(Global.Filter, ecpList);
				break;
			}

		}
		else
		{
			//
			// See if the ECP has already been added to the ECP list
			// already.
			//
			Status = FltFindExtraCreateParameter(Global.Filter,
												 ecpList,
												 &GUID_ECP_NXRMFLT_BLOCKING,
												 NULL,
												 NULL);

			if (Status != STATUS_NOT_FOUND)
			{
				break;
			}

		}

		Status = FltAllocateExtraCreateParameter(Global.Filter,
												 &GUID_ECP_NXRMFLT_BLOCKING,
												 sizeof(NXRMFLT_BLOCKING_ECP_CONTEXT),
												 0,
												 nxrmfltECPCleanupCallback,
												 NXRMFLT_ECP_CTX_TAG,
												 &ecpContext);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		memset(ecpContext, 0, sizeof(NXRMFLT_BLOCKING_ECP_CONTEXT));

		FltReferenceFileNameInformation(NameInfo);

		ecpContext->NameInfo = NameInfo;

		Status = FltInsertExtraCreateParameter(Global.Filter,
											   ecpList,
											   ecpContext);
		if (!NT_SUCCESS(Status))
		{
			FltFreeExtraCreateParameter(Global.Filter, ecpContext);
			break;
		}

	} while (FALSE);

	return Status;
}

NXL_PROCESS_NODE* nxrmfltFindProcessNodeByProcessID(
	__in HANDLE		ProcessID
	)
{
	NXL_PROCESS_NODE *pNode = NULL;

	LIST_ENTRY *ite = NULL;

	do 
	{
		FOR_EACH_LIST(ite, &Global.NxlProcessList)
		{
			pNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

			if(pNode->ProcessId == ProcessID)
			{
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

	} while (FALSE);

	return pNode;
}

BOOLEAN nxrmfltDoesFileExist(
	__in PFLT_INSTANCE					Instance, 
	__in PUNICODE_STRING				FileName,
	__in BOOLEAN						IgnoreCase
	)
{
	BOOLEAN FileExists = FALSE;

	NTSTATUS Status = STATUS_SUCCESS;

	HANDLE		FileHandle = NULL;

	IO_STATUS_BLOCK IoStatus = { 0 };

	OBJECT_ATTRIBUTES	ObjectAttribute = { 0 };

	InitializeObjectAttributes(&ObjectAttribute,
							   FileName,
							   OBJ_KERNEL_HANDLE | (IgnoreCase ? OBJ_CASE_INSENSITIVE : 0),
							   NULL,
							   NULL);

	do 
	{
		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &FileHandle,
								  NULL,
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

		if(NT_SUCCESS(Status))
		{
			FileExists = TRUE;
		}

	} while (FALSE);
	
	if(FileHandle)
	{
		FltClose(FileHandle);
		FileHandle = NULL;
	}

	return FileExists;
}

BOOLEAN nxrmfltDoesFileExistEx(
	__in PFLT_INSTANCE					Instance, 
	__in PFLT_FILE_NAME_INFORMATION		NameInfo,
	__in BOOLEAN						IgnoreCase
	)
{
	return nxrmfltDoesFileExist(Instance, &NameInfo->Name, IgnoreCase);
}

NXL_CACHE_NODE *nxrmfltFindFirstCachNodeByParentDirectoryHash(
	__in ULONG ParentDirectoryHash
	)
{
	NXL_CACHE_NODE *pNode = NULL;

	rb_node *ite = NULL;

	RB_EACH_NODE(ite, &Global.NxlFileCache)
	{
		pNode = CONTAINING_RECORD(ite, NXL_CACHE_NODE, Node);

		if(pNode->ParentDirectoryHash == ParentDirectoryHash)
		{
			break;
		}
		else
		{
			pNode = NULL;
		}
	}

	return pNode;
}

NTSTATUS nxrmfltRenameOnDiskNXLFile(
	__in PFLT_INSTANCE		Instance, 
	__in PUNICODE_STRING	CurrentName,
	__in PUNICODE_STRING	NewName
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	HANDLE				FileHandle = NULL;
	PFILE_OBJECT		FileObject = NULL;

	OBJECT_ATTRIBUTES	ObjectAttributes = { 0 };

	IO_STATUS_BLOCK IoStatus = { 0 };

	PFILE_RENAME_INFORMATION	RenameInfo = NULL;

	ULONG Length = 0;

	do 
	{
		Length = sizeof(FILE_RENAME_INFORMATION) - sizeof(WCHAR) + NewName->Length;

		RenameInfo = ExAllocatePoolWithTag(PagedPool, Length, NXRMFLT_TMP_TAG);

		if(!RenameInfo)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		RenameInfo->ReplaceIfExists = TRUE;
		RenameInfo->RootDirectory	= NULL;
		RenameInfo->FileNameLength	= NewName->Length;

		memcpy(RenameInfo->FileName,
			   NewName->Buffer,
			   NewName->Length);

		InitializeObjectAttributes(&ObjectAttributes,
								   CurrentName,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
								   NULL,
								   NULL);

		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &FileHandle,
								  &FileObject,
								  DELETE,
								  &ObjectAttributes,
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

		Status = FltSetInformationFile(Instance,
									   FileObject,
									   RenameInfo,
									   Length,
									   FileRenameInformation);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

	} while (FALSE);

	if(FileHandle)
	{
		FltClose(FileHandle);
		FileHandle = NULL;
	}

	if(FileObject)
	{
		ObDereferenceObject(FileObject);
		FileObject = NULL;
	}

	if(RenameInfo)
	{
		ExFreePoolWithTag(RenameInfo, NXRMFLT_TMP_TAG);
		RenameInfo = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltRenameOnDiskNXLFileEx(
	__in PFLT_INSTANCE					Instance, 
	__in PUNICODE_STRING				CurrentName,
	__in PUNICODE_STRING				NewName,
	__in BOOLEAN						ReplaceIfExists,
	__in HANDLE							RootDirectory,
	__out PFLT_FILE_NAME_INFORMATION	*NewNameInfo
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	HANDLE				FileHandle = NULL;
	PFILE_OBJECT		FileObject = NULL;

	OBJECT_ATTRIBUTES	ObjectAttributes = { 0 };

	IO_STATUS_BLOCK IoStatus = { 0 };

	PFILE_RENAME_INFORMATION	RenameInfo = NULL;

	ULONG Length = 0;

	NTSTATUS QueryFileNameStatus = STATUS_SUCCESS;

	do 
	{
		*NewNameInfo = NULL;

		Length = sizeof(FILE_RENAME_INFORMATION) - sizeof(WCHAR) + NewName->Length;

		RenameInfo = ExAllocatePoolWithTag(PagedPool, Length, NXRMFLT_TMP_TAG);

		if(!RenameInfo)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		RenameInfo->ReplaceIfExists = ReplaceIfExists;
		RenameInfo->RootDirectory	= RootDirectory;
		RenameInfo->FileNameLength	= NewName->Length;

		memcpy(RenameInfo->FileName,
			   NewName->Buffer,
			   NewName->Length);

		InitializeObjectAttributes(&ObjectAttributes,
								   CurrentName,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
								   NULL,
								   NULL);

		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &FileHandle,
								  &FileObject,
								  DELETE,
								  &ObjectAttributes,
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

		Status = FltSetInformationFile(Instance,
									   FileObject,
									   RenameInfo,
									   Length,
									   FileRenameInformation);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		QueryFileNameStatus = FltGetFileNameInformationUnsafe(FileObject,
															  Instance,
															  FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
															  NewNameInfo);

		if (NT_SUCCESS(QueryFileNameStatus))
		{
			FltParseFileNameInformation(*NewNameInfo);
		}

	} while (FALSE);

	if(FileHandle)
	{
		FltClose(FileHandle);
		FileHandle = NULL;
	}

	if(FileObject)
	{
		ObDereferenceObject(FileObject);
		FileObject = NULL;
	}

	if(RenameInfo)
	{
		ExFreePoolWithTag(RenameInfo, NXRMFLT_TMP_TAG);
		RenameInfo = NULL;
	}

	return Status;

}

NTSTATUS nxrmfltRenameFile(
	__in PFLT_INSTANCE		Instance, 
	__in PUNICODE_STRING	CurrentName,
	__in PUNICODE_STRING	NewName
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	HANDLE				FileHandle = NULL;
	PFILE_OBJECT		FileObject = NULL;

	OBJECT_ATTRIBUTES	ObjectAttributes = { 0 };

	IO_STATUS_BLOCK IoStatus = { 0 };

	PFILE_RENAME_INFORMATION	RenameInfo = NULL;

	ULONG Length = 0;

	do 
	{
		Length = sizeof(FILE_RENAME_INFORMATION) - sizeof(WCHAR) + NewName->Length;

		RenameInfo = ExAllocatePoolWithTag(PagedPool, Length, NXRMFLT_TMP_TAG);

		if(!RenameInfo)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		RenameInfo->ReplaceIfExists = TRUE;
		RenameInfo->RootDirectory	= NULL;
		RenameInfo->FileNameLength	= NewName->Length;

		memcpy(RenameInfo->FileName,
			   NewName->Buffer,
			   NewName->Length);

		InitializeObjectAttributes(&ObjectAttributes,
									CurrentName,
									OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
									NULL,
									NULL);

		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &FileHandle,
								  &FileObject,
								  DELETE,
								  &ObjectAttributes,
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

		Status = FltSetInformationFile(Instance,
									   FileObject,
									   RenameInfo,
									   Length,
									   FileRenameInformation);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

	} while (FALSE);

	if(FileHandle)
	{
		FltClose(FileHandle);
		FileHandle = NULL;
	}

	if(FileObject)
	{
		ObDereferenceObject(FileObject);
		FileObject = NULL;
	}

	if(RenameInfo)
	{
		ExFreePoolWithTag(RenameInfo, NXRMFLT_TMP_TAG);
		RenameInfo = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltBuildNamesInStreamContext(
	__in NXRMFLT_STREAM_CONTEXT		*Ctx, 
	__in UNICODE_STRING				*FileName
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	do 
	{
		if(Ctx->ReleaseFileName)
		{
			ExFreeToPagedLookasideList(&Global.FullPathLookaside, Ctx->FileName.Buffer);
		}

		memset(Ctx->FileNameFastBuffer, 0, sizeof(Ctx->FileNameFastBuffer));

		RtlInitUnicodeString(&Ctx->FileName, NULL);
		RtlInitUnicodeString(&Ctx->FullPathParentDir, NULL);
		RtlInitUnicodeString(&Ctx->FinalComponent, NULL);

		if(FileName->Length > sizeof(Ctx->FileNameFastBuffer))
		{
			Ctx->FileName.Buffer = ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

			if(!Ctx->FileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			Ctx->ReleaseFileName = TRUE;

			memset(Ctx->FileName.Buffer, 0, NXRMFLT_FULLPATH_BUFFER_SIZE);

			Ctx->FileName.MaximumLength = NXRMFLT_FULLPATH_BUFFER_SIZE;
			Ctx->FileName.Length		= 0;
		}
		else
		{
			Ctx->FileName.Buffer		= Ctx->FileNameFastBuffer;
			Ctx->FileName.MaximumLength = sizeof(Ctx->FileNameFastBuffer);
			Ctx->FileName.Length		= 0;
		}

		RtlUnicodeStringCat(&Ctx->FileName, FileName);

		FltParseFileName(&Ctx->FileName, NULL, NULL, &Ctx->FinalComponent);

		Ctx->FullPathParentDir.Buffer			= Ctx->FileName.Buffer;
		Ctx->FullPathParentDir.MaximumLength	= Ctx->FullPathParentDir.Length = (Ctx->FileName.Length - Ctx->FinalComponent.Length);

	} while (FALSE);

	return Status;
}

NTSTATUS nxrmfltBuildRenameNodeFromCcb(
	__in NXRMFLT_STREAMHANDLE_CONTEXT	*Ccb,
	__inout NXL_RENAME_NODE				*RenameNode
	)
{
	NTSTATUS Status =STATUS_SUCCESS;

	do 
	{
		if(Ccb->SourceFileNameInfo->Name.Length > sizeof(RenameNode->SourceFileNameFastBuffer))
		{
			RenameNode->SourceFileName.Buffer = ExAllocatePoolWithTag(PagedPool, Ccb->SourceFileNameInfo->Name.Length, NXRMFLT_RENAMENODE_TAG);

			if(!RenameNode->SourceFileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			RenameNode->ReleaseSourceFileName = TRUE;

			RenameNode->SourceFileName.MaximumLength	= Ccb->SourceFileNameInfo->Name.Length;
			RenameNode->SourceFileName.Length			= 0;
		}
		else
		{
			RenameNode->ReleaseSourceFileName = FALSE;

			RenameNode->SourceFileName.Buffer			= RenameNode->SourceFileNameFastBuffer;
			RenameNode->SourceFileName.MaximumLength	= sizeof(RenameNode->SourceFileNameFastBuffer);
			RenameNode->SourceFileName.Length			= 0;
		}

		RtlUnicodeStringCat(&RenameNode->SourceFileName, &Ccb->SourceFileNameInfo->Name);

		if(Ccb->DestinationFileNameInfo->Name.Length > sizeof(RenameNode->DestinationFileNameFastBuffer))
		{
			RenameNode->DestinationFileName.Buffer = ExAllocatePoolWithTag(PagedPool, Ccb->DestinationFileNameInfo->Name.Length, NXRMFLT_RENAMENODE_TAG);

			if(!RenameNode->DestinationFileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			RenameNode->ReleaseDestinationFileName = TRUE;

			RenameNode->DestinationFileName.MaximumLength	= Ccb->DestinationFileNameInfo->Name.Length;
			RenameNode->DestinationFileName.Length			= 0;
		}
		else
		{
			RenameNode->ReleaseDestinationFileName = FALSE;

			RenameNode->DestinationFileName.Buffer			= RenameNode->DestinationFileNameFastBuffer;
			RenameNode->DestinationFileName.MaximumLength	= sizeof(RenameNode->DestinationFileNameFastBuffer);
			RenameNode->DestinationFileName.Length			= 0;
		}

		RtlUnicodeStringCat(&RenameNode->DestinationFileName, &Ccb->DestinationFileNameInfo->Name);

		RenameNode->SourceFileIsNxlFile = Ccb->SourceFileIsNxlFile;

	} while (FALSE);

	return Status;
}

VOID nxrmfltFreeRenameNode(
	__in NXL_RENAME_NODE	*RenameNode
	)
{
	if(RenameNode->ReleaseSourceFileName)
	{
		if(RenameNode->SourceFileName.Buffer)
		{
			ExFreePoolWithTag(RenameNode->SourceFileName.Buffer, NXRMFLT_RENAMENODE_TAG);
		}
	}

	RtlInitUnicodeString(&RenameNode->SourceFileName, NULL);

	if(RenameNode->ReleaseDestinationFileName)
	{
		if(RenameNode->DestinationFileName.Buffer)
		{
			ExFreePoolWithTag(RenameNode->DestinationFileName.Buffer, NXRMFLT_RENAMENODE_TAG);
		}
	}

	RtlInitUnicodeString(&RenameNode->DestinationFileName, NULL);
}

NXL_RENAME_NODE *nxrmfltFindRenameNodeFromCcb(
	__in NXRMFLT_STREAMHANDLE_CONTEXT	*Ccb
	)
{
	NXL_RENAME_NODE	*pNode = NULL;
	LIST_ENTRY *ite = NULL;

	FOR_EACH_LIST(ite, &Global.RenameList)
	{
		pNode = CONTAINING_RECORD(ite, NXL_RENAME_NODE, Link);

		if(RtlCompareUnicodeString(&pNode->DestinationFileName, &Ccb->DestinationFileNameInfo->Name, FALSE) == 0 &&
		   RtlCompareUnicodeString(&pNode->SourceFileName, &Ccb->SourceFileNameInfo->Name, FALSE) == 0)
		{
			break;
		}
		else
		{
			pNode = NULL;
		}
	}

	return pNode;
}

NTSTATUS nxrmfltCopyOnDiskNxlFile(
	__in PFLT_INSTANCE		SrcInstance,
	__in UNICODE_STRING		*SrcFileName,
	__in PFLT_INSTANCE		DstInstance,
	__in UNICODE_STRING		*DstFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_FILTER		Filter = NULL;
	HANDLE			SourceFileHandle = NULL;
	HANDLE			DestFileHandle = NULL;
	FILE_OBJECT		*SourceFileObject = NULL;
	FILE_OBJECT		*DestFileObject = NULL;
	OBJECT_ATTRIBUTES	SourceObjectAttribute;
	OBJECT_ATTRIBUTES	DestObjectAttribute;

	IO_STATUS_BLOCK		IoStatus = { 0 };

	FILE_STANDARD_INFORMATION FileStandardInfo = { 0 };
	FILE_POSITION_INFORMATION FilePositionInfo = { 0 };
	FILE_BASIC_INFORMATION	FileBasicInfo = { 0 };

	UCHAR	*ReadBuffer = NULL;
	ULONG	ReadBufferLength = 0;
	ULONG	BytesRead = 0;
	ULONG	BytesWrite = 0;

	LARGE_INTEGER	ReadPosition = { 0 };
	LARGE_INTEGER	WritePosition = { 0 };

	ULONG LengthNeeded = 0;
	ULONG SecurityDescriptorLength = 0;
	SECURITY_DESCRIPTOR *pSecurityDescriptor = NULL;

	do
	{
		Filter = Global.Filter;

		InitializeObjectAttributes(&SourceObjectAttribute,
								   SrcFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		InitializeObjectAttributes(&DestObjectAttribute,
								   DstFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		if (!SrcInstance)
		{
			Status = STATUS_INVALID_PARAMETER_1;
			break;
		}

		if (!DstInstance)
		{
			Status = STATUS_INVALID_PARAMETER_3;
			break;
		}

		Status = FltCreateFileEx2(Filter,
								  SrcInstance,
								  &SourceFileHandle,
								  &SourceFileObject,
								  GENERIC_READ | SYNCHRONIZE,
								  &SourceObjectAttribute,
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

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FileStandardInfo,
										 sizeof(FileStandardInfo),
										 FileStandardInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FilePositionInfo,
										 sizeof(FilePositionInfo),
										 FilePositionInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltQueryInformationFile(SrcInstance,
										 SourceFileObject,
										 &FileBasicInfo,
										 sizeof(FileBasicInfo),
										 FileBasicInformation,
										 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltCreateFileEx2(Filter,
								  DstInstance,
								  &DestFileHandle,
								  &DestFileObject,
								  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
								  &DestObjectAttribute,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OVERWRITE_IF,
								  FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  IO_IGNORE_SHARE_ACCESS_CHECK,
								  NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		ReadBufferLength = NXRMFLT_READFILE_BUFFER_SIZE;

		ReadBuffer = ExAllocatePoolWithTag(PagedPool,
										   ReadBufferLength,
										   NXRMFLT_READFILE_TAG);

		if (!ReadBuffer)
		{
			break;
		}

		ReadPosition.QuadPart = 0;
		WritePosition.QuadPart = 0;

		while (TRUE)
		{
			Status = FltReadFile(SrcInstance,
								 SourceFileObject,
								 &ReadPosition,
								 ReadBufferLength,
								 ReadBuffer,
								 0,
								 &BytesRead,
								 NULL,
								 NULL);

			if (Status == STATUS_END_OF_FILE)
			{
				Status = STATUS_SUCCESS;
				break;
			}

			if (!NT_SUCCESS(Status) || !BytesRead)
			{
				break;
			}

			Status = FltWriteFile(DstInstance,
								  DestFileObject,
								  &WritePosition,
								  BytesRead,
								  ReadBuffer,
								  0,
								  &BytesWrite,
								  NULL,
								  NULL);

			if (!NT_SUCCESS(Status) || BytesRead != BytesWrite)
			{
				break;
			}

			WritePosition.QuadPart += BytesRead;
			ReadPosition.QuadPart += BytesRead;
		}

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		do
		{

			//
			// OWNER_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											OWNER_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			if (!pSecurityDescriptor)
			{
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											OWNER_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  OWNER_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			memset(pSecurityDescriptor, 0, SecurityDescriptorLength);

			//
			// GROUP_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											GROUP_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											GROUP_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  GROUP_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// DACL_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											DACL_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											DACL_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  DACL_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			//
			// DACL_SECURITY_INFORMATION
			//
			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											SACL_SECURITY_INFORMATION,
											NULL,
											0,
											&LengthNeeded);

			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				break;
			}

			if (LengthNeeded > SecurityDescriptorLength)
			{
				ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);

				pSecurityDescriptor = NULL;

				pSecurityDescriptor = ExAllocatePoolWithTag(PagedPool, LengthNeeded, NXRMFLT_SECURITY_TAG);

			}

			if (!pSecurityDescriptor)
			{
				SecurityDescriptorLength = 0;
				break;
			}

			SecurityDescriptorLength = LengthNeeded;

			Status = FltQuerySecurityObject(SrcInstance,
											SourceFileObject,
											SACL_SECURITY_INFORMATION,
											pSecurityDescriptor,
											SecurityDescriptorLength,
											&LengthNeeded);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			Status = FltSetSecurityObject(DstInstance,
										  DestFileObject,
										  SACL_SECURITY_INFORMATION,
										  pSecurityDescriptor);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

		} while (FALSE);

		if (pSecurityDescriptor)
		{
			ExFreePoolWithTag(pSecurityDescriptor, NXRMFLT_SECURITY_TAG);
			pSecurityDescriptor = NULL;
		}

		//
		// restore source file attributes
		//

		Status = FltSetInformationFile(SrcInstance,
									   SourceFileObject,
									   &FilePositionInfo,
									   sizeof(FilePositionInfo),
									   FilePositionInformation);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltCopyOnDiskNxlFile: FltSetInformationFile failed to set FilePositionInformation. Status is %x\n", Status));
		}

		//
		// set destination file attributes
		//
		Status = FltSetInformationFile(DstInstance,
									   DestFileObject,
									   &FileBasicInfo,
									   sizeof(FileBasicInfo),
									   FileBasicInformation);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltCopyOnDiskNxlFile: FltSetInformationFile failed to set FileBasicInformation. Status is %x\n", Status));
		}

	} while (FALSE);

	if (SourceFileHandle)
	{
		FltClose(SourceFileHandle);
		SourceFileHandle = NULL;
	}

	if (DestFileHandle)
	{
		FltClose(DestFileHandle);
		DestFileHandle = NULL;
	}

	if (SourceFileObject)
	{
		ObDereferenceObject(SourceFileObject);
	}

	if (DestFileObject)
	{
		ObDereferenceObject(DestFileObject);
	}

	if (ReadBuffer)
	{
		ExFreePoolWithTag(ReadBuffer, NXRMFLT_READFILE_TAG);
		ReadBuffer = NULL;
	}

	return Status;

}

HANDLE nxrmfltReferenceReparseFile(
	__in PFLT_INSTANCE		Instance,
	__in UNICODE_STRING		*FileName)
{
	HANDLE FileHandle = NULL;

	NTSTATUS Status = STATUS_SUCCESS;

	OBJECT_ATTRIBUTES	ObjectAttributes = { 0 };

	IO_STATUS_BLOCK	IoStatus = { 0 };

	do 
	{
		InitializeObjectAttributes(&ObjectAttributes,
								   FileName,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
								   NULL,
								   NULL);
								  
		Status = FltCreateFileEx2(Global.Filter,
								  Instance,
								  &FileHandle,
								  NULL,
								  DELETE,
								  &ObjectAttributes,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OPEN,
								  FILE_NON_DIRECTORY_FILE | FILE_DELETE_ON_CLOSE | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  IO_IGNORE_SHARE_ACCESS_CHECK,
								  NULL);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

	} while (FALSE);

	return FileHandle;
}

VOID nxrmfltFreeProcessNode(
	__in NXL_PROCESS_NODE *ProcessNode
	)
{
	rb_node *ite = NULL;
	rb_node *tmp = NULL;

	RB_EACH_NODE_SAFE(ite, tmp, &ProcessNode->RightsCache)
	{
		NXL_RIGHTS_CACHE_NODE *pNode = CONTAINING_RECORD(ite, NXL_RIGHTS_CACHE_NODE, Node);

		rb_erase(ite, &ProcessNode->RightsCache);

		pNode->FileNameHash		= 0;
		pNode->RightsMask		= MAX_ULONGLONG;

		ExFreeToPagedLookasideList(&Global.NXLRightsCacheLookaside, pNode);
	}

	FltDeletePushLock(&ProcessNode->RightsCacheLock);

	ExFreeToPagedLookasideList(&Global.NXLProcessCacheLookaside, ProcessNode);

	return;
}

NTSTATUS nxrmfltCheckRights(
	__in HANDLE				ProcessId,
	__in HANDLE				ThreadId,
	__in NXL_CACHE_NODE		*pCacheNode,
	__in ULONG				IgnoreCache,
	__inout ULONGLONG		*RightsMask,
	__inout_opt ULONGLONG	*CustomRights,
	__inout_opt	ULONGLONG	*EvaluationId
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	LIST_ENTRY *ite = NULL;
	
	NXL_PROCESS_NODE *pProcessNode = NULL;
	NXL_PROCESS_NODE *pTmpNode = NULL;

	NXL_RIGHTS_CACHE_NODE *pRightsNode = NULL;

	NXRMFLT_CHECK_RIGHTS_REPLY	CheckRigthsReply = {0};

	NXRMFLT_NOTIFICATION *pNotification = NULL;

	BOOLEAN bFreeRightsNode = FALSE;

	ULONG ReplyLength = sizeof(CheckRigthsReply);
	LARGE_INTEGER MsgTimeout = { 0 };
	const LONGLONG _1ms = 10000;  

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	do 
	{
		if (ProcessId == Global.PortProcessId)
		{
			break;
		}

		Status = FltGetInstanceContext(pCacheNode->Instance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		FltAcquirePushLockShared(&Global.NxlProcessListLock);

		FOR_EACH_LIST(ite, &Global.NxlProcessList)
		{
			pProcessNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

			if (pProcessNode->ProcessId == ProcessId)
			{
				if (!ExAcquireRundownProtection(&pProcessNode->NodeRundownRef))
				{
					pProcessNode = NULL;
				}

				break;
			}
			else
			{
				pProcessNode = NULL;
			}
		}

		FltReleasePushLock(&Global.NxlProcessListLock);

		if (!pProcessNode)
		{
			pProcessNode = ExAllocateFromPagedLookasideList(&Global.NXLProcessCacheLookaside);

			if(pProcessNode)
			{
				pProcessNode->AlwaysDenyAccess		= FALSE;
				pProcessNode->AlwaysGrantAcess		= FALSE;
				pProcessNode->ProcessId				= ProcessId;
				pProcessNode->SessionId				= PsGetCurrentProcessSessionId();
				pProcessNode->RightsCache.rb_node	= NULL;
				pProcessNode->HideXNLExtension		= is_app_in_real_name_access_list(PsGetCurrentProcess()) ? FALSE : TRUE;
				pProcessNode->HasGraphicIntegration	= is_app_in_graphic_integration_list(PsGetCurrentProcess());

				FltInitializePushLock(&pProcessNode->RightsCacheLock);

				ExInitializeRundownProtection(&pProcessNode->NodeRundownRef);

				//
				// search the list again because we released lock
				//
				FltAcquirePushLockExclusive(&Global.NxlProcessListLock);

				FOR_EACH_LIST(ite, &Global.NxlProcessList)
				{
					pTmpNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

					if(pTmpNode->ProcessId == ProcessId)
					{
						break;
					}
					else
					{
						pTmpNode = NULL;
					}
				}

				if(!pTmpNode)
				{
					InsertHeadList(&Global.NxlProcessList, &pProcessNode->Link);

					if (!ExAcquireRundownProtection(&pProcessNode->NodeRundownRef))
					{
						pProcessNode = NULL;
					}
				}
				else
				{
					if (!ExAcquireRundownProtection(&pTmpNode->NodeRundownRef))
					{
						pTmpNode = NULL;
					}
				}

				FltReleasePushLock(&Global.NxlProcessListLock);

				//
				// there is a same record in the list already
				if(pTmpNode)
				{
					nxrmfltFreeProcessNode(pProcessNode);

					pProcessNode = pTmpNode;
				}
			}
			else
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}
		}

		//
		// grant access if process is in white list
		//
		if (pProcessNode->AlwaysGrantAcess)
		{
			CheckRigthsReply.RightsMask		= MAX_ULONGLONG;
			CheckRigthsReply.EvaluationId	= MAX_ULONGLONG;
			CheckRigthsReply.CustomRights	= MAX_ULONGLONG;

			break;
		}
		
		//
		// deny access if process is in black list
		//
		if (pProcessNode->AlwaysDenyAccess)
		{
			CheckRigthsReply.RightsMask		= 0;
			CheckRigthsReply.EvaluationId	= MAX_ULONGLONG;
			CheckRigthsReply.CustomRights	= 0;

			break;
		}

		if (IgnoreCache == FALSE)
		{
			FltAcquirePushLockShared(&pProcessNode->RightsCacheLock);

			pRightsNode = FindRightsNodeInCache(&pProcessNode->RightsCache, pCacheNode->FileNameHash);

			if (pRightsNode)
			{
				CheckRigthsReply.RightsMask = pRightsNode->RightsMask;
				CheckRigthsReply.EvaluationId = pRightsNode->EvaluationId;
				CheckRigthsReply.CustomRights = pRightsNode->CustomRights;
			}

			FltReleasePushLock(&pProcessNode->RightsCacheLock);
		}

		if (!pRightsNode)
		{
			//
			// need to send a a message to user mode now
			//

			do 
			{
				pNotification = ExAllocateFromPagedLookasideList(&Global.NotificationLookaside);

				if(!pNotification)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				memset(pNotification, 0, sizeof(*pNotification));

				pNotification->Type	= NXRMFLT_MSG_TYPE_CHECK_RIGHTS;

				pNotification->CheckRightsMsg.ProcessId	= (ULONG)(ULONG_PTR)ProcessId;
				pNotification->CheckRightsMsg.ThreadId	= (ULONG)(ULONG_PTR)ThreadId;

				if (InstCtx->VolDosName.Length && 
					InstCtx->VolumeProperties->RealDeviceName.Length && 
					InstCtx->VolumeProperties->RealDeviceName.Length > InstCtx->VolDosName.Length)
				{
					if (0 == memcmp(InstCtx->VolumeProperties->RealDeviceName.Buffer,
									pCacheNode->FileName.Buffer,
									min(InstCtx->VolumeProperties->RealDeviceName.Length, pCacheNode->FileName.Length)))
					{
						memcpy(pNotification->CheckRightsMsg.FileName, 
							   InstCtx->VolDosName.Buffer,
							   InstCtx->VolDosName.Length);

						memcpy((UCHAR*)pNotification->CheckRightsMsg.FileName + InstCtx->VolDosName.Length,
							   (UCHAR*)pCacheNode->FileName.Buffer + InstCtx->VolumeProperties->RealDeviceName.Length,
							   pCacheNode->FileName.Length - InstCtx->VolumeProperties->RealDeviceName.Length);
					}
				}
				else
				{
					memcpy(pNotification->CheckRightsMsg.FileName, 
						   pCacheNode->FileName.Buffer,
						   min(sizeof(pNotification->CheckRightsMsg.FileName) - sizeof(WCHAR), pCacheNode->FileName.Length));
				}

				MsgTimeout.QuadPart = NXRMFLT_MSG_TIMEOUT_IN_MS;

				MsgTimeout.QuadPart = -(MsgTimeout.QuadPart * _1ms);

				Status = FltSendMessage(Global.Filter,
										&Global.ClientPort,
										pNotification,
										sizeof(NXRMFLT_NOTIFICATION),
										&CheckRigthsReply,
										&ReplyLength,
										&MsgTimeout);

				if (Status == STATUS_SUCCESS)
				{
					//
					// Adding node to cache
					//

					if (!FlagOn(CheckRigthsReply.RightsMask, RIGHTS_NOT_CACHE))
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltCheckRights: Sending message to user mode and successfully received a reply\n"));

						pRightsNode = ExAllocateFromPagedLookasideList(&Global.NXLRightsCacheLookaside);

						if (pRightsNode)
						{
							pRightsNode->FileNameHash	= pCacheNode->FileNameHash;
							pRightsNode->RightsMask		= CheckRigthsReply.RightsMask;
							pRightsNode->CustomRights	= CheckRigthsReply.CustomRights;
							pRightsNode->EvaluationId	= CheckRigthsReply.EvaluationId;
						
							memset(pRightsNode->FileNameBuf, 0, sizeof(pRightsNode->FileNameBuf));

							memcpy(pRightsNode->FileNameBuf,
									pCacheNode->FileName.Buffer,
									min(sizeof(pRightsNode->FileNameBuf) - sizeof(WCHAR), pCacheNode->FileName.Length));

							RtlInitUnicodeString(&pRightsNode->FileName, pRightsNode->FileNameBuf);

							FltAcquirePushLockExclusive(&pProcessNode->RightsCacheLock);

							if (!AddRightsNodeToCache(&pProcessNode->RightsCache, pRightsNode))
							{
								//
								// Update cached result
								//
								NXL_RIGHTS_CACHE_NODE *pExistingNode = FindRightsNodeInCache(&pProcessNode->RightsCache, pRightsNode->FileNameHash);

								if (pExistingNode)
								{
									pExistingNode->RightsMask	= pRightsNode->RightsMask;
									pExistingNode->EvaluationId = pRightsNode->EvaluationId;
									pExistingNode->CustomRights = pRightsNode->CustomRights;
								}

								bFreeRightsNode = TRUE;
							}

							FltReleasePushLock(&pProcessNode->RightsCacheLock);

							if (bFreeRightsNode)
							{
								ExFreeToPagedLookasideList(&Global.NXLRightsCacheLookaside, pRightsNode);
								pRightsNode = NULL;
							}
						}
						else
						{
							Status = STATUS_INSUFFICIENT_RESOURCES;
						}
					}
					else
					{
						PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltCheckRights: Sending message to user mode and successfully received a reply. However, user mode demand no cache\n"));
					}
				}
				else if (Status == STATUS_TIMEOUT)
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltCheckRights: Sending message to user mode and timeout while waiting for reply\n"));
					CheckRigthsReply.RightsMask		= 0;
					CheckRigthsReply.CustomRights	= 0;
					CheckRigthsReply.EvaluationId	= MAX_ULONGLONG;
				}
				else
				{
					PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltCheckRights: Sending message to user mode and received an error while waiting for reply\n"));
				}

				ExFreeToPagedLookasideList(&Global.NotificationLookaside, pNotification);

			} while (FALSE);
		}

	} while (FALSE);

	if (NT_SUCCESS(Status))
	{
		*RightsMask		= CheckRigthsReply.RightsMask;

		if (EvaluationId)
			*EvaluationId	= CheckRigthsReply.EvaluationId;

		if (CustomRights)
			*CustomRights	= CheckRigthsReply.CustomRights;
	}

	if (pProcessNode)
	{
		ExReleaseRundownProtection(&pProcessNode->NodeRundownRef);
	}

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return Status;
}

NXL_RIGHTS_CACHE_NODE *
	FindRightsNodeInCache(
	rb_root			*Cache,
	ULONG			FileNameHash
	)
{
	NXL_RIGHTS_CACHE_NODE *pCacheNode = NULL;

	rb_node *ite = NULL;

	ite = Cache->rb_node;

	while (ite)
	{
		pCacheNode = CONTAINING_RECORD(ite, NXL_RIGHTS_CACHE_NODE, Node);

		if (pCacheNode->FileNameHash < FileNameHash)
			ite = ite->rb_left;
		else if (pCacheNode->FileNameHash > FileNameHash)
			ite = ite->rb_right;
		else
			return pCacheNode;
	}

	return NULL;
}

BOOLEAN
	AddRightsNodeToCache(
	rb_root					*CacheMap,
	NXL_RIGHTS_CACHE_NODE	*pRightsNode
	)
{
	NXL_RIGHTS_CACHE_NODE *pNode = NULL;

	rb_node **ite = NULL;
	rb_node *parent = NULL;

	ite = &(CacheMap->rb_node);
	parent = NULL;


	while (*ite)
	{
		pNode = CONTAINING_RECORD(*ite, NXL_RIGHTS_CACHE_NODE, Node);

		parent = *ite;

		if (pNode->FileNameHash < pRightsNode->FileNameHash)
			ite = &((*ite)->rb_left);
		else if (pNode->FileNameHash > pRightsNode->FileNameHash)
			ite = &((*ite)->rb_right);
		else
			return FALSE;
	}

	rb_link_node(&pRightsNode->Node, parent, ite);
	rb_insert_color(&pRightsNode->Node, CacheMap);

	return TRUE;
}

VOID NTAPI nxrmfltDeleteFileByNameWorkProc(
	__in PFLT_GENERIC_WORKITEM	FltWorkItem, 
	__in PVOID					FltObject, 
	__in_opt PVOID				Context)
{
	UNICODE_STRING FileName = {0};

	WCHAR *FullPathName = (WCHAR*) Context;
	PFLT_INSTANCE Instance = (PFLT_INSTANCE) FltObject;

	FltFreeGenericWorkItem(FltWorkItem);

	if (FullPathName)
	{
		RtlInitUnicodeString(&FileName, FullPathName); 

		nxrmfltDeleteFileByNameSync(Instance, &FileName);

		ExFreeToPagedLookasideList(&Global.FullPathLookaside, FullPathName);
	}

	return;
}

NTSTATUS nxrmfltDeleteFileByName(
	__in	PFLT_INSTANCE		Instance,
	__in	PUNICODE_STRING		FileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_GENERIC_WORKITEM FltWorkItem = NULL;
	WCHAR *FullPathName = NULL;

	do
	{
		FltWorkItem = FltAllocateGenericWorkItem();

		if (!FltWorkItem)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		FullPathName = ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

		if (!FullPathName)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(FullPathName, 0 , NXRMFLT_FULLPATH_BUFFER_SIZE);

		memcpy(FullPathName, 
			   FileName->Buffer, 
			   min(NXRMFLT_FULLPATH_BUFFER_SIZE - sizeof(WCHAR), FileName->Length));

		Status = FltQueueGenericWorkItem(FltWorkItem, 
										 Instance, 
										 nxrmfltDeleteFileByNameWorkProc, 
										 CriticalWorkQueue, 
										 FullPathName);

	} while(FALSE);

	if (!NT_SUCCESS(Status))
	{
		if (FullPathName)
			ExFreeToPagedLookasideList(&Global.FullPathLookaside, FullPathName);

		if (FltWorkItem)
			FltFreeGenericWorkItem(FltWorkItem);
	}

	return Status;
}

VOID NTAPI nxrmfltSetReadOnlyAttrByNameWorkProc(
	__in PFLT_GENERIC_WORKITEM	FltWorkItem,
	__in PVOID					FltObject,
	__in_opt PVOID				Context)
{
	PSET_READONLY_ATTR_CONTEXT SetReadOnlyAttrContext = Context;
	UNICODE_STRING FileName = {0};
	PFLT_INSTANCE Instance = (PFLT_INSTANCE) FltObject;

	FltFreeGenericWorkItem(FltWorkItem);

	if (SetReadOnlyAttrContext)
	{
		RtlInitUnicodeString(&FileName, SetReadOnlyAttrContext->FileNameBuf);

		OBJECT_ATTRIBUTES ObjectAttribute = { 0 };

		InitializeObjectAttributes(&ObjectAttribute,
								   &FileName,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
								   NULL,
								   NULL);

		HANDLE FileHandle;
		FILE_OBJECT *FileObject;
		IO_STATUS_BLOCK IoStatus;
		NTSTATUS Status;

		Status = FltCreateFileEx(Global.Filter,
								 Instance,
								 &FileHandle,
								 &FileObject,
								 FILE_WRITE_ATTRIBUTES,
								 &ObjectAttribute,
								 &IoStatus,
								 NULL,
								 FILE_ATTRIBUTE_NORMAL,
								 FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
								 FILE_OPEN,
								 0,
								 NULL,
								 0,
								 IO_IGNORE_SHARE_ACCESS_CHECK);
		if (NT_SUCCESS(Status))
		{
			FILE_BASIC_INFORMATION FileBasicInfo;
			ULONG lengthReturned;

			Status = FltQueryInformationFile(Instance,
											 FileObject,
											 &FileBasicInfo,
											 sizeof FileBasicInfo,
											 FileBasicInformation,
											 &lengthReturned);
			if (NT_SUCCESS(Status))
			{
				if (BooleanFlagOn(FileBasicInfo.FileAttributes, FILE_ATTRIBUTE_READONLY) != SetReadOnlyAttrContext->ReadOnlyAttr)
				{
					FileBasicInfo.FileAttributes ^= FILE_ATTRIBUTE_READONLY;
					Status = FltSetInformationFile(Instance,
												   FileObject,
												   &FileBasicInfo,
												   sizeof FileBasicInfo,
												   FileBasicInformation);
				}
			}

			Status = FltClose(FileHandle);
		}

		ExFreeToPagedLookasideList(&Global.SetReadOnlyAttrCtxLookaside, SetReadOnlyAttrContext);
	}

	return;
}

NTSTATUS nxrmfltSetReadOnlyAttrByName(
	__in	PFLT_INSTANCE		Instance,
	__in	PUNICODE_STRING		FileName,
	__in	BOOLEAN				ReadOnlyAttr)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_GENERIC_WORKITEM FltWorkItem = NULL;
	PSET_READONLY_ATTR_CONTEXT SetReadOnlyAttrContext = NULL;

	do
	{
		FltWorkItem = FltAllocateGenericWorkItem();

		if (!FltWorkItem)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		SetReadOnlyAttrContext = ExAllocateFromPagedLookasideList(&Global.SetReadOnlyAttrCtxLookaside);

		if (!SetReadOnlyAttrContext)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(SetReadOnlyAttrContext->FileNameBuf, 0 , sizeof SetReadOnlyAttrContext->FileNameBuf);

		memcpy(SetReadOnlyAttrContext->FileNameBuf,
			   FileName->Buffer,
			   min(sizeof SetReadOnlyAttrContext->FileNameBuf - sizeof(WCHAR), FileName->Length));

		SetReadOnlyAttrContext->ReadOnlyAttr = ReadOnlyAttr;

		Status = FltQueueGenericWorkItem(FltWorkItem,
										 Instance,
										 nxrmfltSetReadOnlyAttrByNameWorkProc,
										 CriticalWorkQueue,
										 SetReadOnlyAttrContext);

	} while(FALSE);

	if (!NT_SUCCESS(Status))
	{
		if (SetReadOnlyAttrContext)
			ExFreeToPagedLookasideList(&Global.SetReadOnlyAttrCtxLookaside, SetReadOnlyAttrContext);

		if (FltWorkItem)
			FltFreeGenericWorkItem(FltWorkItem);
	}

	return Status;
}

NTSTATUS nxrmfltGetDeviceInfo(
	IN		PDEVICE_OBJECT				TargetDeviceObject,
	IN OUT  WCHAR						*DeviceName,
	IN		ULONG						DeviceNameLength,
	IN OUT	WCHAR						*SerialNumber,
	IN		ULONG						SerialNumberLength,
	IN OUT	STORAGE_BUS_TYPE			*BusType
	)
{
	NTSTATUS					status = STATUS_SUCCESS;
	IRP							*pQuery_irp = NULL;
	STORAGE_DEVICE_DESCRIPTOR	*pStorage_device_descriptor = NULL;
	STORAGE_PROPERTY_QUERY		Query;
	ULONG						Storage_device_descriptor_size = 0;
	IO_STATUS_BLOCK				ioStatusBlock;
	PCHAR						psn = NULL;
	PCHAR						pvendorid = NULL;
	PCHAR						pproductid = NULL;
	ULONG						uvendorid_length = 0;
	ULONG						uproductid_length = 0;
	KEVENT						ioEvent;

	Query.QueryType		= PropertyStandardQuery;
	Query.PropertyId	= StorageDeviceProperty;

	Storage_device_descriptor_size = (sizeof(STORAGE_DEVICE_DESCRIPTOR)	+ 4095) & (~4095);	// 4K is enough

	do 
	{
		pStorage_device_descriptor = (PSTORAGE_DEVICE_DESCRIPTOR)ExAllocatePoolWithTag(PagedPool, Storage_device_descriptor_size, NXRMFLT_TMP_TAG);

		if(pStorage_device_descriptor == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(pStorage_device_descriptor,0,Storage_device_descriptor_size);

		KeInitializeEvent(&ioEvent, NotificationEvent, FALSE);

		pQuery_irp = IoBuildDeviceIoControlRequest(IOCTL_STORAGE_QUERY_PROPERTY,
												   TargetDeviceObject,
												   &Query,
												   sizeof(Query),
												   pStorage_device_descriptor,
												   Storage_device_descriptor_size,
												   FALSE,
												   &ioEvent,
												   &ioStatusBlock);

		if (pQuery_irp == NULL) 
		{
			status = ioStatusBlock.Status;
			break;
		}


		status = IoCallDriver(TargetDeviceObject,pQuery_irp);

		if(status == STATUS_PENDING)
		{
			status = KeWaitForSingleObject((PVOID)&ioEvent,
										   Executive,
										   KernelMode,
										   FALSE,
										   NULL);


		}

		if(status != STATUS_SUCCESS)
		{
			break;
		}

		if((pStorage_device_descriptor->SerialNumberOffset >= pStorage_device_descriptor->Size) || 
		   (pStorage_device_descriptor->SerialNumberOffset == 0))
		{
			psn = "";
		}
		else
		{
			psn = (PCHAR)((PCHAR)pStorage_device_descriptor + pStorage_device_descriptor->SerialNumberOffset);
			make_device_string_user_friendly(psn,(ULONG)strlen(psn));
		}

		if((pStorage_device_descriptor->VendorIdOffset >= pStorage_device_descriptor->Size) || 
			(pStorage_device_descriptor->VendorIdOffset == 0))
		{
			pvendorid = "";
		}
		else
		{
			pvendorid = (PCHAR)((PCHAR)pStorage_device_descriptor + pStorage_device_descriptor->VendorIdOffset);
			make_device_string_user_friendly(pvendorid,(ULONG)strlen(pvendorid));
		}

		if((pStorage_device_descriptor->ProductIdOffset >= pStorage_device_descriptor->Size) || 
			(pStorage_device_descriptor->ProductIdOffset == 0))
		{
			pproductid = "";
		}
		else
		{
			pproductid = (PCHAR)((PCHAR)pStorage_device_descriptor + pStorage_device_descriptor->ProductIdOffset);
			make_device_string_user_friendly(pproductid,(ULONG)strlen(pproductid));
		}

		*BusType = pStorage_device_descriptor->BusType;

		//
		// Serial number
		//

		RtlStringCbPrintfW(SerialNumber,
						   SerialNumberLength,
						   L"%S",
						   psn);

		uvendorid_length	= strlen(pvendorid);
		uproductid_length	= strlen(pproductid);

		//
		// Device name
		//
		if(uvendorid_length && uproductid_length)
		{
			RtlStringCbPrintfW(DeviceName,
							   DeviceNameLength,
							   L"%S %S",
							   pvendorid,
							   pproductid);
		}
		else if(uvendorid_length == 0 && uproductid_length)
		{
			RtlStringCbPrintfW(DeviceName,
							   DeviceNameLength,
							   L"%S",
							   pproductid);
		}
		else if(uvendorid_length && uproductid_length == 0)
		{
			RtlStringCbPrintfW(DeviceName,
							   DeviceNameLength,
							   L"%S",
							   pvendorid);
		}
		else
		{
			//
			// no name
			//
			RtlStringCbPrintfW(DeviceName,
							   DeviceNameLength,
							   L"%s",
							   L"");

		}


	} while (FALSE);

	if(pStorage_device_descriptor)
	{
		ExFreePoolWithTag(pStorage_device_descriptor, NXRMFLT_TMP_TAG);
		pStorage_device_descriptor = NULL;
	}

	return status;
}

static void make_device_string_user_friendly(char *device_string, ULONG length)
{
	char	tmp[NXRMFLT_MAX_PATH] = {0};
	char	*p = NULL;

	do 
	{
		if(!length)
		{
			break;
		}

		//
		// start processing vendor ID
		//
		memset(tmp,0,sizeof(tmp));

		p = device_string;

		while(myisalnum(*p) == 0 && (*p) != '\0')
		{
			p++;
		}

		memcpy(tmp,
			   p,
			   min(sizeof(tmp)-sizeof(char),strlen(p)));

		p = tmp + strlen(tmp);

		while(myisalnum(*p) == 0 && (ULONG_PTR)p > (ULONG_PTR)tmp)
		{
			*p = '\0';
			p--;
		}

		//
		// erase device string content before copying new content
		//
		memset(device_string,0,length);

		memcpy(device_string,
			   tmp,
			   min(length,sizeof(tmp)-sizeof(char)));		// the "length" does not include ending '\0'

	} while (FALSE);

	return;
}

static BOOLEAN myisalnum(int c)
{
	BOOLEAN bRet = FALSE;


	if(c >= '0' && c <= '9')
	{
		bRet = TRUE;
	}
	else if(c >= 'A' && c <= 'Z')
	{
		bRet = TRUE;
	}
	else if(c >= 'a' && c <= 'z')
	{
		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOLEAN nxrmfltIsProcessDirty(IN HANDLE ProcessId)
{
	BOOLEAN bRet = FALSE;

	LIST_ENTRY *ite = NULL;

	NXL_PROCESS_NODE *pProcessNode = NULL;

	do 
	{
		FltAcquirePushLockShared(&Global.NxlProcessListLock);

		FOR_EACH_LIST(ite, &Global.NxlProcessList)
		{
			pProcessNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

			if (pProcessNode->ProcessId == ProcessId)
			{
				bRet = TRUE;
				break;
			}
		}

		FltReleasePushLock(&Global.NxlProcessListLock);

	} while (FALSE);

	return bRet;
}

NTSTATUS nxrmfltGuessSourceFileFromProcessCache(IN HANDLE ProcessId, IN OUT PUNICODE_STRING SourceFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	LIST_ENTRY *ite = NULL;

	rb_node	*rb_ite = NULL;
	
	NXL_PROCESS_NODE *pProcessNode = NULL;

	NXL_RIGHTS_CACHE_NODE *pFirstRightsCacheNode = NULL;

	do 
	{
		FltAcquirePushLockShared(&Global.NxlProcessListLock);

		FOR_EACH_LIST(ite, &Global.NxlProcessList)
		{
			pProcessNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

			if (pProcessNode->ProcessId == ProcessId)
			{
				if (!ExAcquireRundownProtection(&pProcessNode->NodeRundownRef))
				{
					pProcessNode = NULL;
				}

				break;
			}
			else
			{
				pProcessNode = NULL;
			}
		}

		FltReleasePushLock(&Global.NxlProcessListLock);

		if (!pProcessNode)
		{
			Status = STATUS_NOT_FOUND;
			break;
		}

		FltAcquirePushLockShared(&pProcessNode->RightsCacheLock);

		RB_EACH_NODE(rb_ite, &pProcessNode->RightsCache)
		{
			pFirstRightsCacheNode = CONTAINING_RECORD(rb_ite, NXL_RIGHTS_CACHE_NODE, Node);

			if (FlagOn(pFirstRightsCacheNode->RightsMask, BUILTIN_RIGHT_VIEW))
			{
				break;
			}
			else
			{
				pFirstRightsCacheNode = NULL;
			}
		}

		if (pFirstRightsCacheNode)
		{
			RtlCopyUnicodeString(SourceFileName, &pFirstRightsCacheNode->FileName);
		}
		else
		{
			Status = STATUS_NOT_FOUND;
		}

		FltReleasePushLock(&pProcessNode->RightsCacheLock);

	} while (FALSE);

	if (pProcessNode)
	{
		ExReleaseRundownProtection(&pProcessNode->NodeRundownRef);
		pProcessNode = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltSendBlockNotificationMsg(IN PFLT_CALLBACK_DATA Data, IN PUNICODE_STRING FileName, IN NXRMFLT_BLOCK_REASON Reason)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_NOTIFICATION *pNotification = NULL;

	LARGE_INTEGER MsgTimeout = { 0 };
	const LONGLONG _1ms = 10000;  

	ULONG ReplyLength = 0;

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	do 
	{
		if (!Global.ClientPort)
		{
			break;
		}

		Status = FltGetInstanceContext(Data->Iopb->TargetInstance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		pNotification = ExAllocateFromPagedLookasideList(&Global.NotificationLookaside);

		if(!pNotification)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(pNotification, 0, sizeof(*pNotification));

		pNotification->Type	= NXRMFLT_MSG_TYPE_BLOCK_NOTIFICATION;

		pNotification->BlockMsg.ProcessId	= FltGetRequestorProcessId(Data);
		pNotification->BlockMsg.ThreadId	= (ULONG)(ULONG_PTR)(Data->Thread ? PsGetThreadId(Data->Thread) : PsGetCurrentThreadId());
		pNotification->BlockMsg.Reason		= Reason;
		pNotification->BlockMsg.SessionId	= PsGetCurrentProcessSessionId();
	
		if (InstCtx->VolDosName.Length && 
			InstCtx->VolumeProperties->RealDeviceName.Length && 
			InstCtx->VolumeProperties->RealDeviceName.Length > InstCtx->VolDosName.Length)
		{
			if (0 == memcmp(InstCtx->VolumeProperties->RealDeviceName.Buffer,
							FileName->Buffer,
							min(InstCtx->VolumeProperties->RealDeviceName.Length, FileName->Length)))
			{
				memcpy(pNotification->BlockMsg.FileName, 
					   InstCtx->VolDosName.Buffer,
					   InstCtx->VolDosName.Length);

				memcpy((UCHAR*)pNotification->BlockMsg.FileName + InstCtx->VolDosName.Length,
					   (UCHAR*)FileName->Buffer + InstCtx->VolumeProperties->RealDeviceName.Length,
					   FileName->Length - InstCtx->VolumeProperties->RealDeviceName.Length);
			}
		}
		else
		{
			memcpy(pNotification->BlockMsg.FileName,
				   FileName->Buffer,
				   min(sizeof(pNotification->BlockMsg.FileName) - sizeof(WCHAR), FileName->Length));
		}
	
		MsgTimeout.QuadPart = NXRMFLT_MSG_TIMEOUT_IN_MS;

		MsgTimeout.QuadPart = -(MsgTimeout.QuadPart * _1ms);

		Status = FltSendMessage(Global.Filter,
								&Global.ClientPort,
								pNotification,
								sizeof(NXRMFLT_NOTIFICATION),
								NULL,
								&ReplyLength,
								&MsgTimeout);

		if (Status == STATUS_SUCCESS)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendBlockNotificationMsg: Successfully sent a block notification message to user mode\n"));
		}
		else if (Status == STATUS_TIMEOUT)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendBlockNotificationMsg: Timeout while sending a block notification message to user mode\n"));
		}
		else
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendBlockNotificationMsg: Sending a block notification message to user mode and received an error. Status is %x\n", Status));
		}

		ExFreeToPagedLookasideList(&Global.NotificationLookaside, pNotification);

		pNotification = NULL;

	} while (FALSE);

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return Status;
}

NTSTATUS nxrmfltSendKeyChainErrorMsg(IN PFLT_INSTANCE Instance, IN PUNICODE_STRING FileName, IN NXL_KEKEY_ID *KeyId)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_NOTIFICATION *pNotification = NULL;

	LARGE_INTEGER MsgTimeout = { 0 };
	const LONGLONG _1ms = 10000;  

	ULONG ReplyLength = 0;

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	do 
	{
		if (!Global.ClientPort)
		{
			break;
		}

		Status = FltGetInstanceContext(Instance, &InstCtx);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		pNotification = ExAllocateFromPagedLookasideList(&Global.NotificationLookaside);

		if(!pNotification)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(pNotification, 0, sizeof(*pNotification));

		pNotification->Type	= NXRMFLT_MSG_TYPE_KEYCHAIN_ERROR_NOTIFICATION;
		
		if (InstCtx->VolDosName.Length && 
			InstCtx->VolumeProperties->RealDeviceName.Length && 
			InstCtx->VolumeProperties->RealDeviceName.Length > InstCtx->VolDosName.Length)
		{
			if (0 == memcmp(InstCtx->VolumeProperties->RealDeviceName.Buffer,
							FileName->Buffer,
							min(InstCtx->VolumeProperties->RealDeviceName.Length, FileName->Length)))
			{
				memcpy(pNotification->KeyChainErrorMsg.FileName, 
					   InstCtx->VolDosName.Buffer,
					   InstCtx->VolDosName.Length);

				memcpy((UCHAR*)pNotification->KeyChainErrorMsg.FileName + InstCtx->VolDosName.Length,
					   (UCHAR*)FileName->Buffer + InstCtx->VolumeProperties->RealDeviceName.Length,
					   FileName->Length - InstCtx->VolumeProperties->RealDeviceName.Length);
			}
		}
		else
		{
			memcpy(pNotification->KeyChainErrorMsg.FileName,
				   FileName->Buffer,
				   min(sizeof(pNotification->KeyChainErrorMsg.FileName) - sizeof(WCHAR), FileName->Length));
		}

		pNotification->KeyChainErrorMsg.KeyId		= *KeyId;
		pNotification->KeyChainErrorMsg.SessionId	= PsGetCurrentProcessSessionId();

		MsgTimeout.QuadPart = NXRMFLT_MSG_TIMEOUT_IN_MS;

		MsgTimeout.QuadPart = -(MsgTimeout.QuadPart * _1ms);

		Status = FltSendMessage(Global.Filter,
								&Global.ClientPort,
								pNotification,
								sizeof(NXRMFLT_NOTIFICATION),
								NULL,
								&ReplyLength,
								&MsgTimeout);

		if (Status == STATUS_SUCCESS)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendKeyChainErrorMsg: Successfully sent a block notification message to user mode\n"));
		}
		else if (Status == STATUS_TIMEOUT)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendKeyChainErrorMsg: Timeout while sending a block notification message to user mode\n"));
		}
		else
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendKeyChainErrorMsg: Sending a block notification message to user mode and received an error. Status is %x\n", Status));
		}

		ExFreeToPagedLookasideList(&Global.NotificationLookaside, pNotification);

		pNotification = NULL;

	} while (FALSE);

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return Status;
}

NTSTATUS nxrmfltCopyTags(
	__in PFLT_INSTANCE		SrcInstance,
	__in UNICODE_STRING		*SrcFileName,
	__in PFLT_INSTANCE		DstInstance,
	__in UNICODE_STRING		*DstFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_FILTER		Filter = NULL;
	HANDLE			SourceFileHandle = NULL;
	HANDLE			DestFileHandle = NULL;
	FILE_OBJECT		*SourceFileObject = NULL;
	FILE_OBJECT		*DestFileObject = NULL;
	OBJECT_ATTRIBUTES	SourceObjectAttribute;
	OBJECT_ATTRIBUTES	DestObjectAttribute;

	IO_STATUS_BLOCK		IoStatus = { 0 };

	UCHAR	*ReadBuffer = NULL;
	ULONG	ReadBufferLength = 0;
	ULONG	BytesRead = 0;

	BOOLEAN			IsGoodNXLFile = FALSE;
	NXL_HEADER		*NXLHdr = NULL;
	NXL_KEKEY_BLOB	PrimaryKey = {0};
	UCHAR			ContentKey[NXRMFLT_CONTENT_KEY_LENGTH] = { 0 };
	LIST_ENTRY		*ite = NULL;
	NXL_KEYCHAIN_NODE *pNode = NULL;

	do 
	{
		Filter = Global.Filter;

		InitializeObjectAttributes(&SourceObjectAttribute,
								   SrcFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		InitializeObjectAttributes(&DestObjectAttribute,
								   DstFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		Status = FltCreateFileEx2(Filter,
								  SrcInstance,
								  &SourceFileHandle,
								  &SourceFileObject,
								  GENERIC_READ | SYNCHRONIZE,
								  &SourceObjectAttribute,
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

		Status = NXLCheck(SrcInstance, SourceFileObject, &IsGoodNXLFile);

		if(!NT_SUCCESS(Status) || (!IsGoodNXLFile))
		{
			if (!IsGoodNXLFile)
			{
				Status = STATUS_INVALID_PARAMETER;
			}

			break;
		}

		Status = FltCreateFileEx2(Filter,
								  DstInstance,
								  &DestFileHandle,
								  &DestFileObject,
								  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
								  &DestObjectAttribute,
								  &IoStatus,
								  NULL,
								  FILE_ATTRIBUTE_NORMAL,
								  FILE_SHARE_VALID_FLAGS,
								  FILE_OPEN,
								  FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
								  NULL,
								  0,
								  IO_IGNORE_SHARE_ACCESS_CHECK,
								  NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = NXLCheck(DstInstance, DestFileObject, &IsGoodNXLFile);

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

		Status = NXLReadHeader(DstInstance, DestFileObject, NXLHdr);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		FltAcquirePushLockShared(&Global.KeyChainLock);

		FOR_EACH_LIST(ite, &Global.KeyChain)
		{
			pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

			if (memcmp(pNode->KeyBlob.KeyId.Id, NXLHdr->Crypto.PrimaryKey.KeKeyId.Id, min(pNode->KeyBlob.KeyId.IdSize, NXLHdr->Crypto.PrimaryKey.KeKeyId.IdSize)) == 0 &&
				pNode->KeyBlob.KeyId.Algorithm	== NXLHdr->Crypto.PrimaryKey.KeKeyId.Algorithm)
			{
				memcpy(&PrimaryKey, 
					   &pNode->KeyBlob, 
					   min(sizeof(PrimaryKey),sizeof(pNode->KeyBlob)));

				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		FltReleasePushLock(&Global.KeyChainLock);

		if (!pNode)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL,
						 ("nxrmflt!nxrmfltCopyTags: Can't find key in current key chain to set tags for file %wZ.", DstFileName));

			nxrmfltSendKeyChainErrorMsg(DstInstance, DstFileName, &NXLHdr->Crypto.PrimaryKey.KeKeyId);

			Status = STATUS_DECRYPTION_FAILED;
			break;
		}

		Status = NXLGetContentKey((PCNXL_HEADER)NXLHdr, PrimaryKey.Key, ContentKey);

		if(!NT_SUCCESS(Status))
		{
			break;
		}


		ReadBufferLength = NXRMFLT_READFILE_BUFFER_SIZE;

		ReadBuffer = ExAllocatePoolWithTag(PagedPool,
										   ReadBufferLength,
										   NXRMFLT_READFILE_TAG);

		if (!ReadBuffer)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		
		BytesRead = ReadBufferLength;

		Status = NXLReadSectionData(SrcInstance,
									SourceFileObject,
									NXL_SECTION_TAGS,
									ReadBuffer,
									&BytesRead);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = NXLWriteSectionData(DstInstance,
									 DestFileObject,
									 ContentKey,
									 NXL_SECTION_TAGS,
									 ReadBuffer,
									 BytesRead);


		RtlSecureZeroMemory(ReadBuffer, ReadBufferLength);

		BytesRead = ReadBufferLength;

		Status = NXLReadSectionData(SrcInstance,
									SourceFileObject,
									NXL_SECTION_TEMPLATES,
									ReadBuffer,
									&BytesRead);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = NXLWriteSectionData(DstInstance,
									 DestFileObject,
									 ContentKey,
									 NXL_SECTION_TEMPLATES,
									 ReadBuffer,
									 BytesRead);

		RtlSecureZeroMemory(ReadBuffer, ReadBufferLength);

	} while (FALSE);

	if (SourceFileHandle)
	{
		FltClose(SourceFileHandle);
		SourceFileHandle = NULL;
	}

	if (DestFileHandle)
	{
		FltClose(DestFileHandle);
		DestFileHandle = NULL;
	}

	if (SourceFileObject)
	{
		ObDereferenceObject(SourceFileObject);
	}

	if (DestFileObject)
	{
		ObDereferenceObject(DestFileObject);
	}

	if (ReadBuffer)
	{
		ExFreePoolWithTag(ReadBuffer, NXRMFLT_READFILE_TAG);
		ReadBuffer = NULL;
	}

	if (NXLHdr)
	{
		ExFreePoolWithTag(NXLHdr, NXRMFLT_TMP_TAG);
		NXLHdr = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltPurgeRightsCache(IN PFLT_INSTANCE Instance, IN ULONG FileNameHash)
{
	NTSTATUS Status = STATUS_NOT_FOUND;

	NXL_RIGHTS_CACHE_NODE *pRightsNode = NULL;

	NXL_PROCESS_NODE *pProcNode = NULL;

	LIST_ENTRY *ite = NULL;

	WCHAR *FullPathName = NULL;

	UNICODE_STRING FileName = {0};

	BOOLEAN CopyFileName = TRUE;

	do 
	{
		FullPathName = ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

		if (FullPathName)
		{
			memset(FullPathName, 0 , NXRMFLT_FULLPATH_BUFFER_SIZE);

			FileName.Buffer			= FullPathName;
			FileName.Length			= 0;
			FileName.MaximumLength	= NXRMFLT_FULLPATH_BUFFER_SIZE;
		}
		else
		{
			FileName.Buffer			= NULL;
			FileName.Length			= 0;
			FileName.MaximumLength	= 0;
		}

		FltAcquirePushLockExclusive(&Global.NxlProcessListLock);

		FOR_EACH_LIST(ite, &Global.NxlProcessList)
		{

			pProcNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

			pRightsNode = FindRightsNodeInCache(&pProcNode->RightsCache, FileNameHash);

			if (pRightsNode)
			{
				if (CopyFileName)
				{
					if (FullPathName)
					{
						RtlCopyUnicodeString(&FileName, &pRightsNode->FileName);
					}

					CopyFileName = FALSE;
				}

				rb_erase(&pRightsNode->Node, &pProcNode->RightsCache);

				memset(pRightsNode, 0, sizeof(*pRightsNode));

				ExFreeToPagedLookasideList(&Global.NXLRightsCacheLookaside, pRightsNode);

				pRightsNode = NULL;

				if (Status == STATUS_NOT_FOUND)
				{
					Status = STATUS_SUCCESS;
				}
			}
		}

		FltReleasePushLock(&Global.NxlProcessListLock);

		if (FileName.Length)
			send_purge_cache_notification(Instance, &FileName);

	} while (FALSE);

	if (FullPathName)
	{
		ExFreeToPagedLookasideList(&Global.FullPathLookaside, FullPathName);
		FullPathName = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltForceAccessCheck(
	__in PFLT_INSTANCE		Instance,
	__in PUNICODE_STRING	FileName,  
	__in ACCESS_MASK		DesiredAccess, 
	__in ULONG				FileAttributes,
	__in ULONG				ShareAccess,
	__in ULONG				CreateOptions)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_FILTER			Filter = NULL;
	HANDLE				FileHandle = NULL;
	OBJECT_ATTRIBUTES	ObjectAttribute = {0};

	IO_STATUS_BLOCK		IoStatus = { 0 };

	do 
	{
		Filter = Global.Filter;

		InitializeObjectAttributes(&ObjectAttribute,
								   FileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		Status = FltCreateFileEx2(Filter,
								  Instance,
								  &FileHandle,
								  NULL,
								  DesiredAccess,
								  &ObjectAttribute,
								  &IoStatus,
								  NULL,
								  FileAttributes,
								  ShareAccess ,
								  FILE_OPEN,
								  CreateOptions,
								  NULL,
								  0,
								  IO_FORCE_ACCESS_CHECK,
								  NULL);

	} while (FALSE);

	if (FileHandle)
	{
		FltClose(FileHandle);
		FileHandle = NULL;
	}

	return Status;
}

static NTSTATUS send_purge_cache_notification(PFLT_INSTANCE Instance, PUNICODE_STRING FileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_NOTIFICATION *pNotification = NULL;

	LARGE_INTEGER MsgTimeout = { 0 };
	const LONGLONG _1ms = 10000;  

	ULONG ReplyLength = 0;

	NXRMFLT_INSTANCE_CONTEXT *InstCtx = NULL;

	do 
	{
		if (!Global.ClientPort)
		{
			break;
		}

		pNotification = ExAllocateFromPagedLookasideList(&Global.NotificationLookaside);

		if(!pNotification)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(pNotification, 0, sizeof(*pNotification));

		pNotification->Type	= NXRMFLT_MSG_TYPE_PURGE_CACHE_NOTIFICATION;

		memcpy(pNotification->PurgeCacheMsg.FileName,
			   FileName->Buffer,
			   min(sizeof(pNotification->PurgeCacheMsg.FileName) - sizeof(WCHAR), FileName->Length));

		Status = FltGetInstanceContext(Instance, &InstCtx);

		if (NT_SUCCESS(Status))
		{
			if (InstCtx->VolDosName.Length && 
				InstCtx->VolumeProperties->RealDeviceName.Length && 
				InstCtx->VolumeProperties->RealDeviceName.Length > InstCtx->VolDosName.Length)
			{
				if (0 == memcmp(InstCtx->VolumeProperties->RealDeviceName.Buffer,
								FileName->Buffer,
								min(InstCtx->VolumeProperties->RealDeviceName.Length, FileName->Length)))
				{
					memset(pNotification->PurgeCacheMsg.FileName, 0, sizeof(pNotification->PurgeCacheMsg.FileName));

					memcpy(pNotification->PurgeCacheMsg.FileName, 
						   InstCtx->VolDosName.Buffer,
						   InstCtx->VolDosName.Length);

					memcpy((UCHAR*)pNotification->PurgeCacheMsg.FileName + InstCtx->VolDosName.Length,
						   (UCHAR*)FileName->Buffer + InstCtx->VolumeProperties->RealDeviceName.Length,
						   FileName->Length - InstCtx->VolumeProperties->RealDeviceName.Length);
				}
			}
		}

		MsgTimeout.QuadPart = NXRMFLT_MSG_TIMEOUT_IN_MS;

		MsgTimeout.QuadPart = -(MsgTimeout.QuadPart * _1ms);

		Status = FltSendMessage(Global.Filter,
								&Global.ClientPort,
								pNotification,
								sizeof(NXRMFLT_NOTIFICATION),
								NULL,
								&ReplyLength,
								&MsgTimeout);

		if (Status == STATUS_SUCCESS)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!send_purge_cache_notification: Successfully sent a purge cache notification message to user mode\n"));
		}
		else if (Status == STATUS_TIMEOUT)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!send_purge_cache_notification: Timeout while sending a purge cache notification message to user mode\n"));
		}
		else
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!send_purge_cache_notification: Sending a purge cache notification message to user mode and received an error. Status is %x\n", Status));
		}

		ExFreeToPagedLookasideList(&Global.NotificationLookaside, pNotification);

		pNotification = NULL;

	} while (FALSE);

	if (InstCtx)
	{
		FltReleaseContext(InstCtx);
	}

	return Status;
}

NTSTATUS nxrmfltCheckHideNXLExtsionByProcessId(
	__in HANDLE				ProcessId,
	__inout ULONG			*HideExt
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	LIST_ENTRY *ite = NULL;

	NXL_PROCESS_NODE *pProcessNode = NULL;
	NXL_PROCESS_NODE *pTmpNode = NULL;

	do 
	{
		if (ProcessId == Global.PortProcessId)
		{
			*HideExt = 1; 
			break;
		}

		FltAcquirePushLockShared(&Global.NxlProcessListLock);

		FOR_EACH_LIST(ite, &Global.NxlProcessList)
		{
			pProcessNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

			if (pProcessNode->ProcessId == ProcessId)
			{
				if (!ExAcquireRundownProtection(&pProcessNode->NodeRundownRef))
				{
					pProcessNode = NULL;
				}

				break;
			}
			else
			{
				pProcessNode = NULL;
			}
		}

		FltReleasePushLock(&Global.NxlProcessListLock);


		if (pProcessNode)
		{
			if (pProcessNode->HideXNLExtension)
			{
				*HideExt = 1;
			}
			else
			{
				*HideExt = 0;
			}
		}
		else
		{
			//
			// in case process created before nxrmflt start
			// this will happen if user stop nxrmflt, start process, start nxrmflt
			//
			pProcessNode = ExAllocateFromPagedLookasideList(&Global.NXLProcessCacheLookaside);

			if(pProcessNode)
			{
				pProcessNode->AlwaysDenyAccess		= FALSE;
				pProcessNode->AlwaysGrantAcess		= FALSE;
				pProcessNode->ProcessId				= ProcessId;
				pProcessNode->SessionId				= PsGetCurrentProcessSessionId();
				pProcessNode->RightsCache.rb_node	= NULL;
				pProcessNode->HideXNLExtension		= is_app_in_real_name_access_list(PsGetCurrentProcess()) ? FALSE : TRUE;
				pProcessNode->HasGraphicIntegration	= is_app_in_graphic_integration_list(PsGetCurrentProcess());

				FltInitializePushLock(&pProcessNode->RightsCacheLock);

				ExInitializeRundownProtection(&pProcessNode->NodeRundownRef);

				//
				// search the list again because we released lock
				//
				FltAcquirePushLockExclusive(&Global.NxlProcessListLock);

				FOR_EACH_LIST(ite, &Global.NxlProcessList)
				{
					pTmpNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

					if(pTmpNode->ProcessId == ProcessId)
					{
						break;
					}
					else
					{
						pTmpNode = NULL;
					}
				}

				if(!pTmpNode)
				{
					InsertHeadList(&Global.NxlProcessList, &pProcessNode->Link);

					if (!ExAcquireRundownProtection(&pProcessNode->NodeRundownRef))
					{
						pProcessNode = NULL;
					}
				}
				else
				{
					if (!ExAcquireRundownProtection(&pTmpNode->NodeRundownRef))
					{
						pTmpNode = NULL;
					}
				}

				FltReleasePushLock(&Global.NxlProcessListLock);

				//
				// there is a same record in the list already
				if(pTmpNode)
				{
					nxrmfltFreeProcessNode(pProcessNode);

					pProcessNode = pTmpNode;
				}
			}

			if (pProcessNode)
			{
				if (pProcessNode->HideXNLExtension)
				{
					*HideExt = 1;
				}
				else
				{
					*HideExt = 0;
				}
	
			}
			else
			{
				//
				// out of memory
				//
				if (is_app_in_real_name_access_list(PsGetCurrentProcess()))
				{
					*HideExt = 0;
				}
				else
				{
					*HideExt = 1;
				}
			}
		}

	} while(FALSE);

	if (pProcessNode)
	{
		ExReleaseRundownProtection(&pProcessNode->NodeRundownRef);
	}

	return Status;
}

NTSTATUS nxrmfltScanNotifyChangeDirectorySafe(
	__in PUNICODE_STRING			DirName, 
	__in PFILE_NOTIFY_INFORMATION	NotifyInfo, 
	__in ULONG						Length,
	__inout ULONG					*ContentDirty
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	FILE_NOTIFY_INFORMATION *p = NULL;

	WCHAR *FullPathBuffer = NULL;

	UNICODE_STRING FileNameWithOutPath = {0};
	UNICODE_STRING FileName = {0};

	NXL_CACHE_NODE *pCacheNode = NULL;

	do 
	{
		FullPathBuffer = ExAllocateFromPagedLookasideList(&Global.FullPathLookaside);

		if (!FullPathBuffer)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		p = NotifyInfo;

		//
		// NotifyInfo is user mode address. __try/__except to be safe
		//
		__try
		{
			do
			{
				do 
				{
					if (p->Action != FILE_ACTION_REMOVED)
					{
						break;				
					}

					FileNameWithOutPath.Buffer			= p->FileName;
					FileNameWithOutPath.MaximumLength	= (USHORT)p->FileNameLength;
					FileNameWithOutPath.Length			= (USHORT)p->FileNameLength;

					memset(FullPathBuffer, 0, NXRMFLT_FULLPATH_BUFFER_SIZE);

					FileName.Buffer			= FullPathBuffer;
					FileName.Length			= 0;
					FileName.MaximumLength	= NXRMFLT_FULLPATH_BUFFER_SIZE;

					RtlUnicodeStringCat(&FileName, DirName);

					if (DirName->Length != 0 && DirName->Buffer[DirName->Length / sizeof(WCHAR) - 1] != L'\\')
						RtlUnicodeStringCatString(&FileName, L"\\");

					RtlUnicodeStringCat(&FileName, &FileNameWithOutPath);

					FltAcquirePushLockShared(&Global.NxlFileCacheLock);

					pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &FileName);

					if (pCacheNode)
					{
						if (!FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED) &&
							!FlagOn(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX))
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltScanNotifyChangeDirectorySafe: Removing %wZ delete notify\n", &FileName));
							p->Action = FILE_ACTION_MODIFIED;
							*ContentDirty = 1;
						}
						else
						{
							PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltScanNotifyChangeDirectorySafe: Found %wZ delete notify but Flag is %x\n", &FileName, pCacheNode->Flags));
						}
					}

					FltReleasePushLock(&Global.NxlFileCacheLock);

				} while (FALSE);

				if (!p->NextEntryOffset)
				{
					break;
				}

				p = (FILE_NOTIFY_INFORMATION *)((ULONG_PTR)p + p->NextEntryOffset);

			} while((ULONG_PTR)p < (ULONG_PTR)NotifyInfo + Length);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			Status = STATUS_ACCESS_VIOLATION;
			break;
		}
		
	} while (FALSE);

	if (FullPathBuffer)
	{
		ExFreeToPagedLookasideList(&Global.FullPathLookaside, FullPathBuffer);
		FullPathBuffer = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltDuplicateNXLFileAndItsRecords(
	__in ULONG						RequestorSessionId,
	__in PUNICODE_STRING			SrcFileName,
	__in PUNICODE_STRING			DstFileName,
	__in PFLT_INSTANCE				DstInstance,
	__in PUNICODE_STRING			DstDirName
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_STREAM_CONTEXT *Ctx = NULL;

	NXL_CACHE_NODE *pCacheNode = NULL;
	NXL_CACHE_NODE *pSrcCacheNode = NULL;

	ULONG DirHash = 0;

	LARGE_INTEGER FileId = {0};
	ULONG FileAttributes = 0;

	NXL_KEKEY_BLOB	PrimaryKey = {0};

	FILE_OBJECT *FileObject = NULL;
	HANDLE		FileHandle = NULL;

	OBJECT_ATTRIBUTES	ObjectAttribute = {0};
	IO_STATUS_BLOCK		IoStatus = { 0 };

	BOOLEAN SrcFileOnRemoteOrRemovableMedia = FALSE;

	do 
	{
		RtlHashUnicodeString(DstDirName, TRUE, HASH_STRING_ALGORITHM_X65599, &DirHash);

		//
		// Calling this function before acquiring the lock because this function generates I/O.
		//
		get_file_id_and_attribute(DstInstance, DstFileName, &FileId, &FileAttributes);


		//
		// find out if source file is on remote or removable media
		//
		FltAcquirePushLockShared(&Global.NxlFileCacheLock);

		pSrcCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, SrcFileName);

		if (pSrcCacheNode)
		{
			SrcFileOnRemoteOrRemovableMedia = pSrcCacheNode->OnRemoveOrRemovableMedia;
			pSrcCacheNode = NULL;
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

		FltAcquirePushLockExclusive(&Global.NxlFileCacheLock);

		do
		{
			pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, DstFileName);

			if (pCacheNode)
			{
				//
				// This is NOT expected
				//
				pCacheNode = NULL;
				Status = STATUS_INVALID_PARAMETER_3;
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
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			memset(pCacheNode, 0, sizeof(NXL_CACHE_NODE));

			Status = build_nxlcache_file_name(pCacheNode, DstFileName);

			if (!NT_SUCCESS(Status))
			{
				//
				// ERROR case. No memory
				//
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", DstFileName));

				ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pCacheNode);
				pCacheNode = NULL;
				break;
			}

			pCacheNode->FileAttributes = FileAttributes;
			pCacheNode->FileID.QuadPart = FileId.QuadPart;
			pCacheNode->Flags |= FlagOn(FileAttributes, FILE_ATTRIBUTE_READONLY) ? NXRMFLT_FLAG_READ_ONLY : 0;
			pCacheNode->Flags |= NXRMFLT_FLAG_ATTACHING_CTX;
			pCacheNode->Instance = DstInstance;
			pCacheNode->ParentDirectoryHash = DirHash;
			pCacheNode->OnRemoveOrRemovableMedia = SrcFileOnRemoteOrRemovableMedia;

			ExInitializeRundownProtection(&pCacheNode->NodeRundownRef);

			Status = build_nxlcache_reparse_file_name(pCacheNode, DstFileName);

			if (!NT_SUCCESS(Status))
			{
				//
				// ERROR case. No memory
				//
				PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't allocate memory to build cache for %wZ\n", DstFileName));

				if (pCacheNode->ReleaseFileName)
				{
					ExFreePoolWithTag(pCacheNode->FileName.Buffer, NXRMFLT_NXLCACHE_TAG);
					RtlInitUnicodeString(&pCacheNode->FileName, NULL);
				}

				ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pCacheNode);
				pCacheNode = NULL;
				break;
			}

			PT_DBG_PRINT(PTDBG_TRACE_CACHE_NODE, ("nxrmflt!AddNXLNodeToCache add file %wZ into cache\n", DstFileName));

			AddNXLNodeToCache(&Global.NxlFileCache, pCacheNode);

			if (!ExAcquireRundownProtection(&pCacheNode->NodeRundownRef))
			{
				pCacheNode = NULL;
			}

		} while (FALSE);

		FltReleasePushLock(&Global.NxlFileCacheLock);

		if (!pCacheNode)
		{
			break;
		}

		FltAcquirePushLockShared(&Global.PrimaryKeyLock);

		memcpy(&PrimaryKey, 
			   &Global.PrimaryKey, 
			   min(sizeof(PrimaryKey),sizeof(Global.PrimaryKey)));

		FltReleasePushLock(&Global.PrimaryKeyLock);

		Status = NXLCreateFile(Global.Filter,
							   DstInstance,
							   &pCacheNode->OriginalFileName,
							   &PrimaryKey,
							   NULL,
							   TRUE);

		if (!NT_SUCCESS(Status))
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!Can't create NXL file %wZ\n", &pCacheNode->OriginalFileName));
		}

		FltAcquirePushLockShared(&Global.NxlFileCacheLock);

		pSrcCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, SrcFileName);

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

		//
		// Build and attach Ctx
		//
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

		Status = nxrmfltBuildNamesInStreamContext(Ctx, DstFileName);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		Ctx->OriginalInstance	= DstInstance;
		Ctx->RequestorSessionId = RequestorSessionId;

		FltInitializePushLock(&Ctx->CtxLock);

		//
		// force sync when close
		//
		Ctx->ContentDirty = TRUE;

		InitializeObjectAttributes(&ObjectAttribute,
								   DstFileName,
								   OBJ_KERNEL_HANDLE,
								   NULL,
								   NULL);

		Status = FltCreateFileEx2(Global.Filter,
								  DstInstance,
								  &FileHandle,
								  &FileObject,
								  GENERIC_READ | SYNCHRONIZE,
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

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = FltSetStreamContext(DstInstance,
									 FileObject,
									 FLT_SET_CONTEXT_KEEP_IF_EXISTS,
									 Ctx,
									 NULL);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		InterlockedIncrement(&Global.TotalContext);

		FltAcquirePushLockShared(&Global.NxlFileCacheLock);

		pCacheNode = FindNXLNodeInCache(&Global.NxlFileCache, &Ctx->FileName);

		if(pCacheNode)
		{
			SetFlag(pCacheNode->Flags, NXRMFLT_FLAG_CTX_ATTACHED);
			ClearFlag(pCacheNode->Flags, NXRMFLT_FLAG_ATTACHING_CTX);
		}

		FltReleasePushLock(&Global.NxlFileCacheLock);

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

	if (Ctx)
	{
		FltReleaseContext(Ctx);
	}

	return Status;
}

NTSTATUS nxrmfltBuildAdobeRenameNode(
	__in PUNICODE_STRING			SrcFileName,
	__in PUNICODE_STRING			DstFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	ADOBE_RENAME_NODE *pAdobeRenameNode = NULL;

	ULONG SrcFileNameLengthInBytes = 0;
	ULONG DstFileNameLengthInBytes = 0;

	LIST_ENTRY *ite = NULL;

	do 
	{
		pAdobeRenameNode = ExAllocateFromPagedLookasideList(&Global.AdobeRenameLookaside);

		if (!pAdobeRenameNode)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(pAdobeRenameNode, 0, sizeof(*pAdobeRenameNode));

		ExInitializeRundownProtection(&pAdobeRenameNode->NodeRundownRef);

		RtlInitUnicodeString(&pAdobeRenameNode->SourceFileName, NULL);
		RtlInitUnicodeString(&pAdobeRenameNode->DestinationFileName, NULL);

		SrcFileNameLengthInBytes = SrcFileName->Length;
		DstFileNameLengthInBytes = DstFileName->Length;

		if(SrcFileNameLengthInBytes > sizeof(pAdobeRenameNode->SourceFileNameFastBuffer))
		{
			pAdobeRenameNode->SourceFileName.Buffer = ExAllocatePoolWithTag(PagedPool, 
																			SrcFileNameLengthInBytes, 
																			NXRMFLT_ADOBERENAME_NAME_TAG);

			if(!pAdobeRenameNode->SourceFileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pAdobeRenameNode->ReleaseSourceFileName = TRUE;

			memset(pAdobeRenameNode->SourceFileName.Buffer, 
				   0, 
				   SrcFileNameLengthInBytes);

			pAdobeRenameNode->SourceFileName.MaximumLength = (USHORT)SrcFileNameLengthInBytes;
			pAdobeRenameNode->SourceFileName.Length		   = 0;
		}
		else
		{
			pAdobeRenameNode->ReleaseSourceFileName = FALSE;

			pAdobeRenameNode->SourceFileName.Buffer			= pAdobeRenameNode->SourceFileNameFastBuffer;
			pAdobeRenameNode->SourceFileName.MaximumLength	= sizeof(pAdobeRenameNode->SourceFileNameFastBuffer);
			pAdobeRenameNode->SourceFileName.Length			= 0;
		}

		if(DstFileNameLengthInBytes > sizeof(pAdobeRenameNode->DestinationFileNameFastBuffer))
		{
			pAdobeRenameNode->DestinationFileName.Buffer = ExAllocatePoolWithTag(PagedPool, 
																				 DstFileNameLengthInBytes, 
																				 NXRMFLT_ADOBERENAME_NAME_TAG);

			if(!pAdobeRenameNode->DestinationFileName.Buffer)
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			pAdobeRenameNode->ReleaseDestinationFileName = TRUE;

			memset(pAdobeRenameNode->DestinationFileName.Buffer, 
				   0, 
				   DstFileNameLengthInBytes);

			pAdobeRenameNode->DestinationFileName.MaximumLength = (USHORT)DstFileNameLengthInBytes;
			pAdobeRenameNode->DestinationFileName.Length		= 0;
		}
		else
		{
			pAdobeRenameNode->ReleaseDestinationFileName = FALSE;

			pAdobeRenameNode->DestinationFileName.Buffer			= pAdobeRenameNode->DestinationFileNameFastBuffer;
			pAdobeRenameNode->DestinationFileName.MaximumLength		= sizeof(pAdobeRenameNode->DestinationFileNameFastBuffer);
			pAdobeRenameNode->DestinationFileName.Length			= 0;
		}

		Status = RtlUnicodeStringCat(&pAdobeRenameNode->SourceFileName, SrcFileName);

		if (!NT_SUCCESS(Status))
		{
			break;
		}
		
		Status = RtlUnicodeStringCat(&pAdobeRenameNode->DestinationFileName, DstFileName);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		FltAcquirePushLockExclusive(&Global.AdobeRenameExpireTableLock);

		FOR_EACH_LIST(ite, &Global.AdobeRenameExpireTable)
		{
			ADOBE_RENAME_NODE *pNode = CONTAINING_RECORD(ite, ADOBE_RENAME_NODE, Link);

			if (RtlEqualUnicodeString(&pNode->SourceFileName, &pAdobeRenameNode->SourceFileName, TRUE) &&
				RtlEqualUnicodeString(&pNode->DestinationFileName, &pAdobeRenameNode->DestinationFileName, TRUE))
			{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
		}

		if (NT_SUCCESS(Status))
		{
			InsertHeadList(&Global.AdobeRenameExpireTable, &pAdobeRenameNode->Link);
		}

		FltReleasePushLock(&Global.AdobeRenameExpireTableLock);

	} while (FALSE);

	if (!NT_SUCCESS(Status) && pAdobeRenameNode)
	{
		nxrmfltFreeAdobeRenameNode(pAdobeRenameNode);
		pAdobeRenameNode = NULL;
	}

	return Status;
}

void nxrmfltFreeAdobeRenameNode(__in PADOBE_RENAME_NODE	pNode)
{
	//
	// Wait for all other threads rundown
	//
	ExWaitForRundownProtectionRelease(&pNode->NodeRundownRef);

	ExRundownCompleted(&pNode->NodeRundownRef);

	//
	// Free resources here
	//
	if (pNode->ReleaseSourceFileName)
	{
		ExFreePoolWithTag(pNode->SourceFileName.Buffer, NXRMFLT_ADOBERENAME_NAME_TAG);

	}

	RtlInitUnicodeString(&pNode->SourceFileName, NULL);

	if (pNode->ReleaseDestinationFileName)
	{
		ExFreePoolWithTag(pNode->DestinationFileName.Buffer, NXRMFLT_ADOBERENAME_NAME_TAG);

	}

	RtlInitUnicodeString(&pNode->DestinationFileName, NULL);

	//
	// Free to look aside list
	//
	ExFreeToPagedLookasideList(&Global.AdobeRenameLookaside, pNode);

	return;
}

NTSTATUS nxrmfltBlockPreCreate(
	__inout PFLT_CALLBACK_DATA			Data,
	__in	PUNICODE_STRING				ReparseFileName,
	__in	PFLT_FILE_NAME_INFORMATION	NameInfo
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	do
	{

		Status = IoReplaceFileObjectName(Data->Iopb->TargetFileObject,
										 ReparseFileName->Buffer,
										 ReparseFileName->Length);
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Status = nxrmfltAddBlockingECP(Data, NameInfo);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		Data->IoStatus.Status		= STATUS_REPARSE;
		Data->IoStatus.Information	= IO_REPARSE;

	} while (FALSE);

	return Status;
}

PFLT_INSTANCE nxrmfltFindInstanceByFileName(__in UNICODE_STRING *FileName)
{
	PFLT_INSTANCE Instance = NULL;

	LIST_ENTRY *ite = NULL;

	FltAcquirePushLockShared(&Global.AttachedInstancesListLock);

	do 
	{
		FOR_EACH_LIST(ite, &Global.AttachedInstancesList)
		{
			NXRMFLT_INSTANCE_CONTEXT *InstCtx = CONTAINING_RECORD(ite, NXRMFLT_INSTANCE_CONTEXT, Link);

			if (RtlPrefixUnicodeString(&InstCtx->VolumeProperties->RealDeviceName, FileName, TRUE))
			{
				Instance = InstCtx->Instance;
				break;
			}
		}

	} while (FALSE);

	FltReleasePushLock(&Global.AttachedInstancesListLock);

	return Instance;
}

LONG nxrmfltCompareFinalComponent(
	__in UNICODE_STRING *FileName1,
	__in UNICODE_STRING *FileName2,
	__in BOOLEAN		CaseInSensitive)
{
	UNICODE_STRING FinalComponent1 = { 0 };
	UNICODE_STRING FinalComponent2 = { 0 };

	FindFinalComponent(FileName1, &FinalComponent1);
	FindFinalComponent(FileName2, &FinalComponent2);

	return RtlCompareUnicodeString(&FinalComponent1, &FinalComponent2, CaseInSensitive);
}

void FindFinalComponent(
	__in UNICODE_STRING *FileName, 
	__in UNICODE_STRING *FinalComponent)
{
	WCHAR *p = NULL;

	USHORT l = 0;

	do 
	{
		p = (WCHAR*)((UCHAR*)FileName->Buffer + FileName->Length);

		while (p != FileName->Buffer)
		{
			if (*p == L'\\')
			{
				break;
			}

			p--;
			l+= sizeof(WCHAR);
		}

		FinalComponent->Buffer = (p == (FileName->Buffer + FileName->Length / sizeof(WCHAR))) ? NULL : (p + 1);
		FinalComponent->Length = l;
		FinalComponent->MaximumLength = l;

	} while (FALSE);

	return;
}

NTSTATUS nxrmfltSendProcessNotification(
	__in HANDLE		ProcessId,
	__in ULONG		SessionId,
	__in BOOLEAN	Create,
	__in ULONGLONG	Flags,
	__in_opt PCUNICODE_STRING ImageFileName)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PFLT_GENERIC_WORKITEM FltWorkItem = NULL;

	NXRMFLT_NOTIFICATION *pNotification = NULL;


	do
	{
		if (!Global.ClientPort)
		{
			break;
		}

		FltWorkItem = FltAllocateGenericWorkItem();

		if (!FltWorkItem)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pNotification = ExAllocateFromPagedLookasideList(&Global.NotificationLookaside);

		if (!pNotification)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memset(pNotification, 0, sizeof(*pNotification));

		pNotification->Type = NXRMFLT_MSG_TYPE_PROCESS_NOTIFICATION;

		pNotification->ProcessMsg.SessionId = SessionId;
		pNotification->ProcessMsg.ProcessId = (ULONG)(ULONG_PTR)ProcessId;
		pNotification->ProcessMsg.Create = Create ? 1 : 0;
		pNotification->ProcessMsg.Flags = Flags;

		if (ImageFileName)
		{
			memcpy(pNotification->ProcessMsg.ProcessImagePath,
				   ImageFileName->Buffer,
				   min(sizeof(pNotification->ProcessMsg.ProcessImagePath) - sizeof(WCHAR), ImageFileName->Length));
		}

		Status = FltQueueGenericWorkItem(FltWorkItem,
										 Global.Filter,
										 nxrmfltSendProcessNotificationWorkProc,
										 DelayedWorkQueue,
										 pNotification);

	} while (FALSE);

	if (!NT_SUCCESS(Status))
	{
		if (pNotification)
		{
			ExFreeToPagedLookasideList(&Global.NotificationLookaside, pNotification);
			pNotification = NULL;
		}

		if (FltWorkItem)
			FltFreeGenericWorkItem(FltWorkItem);
	}

	return Status;
}

VOID NTAPI nxrmfltSendProcessNotificationWorkProc(
	__in PFLT_GENERIC_WORKITEM	FltWorkItem,
	__in PVOID					FltObject,
	__in_opt PVOID				Context)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_NOTIFICATION *pNotification = (NXRMFLT_NOTIFICATION*)Context;

	LARGE_INTEGER MsgTimeout = { 0 };
	const LONGLONG _1ms = 10000;

	ULONG ReplyLength = 0;

	FltFreeGenericWorkItem(FltWorkItem);

	if (pNotification)
	{
		MsgTimeout.QuadPart = NXRMFLT_MSG_TIMEOUT_IN_MS;

		MsgTimeout.QuadPart = -(MsgTimeout.QuadPart * _1ms);

		Status = FltSendMessage(Global.Filter,
								&Global.ClientPort,
								pNotification,
								sizeof(NXRMFLT_NOTIFICATION),
								NULL,
								&ReplyLength,
								&MsgTimeout);

		if (Status == STATUS_SUCCESS)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendProcessNotificationWorkProc: Successfully sent a process notification message to user mode\n"));
		}
		else if (Status == STATUS_TIMEOUT)
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendProcessNotificationWorkProc: Timeout while sending a process notification message to user mode\n"));
		}
		else
		{
			PT_DBG_PRINT(PTDBG_TRACE_CRITICAL, ("nxrmflt!nxrmfltSendProcessNotificationWorkProc: Sending a process notification message to user mode and received an error. Status is %x\n", Status));
		}

		ExFreeToPagedLookasideList(&Global.NotificationLookaside, pNotification);

		pNotification = NULL;
	}

	return;
}
