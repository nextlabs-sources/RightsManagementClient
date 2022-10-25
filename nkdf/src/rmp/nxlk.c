
#include <ntifs.h>

#include <nkdf/error.h>
#include <nkdf/basic.h>
#include <nkdf/crypto.h>
#include <nkdf/fs.h>

#include <nkdf/nxlk.h>



_Check_return_
NTSTATUS
SetPrimaryKey(
			  _In_ PCNXL_KEKEY_BLOB PrimaryKey,
			  _In_reads_(32) const UCHAR* AesKey,
			  _Out_ PNXL_HEADER Header
			  );

_Check_return_
NTSTATUS
SetRecoveryKey(
			   _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
			   _In_reads_(32) const UCHAR* AesKey,
			   _Out_ PNXL_HEADER Header
			   );

_Check_return_
NTSTATUS
FillDefaultHeader(
				  _In_ PCNXL_KEKEY_BLOB PrimaryKey,
				  _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
				  _In_reads_(32) const UCHAR* AesKey,
				  _Out_ PNXL_HEADER Header
				  );

_Check_return_
NTSTATUS
DecryptContentKey(
                  _In_ PCNXL_HEADER Header,
				  _In_ PCNXL_KEKEY_BLOB KeKey,
			      _Out_writes_bytes_(32) UCHAR* CeKey
                  );

_Check_return_
NTSTATUS
DecryptSectionTableChecksum(
                            _In_ PCNXL_HEADER Header,
			                _In_reads_(16) const UCHAR* CeKey,
                            _Out_ PULONG Checksum
                            );

_Check_return_
ULONG
CalcSectionTableChecksum(
						 _In_ PCNXL_HEADER Header
						 );

_Check_return_
NTSTATUS
SetSectionTableChecksum(
						_In_reads_(16) const UCHAR* AesKey,
						_Inout_ PNXL_HEADER Header
						);

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NXLCheck)
#pragma alloc_text(PAGE, NXLCheckEx)
#pragma alloc_text(PAGE, NXLCheck2)
#pragma alloc_text(PAGE, NXLCheckEx2)
#pragma alloc_text(PAGE, NXLValidate)
#pragma alloc_text(PAGE, NXLValidateEx)
#pragma alloc_text(PAGE, NXLCreateFile)
#pragma alloc_text(PAGE, NXLCreateEmptyFile)
#pragma alloc_text(PAGE, NXLReadHeader)
#pragma alloc_text(PAGE, NXLWriteHeader)
#pragma alloc_text(PAGE, NXLGetContentKey)
#pragma alloc_text(PAGE, NXLReadData)
#pragma alloc_text(PAGE, NXLWriteData)
#pragma alloc_text(PAGE, NXLGetFileSize)
#pragma alloc_text(PAGE, NXLSetFileSize)
#pragma alloc_text(PAGE, NXLReadSectionData)
#pragma alloc_text(PAGE, NXLWriteSectionData)
#pragma alloc_text(PAGE, NXLGetSectionBlock)
#pragma alloc_text(PAGE, NXLSetSectionBlock)
#pragma alloc_text(PAGE, NXLSetSectionBlockEx)
#endif




_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheck(
		 _In_ PFLT_INSTANCE Instance,
		 _In_ PFILE_OBJECT FileObject,
		 _Out_ PBOOLEAN Result
		 )
{
	PAGED_CODE();
    return NXLCheck2(Instance, FileObject, NULL, Result);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheck2(
		  _In_ PFLT_INSTANCE Instance,
		  _In_ PFILE_OBJECT FileObject,
          _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey,
		  _Out_ PBOOLEAN Result
		  )
{
	NTSTATUS        Status;
	LONG            ValidateResult = 0;

	PAGED_CODE();

	*Result = FALSE;
	Status = NXLValidateEx(Instance, FileObject, PrimaryKey, &ValidateResult);
	if (NT_SUCCESS(Status)) {
		*Result = (0 == ValidateResult) ? TRUE : FALSE;
	}
	return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheckEx(
		   _In_  PFLT_FILTER Filter,
		   _In_ PFLT_INSTANCE Instance,
		   _In_ PCUNICODE_STRING FileName,
		   _Out_ PBOOLEAN Result
		   )
{
	PAGED_CODE();
    return NXLCheckEx2(Filter, Instance, FileName, NULL, Result);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCheckEx2(
		    _In_  PFLT_FILTER Filter,
		    _In_ PFLT_INSTANCE Instance,
		    _In_ PCUNICODE_STRING FileName,
            _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey,
		    _Out_ PBOOLEAN Result
		    )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	HANDLE          FileHandle = NULL;
	PFILE_OBJECT    FileObject = NULL;

	PAGED_CODE();

	__try {

		*Result = FALSE;

		Status = NkFltOpenFile(Filter,
							   Instance,
							   &FileHandle,
							   GENERIC_READ,
							   FileName,
							   NULL,
							   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							   FILE_NON_DIRECTORY_FILE,
							   IO_IGNORE_SHARE_ACCESS_CHECK);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = ObReferenceObjectByHandle(FileHandle, GENERIC_READ, *IoFileObjectType, KernelMode, &FileObject, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = NXLCheck2(Instance, FileObject, PrimaryKey, Result);

try_exit: NOTHING;
	}
	__finally {
		if (NULL != FileObject) {
			ObDereferenceObject(FileObject);
		}
		if (NULL != FileHandle) {
			NkFltCloseFile(FileHandle);
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLValidate(
			_In_ PFLT_INSTANCE Instance,
			_In_ PFILE_OBJECT FileObject,
			_Out_ LONG* Result
			)
{
	PAGED_CODE();
    return NXLValidateEx(Instance, FileObject, NULL, Result);
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLValidateEx(
              _In_ PFLT_INSTANCE Instance,
              _In_ PFILE_OBJECT FileObject,
              _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey,
              _Out_ LONG* Result
              )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	LARGE_INTEGER   FileSize = { 0, 0 };
	PNXL_HEADER     Header = NULL;

	PAGED_CODE();


	*Result = NE_INVALID_TYPE;

	__try {

		Header = (PNXL_HEADER)ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}
		RtlZeroMemory(Header, sizeof(NXL_HEADER));

		Status = NkFltSyncGetFileSize(Instance, FileObject, &FileSize);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		if (0 != (FileSize.QuadPart % NXL_PAGE_SIZE)) {
			*Result = NE_NXL_INVALID_FILE_SIZE;
			try_return(Status = STATUS_SUCCESS);
		}

#pragma prefast(suppress: 6386, "somehow prefast report a nonsense warning here")
		Status = NXLReadHeader(Instance, FileObject, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		
#pragma prefast(suppress: 6385, "somehow prefast report a nonsense warning here")
		*Result = NXLValidateHeader(Header);
		Status = STATUS_SUCCESS;

try_exit: NOTHING;
	}
	__finally {
		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONG
NXLValidateHeader(
				  _In_ PCNXL_HEADER Header
				  )
{
    return NXLValidateHeaderEx(Header, NULL);
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
LONG
NXLValidateHeaderEx(
				    _In_ PCNXL_HEADER Header,
                    _In_opt_ PCNXL_KEKEY_BLOB PrimaryKey
				    )
{
	static const SIGNATURE_CODE DesiredCode = { NXL_SIGNATURE_LOW, NXL_SIGNATURE_HIGH };

	//*****************************************
	//  Quick Check
	//*****************************************

	// check signature
	if (DesiredCode.QuadPart != Header->Signature.Code.QuadPart) {
		return NE_NXL_INVALID_SIGNATURE;
	}

	//*****************************************
	//  Full Check
	//*****************************************

	// check version
	if (NXL_VERSION_10 != Header->Basic.Version) {
		return NE_NXL_INVALID_VERSION;
	}

	// check alignment
	if (NXL_PAGE_SIZE != Header->Basic.Alignment) {
		return NE_NXL_INVALID_ALIGNMENT;
	}

	// check data offset
	if (Header->Basic.PointerOfContent < NXL_MIN_SIZE || 0 != (Header->Basic.PointerOfContent % NXL_PAGE_SIZE)) {
		return NE_NXL_INVALID_CONTENT_OFFSET;
	}

	// Check Thumbprint
	if (NkIsMemoryZero(Header->Basic.Thumbprint, 16)) {
		return NE_NXL_INVALID_THUMBPRINT;
	}

	// Check Algorithm
	if (NXL_ALGORITHM_AES128 != Header->Crypto.Algorithm && NXL_ALGORITHM_AES256 != Header->Crypto.Algorithm) {
		return NE_NXL_INVALID_ENCRYPT_ALGORITHM;
	}

	// check cbc size
	if (Header->Crypto.CbcSize != NXL_CBC_SIZE) {
		return NE_NXL_INVALID_CBC_SIZE;
	}

	// Check KEKs

	// a. Primary KEK
	if (Header->Crypto.PrimaryKey.KeKeyId.Algorithm != NXL_ALGORITHM_AES256) {
		return NE_NXL_INVALID_KEY_ALGORITHM;
	}

	if (NXL_ALGORITHM_NONE != Header->Crypto.RecoveryKey.KeKeyId.Algorithm) {
		if (NXL_ALGORITHM_RSA1024 != Header->Crypto.RecoveryKey.KeKeyId.Algorithm && NXL_ALGORITHM_RSA2048 != Header->Crypto.RecoveryKey.KeKeyId.Algorithm) {
			return NE_NXL_INVALID_KEY_ALGORITHM;
		}
		if (20 != Header->Crypto.RecoveryKey.KeKeyId.IdSize) {  // SHA1 Thumbprint
			return NE_NXL_INVALID_KEYID_SIZE;
		}
	}
    
	// Check section count
	if (Header->Sections.Count < 3) {
		return NE_NXL_TOO_FEW_SECTIONS;
	}

	// Check section ".Attrs"
	if (0 != NkCompareStringA(Header->Sections.Sections[0].Name, NXL_SECTION_ATTRIBUTES, TRUE)) {
		return NE_NXL_DEFAULT_SECTION_NOT_EXIST;
	}
	if (0 == Header->Sections.Sections[0].Size || (2048 != Header->Sections.Sections[0].Size && 0 != (Header->Sections.Sections[0].Size%NXL_PAGE_SIZE))) {
		return NE_NXL_INVALID_SECTION_SIZE;
	}

	// Check section ".Rights"
	if (0 != NkCompareStringA(Header->Sections.Sections[1].Name, NXL_SECTION_TEMPLATES, TRUE)) {
		return NE_NXL_DEFAULT_SECTION_NOT_EXIST;
	}
	if (0 == Header->Sections.Sections[1].Size || 0 != (Header->Sections.Sections[1].Size%NXL_PAGE_SIZE)) {
		return NE_NXL_INVALID_SECTION_SIZE;
	}

	// Check section ".Tags"
	if (0 != NkCompareStringA(Header->Sections.Sections[2].Name, NXL_SECTION_TAGS, TRUE)) {
		return NE_NXL_DEFAULT_SECTION_NOT_EXIST;
	}
	if (0 == Header->Sections.Sections[2].Size || 0 != (Header->Sections.Sections[2].Size%NXL_PAGE_SIZE)) {
		return NE_NXL_INVALID_SECTION_SIZE;
	}

    if (NULL != PrimaryKey) {

        NTSTATUS Status = STATUS_SUCCESS;
        ULONG   RealChecksum  = 0;
        ULONG   ExistChecksum = 0;
        UCHAR   CeKey[32] = { 0 };

        // Get cekey
        Status = DecryptContentKey(Header, PrimaryKey, CeKey);
        if (!NT_SUCCESS(Status)) {
            return NE_NXL_KEY_NOT_EXIST;
        }

        Status = DecryptSectionTableChecksum(Header, CeKey, &ExistChecksum);
        if (!NT_SUCCESS(Status)) {
            return NE_NXL_KEY_NOT_EXIST;
        }

        //  Calculate Section Table Checksum
        RealChecksum = CalcSectionTableChecksum(Header);
        if (RealChecksum != ExistChecksum) {
            return NE_NXL_INVALID_SECTION_TABLE_CHECKSUM;
        }
    }
    
	return 0;
}

_Check_return_
_IRQL_requires_max_(DISPATCH_LEVEL)
NXL_SECTION*
NXLFindSection(
			   _In_ PNXL_HEADER Header,
			   _In_ const CHAR* Name,
			   _Out_ PULONG Offset
			   )
{
	ULONG Index = 0;
	ULONG Position = NXL_SCNDATA_OFFSET;  //FIELD_OFFSET(NXL_HEADER, Sections.Sections);

	for (Index = 0; Index < Header->Sections.Count; Index++) {
		if (0 == NkCompareStringA(Name, Header->Sections.Sections[Index].Name, TRUE)) {
			*Offset = Position;
			return &Header->Sections.Sections[Index];
		}

		Position += Header->Sections.Sections[Index].Size;
	}

	*Offset = 0;
	return NULL;
}

_Check_return_
NTSTATUS
SetPrimaryKey(
			  _In_ PCNXL_KEKEY_BLOB PrimaryKey,
			  _In_reads_(32) const UCHAR* AesKey,
			  _Out_ PNXL_HEADER Header
			  )
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE   AesObject = NULL;

	RtlZeroMemory(&Header->Crypto.PrimaryKey, sizeof(NXL_KEY_BLOB));
	RtlCopyMemory(&Header->Crypto.PrimaryKey.KeKeyId, &PrimaryKey->KeyId, sizeof(NXL_KEKEY_ID));
	RtlCopyMemory(Header->Crypto.PrimaryKey.CeKey, AesKey, 32);
	Header->Crypto.PrimaryKey.KeKeyId.Algorithm = PrimaryKey->KeyId.Algorithm;
	Status = NkCreateAesObject(PrimaryKey->Key, PrimaryKey->keySize, NXL_CBC_SIZE, &AesObject);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}
	Status = NkAesEncrypt(AesObject, 0, Header->Crypto.PrimaryKey.CeKey, 256);
	NkDestroyAesObject(AesObject);
	AesObject = NULL;
	return Status;
}

_Check_return_
NTSTATUS
SetRecoveryKey(
			   _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
			   _In_reads_(32) const UCHAR* AesKey,
			   _Out_ PNXL_HEADER Header
			   )
{
	RtlZeroMemory(&Header->Crypto.RecoveryKey, sizeof(NXL_KEY_BLOB));
	if (NULL == RecoveryKey) {
		return STATUS_SUCCESS;
	}

	return STATUS_NOT_IMPLEMENTED;
}

_Check_return_
BOOLEAN
EqualKeyId(
           _In_ PCNXL_KEKEY_ID KeyId1,
           _In_ PCNXL_KEKEY_ID KeyId2
           )
{
    if (NULL == KeyId1 || NULL == KeyId1) {
        return FALSE;
    }

    if (KeyId1->Algorithm != KeyId2->Algorithm || KeyId1->IdSize != KeyId2->IdSize || 0 == KeyId1->IdSize) {
        return FALSE;
    }

    if (KeyId1->IdSize != RtlCompareMemory(KeyId1->Id, KeyId2->Id, KeyId1->IdSize)) {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
NTSTATUS
DecryptContentKey(
                  _In_ PCNXL_HEADER Header,
				  _In_ PCNXL_KEKEY_BLOB KeKey,
			      _Out_writes_bytes_(32) UCHAR* CeKey
                  )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE AesObject = NULL;
    UCHAR CipherCeKey[32] = { 0 };

    if (NULL == Header || NULL == KeKey || NULL == CeKey) {
        return STATUS_INVALID_PARAMETER;
    }

    if (EqualKeyId(&Header->Crypto.PrimaryKey.KeKeyId, &KeKey->KeyId)) {
        RtlCopyMemory(CipherCeKey, Header->Crypto.PrimaryKey.CeKey, 32);
    }
    else if (EqualKeyId(&Header->Crypto.RecoveryKey.KeKeyId, &KeKey->KeyId)) {
        RtlCopyMemory(CipherCeKey, Header->Crypto.RecoveryKey.CeKey, 32);
    }
    else {
        return STATUS_INVALID_PARAMETER;
    }
    
    try {
        
        Status = NkCreateAesObject(KeKey->Key, KeKey->keySize, NXL_CBC_SIZE, &AesObject);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = NkAesDecrypt(AesObject, 0, CipherCeKey, 32);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlCopyMemory(CeKey, CipherCeKey, 32);

try_exit: NOTHING;
    }
    finally {

        if (NULL != AesObject) {
            NkDestroyAesObject(AesObject);
            AesObject = NULL;
        }
    }

    return Status;
}

_Check_return_
NTSTATUS
DecryptSectionTableChecksum(
                            _In_ PCNXL_HEADER Header,
			                _In_reads_(16) const UCHAR* CeKey,
                            _Out_ PULONG Checksum
                            )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE AesObject = NULL;

    if (NULL == Header || NULL == Checksum) {
        return STATUS_INVALID_PARAMETER;
    }

    *Checksum = 0;

    try {

        UCHAR ChecksumData[16] = { 0 };

        Status = NkCreateAesObject(CeKey, 16, NXL_CBC_SIZE, &AesObject);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        RtlCopyMemory(ChecksumData, Header->Sections.Checksum, 16);
        Status = NkAesDecrypt(AesObject, 0, ChecksumData, 16);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        *Checksum = *((PULONG)ChecksumData);

try_exit: NOTHING;
    }
    finally {

        if (NULL != AesObject) {
            NkDestroyAesObject(AesObject);
            AesObject = NULL;
        }
    }

    return Status;
}

_Check_return_
ULONG
CalcSectionTableChecksumEx(
						   _In_ PCNXL_SECTION_TABLE Table
						   )
{
	ULONG Checksum = 0;

	Checksum = NkCrc32(0, &Table->Count, sizeof(ULONG));
	if (0 != Table->Count) {
		Checksum = NkCrc32(Checksum, &Table->Sections[0], sizeof(NXL_SECTION) * Table->Count);
	}

	return Checksum;
}

_Check_return_
ULONG
CalcSectionTableChecksum(
						 _In_ PCNXL_HEADER Header
						 )
{
	return CalcSectionTableChecksumEx(&Header->Sections);
}

_Check_return_
NTSTATUS
SetSectionTableChecksumEx (
						   _In_reads_(16) const UCHAR* AesKey,
						   _Inout_ PNXL_SECTION_TABLE Table
						   )
{
	NTSTATUS Status    = STATUS_SUCCESS;
	ULONG    Checksum  = 0;
	HANDLE   AesObject = NULL;

	Status = NkCreateAesObject(AesKey, 16, NXL_CBC_SIZE, &AesObject);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	Checksum = CalcSectionTableChecksumEx(Table);
	RtlCopyMemory(Table->Checksum, &Checksum, sizeof(ULONG));
	Status = NkAesEncrypt(AesObject, 0, Table->Checksum, 16);
	NkDestroyAesObject(AesObject);
	AesObject = NULL;
	return Status;
}

_Check_return_
NTSTATUS
SetSectionTableChecksum(
						_In_reads_(16) const UCHAR* AesKey,
						_Inout_ PNXL_HEADER Header
						)
{
	return SetSectionTableChecksumEx(AesKey, &Header->Sections);
}

_Check_return_
NTSTATUS
FillDefaultHeader(
				  _In_ PCNXL_KEKEY_BLOB PrimaryKey,
				  _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
				  _In_reads_(32) const UCHAR* AesKey,
				  _Out_ PNXL_HEADER Header
				  )
{
	NTSTATUS    Status = STATUS_SUCCESS;
	LARGE_INTEGER CurrentTime = { 0, 0 };

    static const NXL_SECTION DefaultSectionAttr = { { '.', 'A', 't', 't', 'r', 's', 0, 0 }, 2048, 0xF1E8BA9E };
    static const NXL_SECTION DefaultSectionRights = { { '.', 'R', 'i', 'g', 'h', 't', 's', 0 }, NXL_PAGE_SIZE, 0xC71C0011 };
    static const NXL_SECTION DefaultSectionTags = { { '.', 'T', 'a', 'g', 's', 0, 0, 0 }, NXL_PAGE_SIZE, 0xC71C0011 };

	KeQuerySystemTime(&CurrentTime);


	__try {

		RtlZeroMemory(Header, sizeof(NXL_HEADER));

		// Create Thumbprint

		// signature
		Header->Signature.Code.HighPart = NXL_SIGNATURE_HIGH;
		Header->Signature.Code.LowPart  = NXL_SIGNATURE_LOW;
		RtlCopyMemory(Header->Signature.Message, NXL_DEFAULT_MSG, sizeof(NXL_DEFAULT_MSG));

		// basic
		Status = NkMd5Hash(&CurrentTime, sizeof(LARGE_INTEGER), Header->Basic.Thumbprint);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		Header->Basic.Version = NXL_VERSION_10;
		Header->Basic.Flags = 0;
		Header->Basic.Alignment = NXL_PAGE_SIZE;
		Header->Basic.PointerOfContent = NXL_MIN_SIZE;

		// crypto
		Header->Crypto.Algorithm = NXL_ALGORITHM_AES256;
		Header->Crypto.CbcSize = NXL_CBC_SIZE;
		Header->Crypto.ContentLength = 0;
		Header->Crypto.AllocateLength = 0;

		// Primary Key
		Status = SetPrimaryKey(PrimaryKey, AesKey, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		Status = SetRecoveryKey(RecoveryKey, AesKey, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		// sections
		Header->Sections.Count = 3;
		RtlCopyMemory(&Header->Sections.Sections[0], &DefaultSectionAttr, sizeof(NXL_SECTION));
		RtlCopyMemory(&Header->Sections.Sections[1], &DefaultSectionRights, sizeof(NXL_SECTION));
		RtlCopyMemory(&Header->Sections.Sections[2], &DefaultSectionTags, sizeof(NXL_SECTION));

		// Checksum
		Status = SetSectionTableChecksum(AesKey, Header);

try_exit: NOTHING;
	}
	__finally {
		if (!NT_SUCCESS(Status)) {
			RtlZeroMemory(Header, sizeof(NXL_HEADER));
		}
	}

	return Status;
}

VOID
NXLGetOrignalFileExtension(
						   _In_ PCUNICODE_STRING FileName,
						   _In_ PUNICODE_STRING FileExtension
						   )
{
	UNICODE_STRING ParentDir = { 0, 0, NULL };
	UNICODE_STRING FinalPart = { 0, 0, NULL };
	UNICODE_STRING NXLExtension = { 8, 10, L".nxl" };
	USHORT i = 0;
	USHORT Max = 0;

	NkBackSplitUnicodeString(*FileName, L'\\', &FinalPart, &ParentDir);
	if (NkEndsWithUnicodeString(&FinalPart, &NXLExtension, TRUE)) {
		FinalPart.Length -= NXLExtension.Length;
		FinalPart.MaximumLength -= NXLExtension.Length;
	}

	RtlZeroMemory(FileExtension, sizeof(UNICODE_STRING));
	Max = FinalPart.Length / sizeof(WCHAR);
	for (i = 0; i < Max; i++) {
		if (L'.' == FinalPart.Buffer[Max - i - 1]) {
			FileExtension->Buffer = FinalPart.Buffer + (Max - i - 1);
			FileExtension->Length = (i + 1) * sizeof(WCHAR);
			FileExtension->MaximumLength = FileExtension->Length;
			return;
		}
	}
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCreateFile(
			  _In_ PFLT_FILTER Filter,
			  _In_ PFLT_INSTANCE Instance,
			  _In_ PCUNICODE_STRING FileName,
			  _In_ PCNXL_KEKEY_BLOB PrimaryKey,
			  _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
			  _In_ BOOLEAN Overwrite
			  )
{
	NTSTATUS     Status = STATUS_SUCCESS;
	HANDLE       FileHandle = NULL;
	PFILE_OBJECT FileObject = NULL;
	ULONG        Result = 0;
	FILE_DISPOSITION_INFORMATION DispoInfo = { TRUE };
	UNICODE_STRING Extension = { 0, 0, NULL };

	PAGED_CODE();

	__try {

		Status = NkFltCreateFile(Filter,
								 Instance,
								 &FileHandle,
								 GENERIC_READ | GENERIC_WRITE,
								 FileName,
								 &Result,
								 FILE_ATTRIBUTE_NORMAL,
								 0,
								 Overwrite ? FILE_OVERWRITE_IF : FILE_CREATE,
								 FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING,
								 IO_FORCE_ACCESS_CHECK);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = ObReferenceObjectByHandle(FileHandle, GENERIC_READ | GENERIC_WRITE, *IoFileObjectType, KernelMode, &FileObject, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		NXLGetOrignalFileExtension(FileName, &Extension);
		Status = NXLCreateEmptyFile(Instance, FileObject, PrimaryKey,  RecoveryKey, (0 == Extension.Length) ? NULL : (&Extension), NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}


try_exit: NOTHING;
	}
	__finally {

		// Delete new created file if failed
		if (!NT_SUCCESS(Status) && FILE_CREATED == Result) {
			if(NULL != FileHandle && NULL != FileObject) {
				(VOID)FltSetInformationFile(Instance, FileObject, &DispoInfo, sizeof(FILE_DISPOSITION_INFORMATION), FileDispositionInformation);
			}
		}

		// Close file
		if (NULL != FileObject) {
			ObDereferenceObject(FileObject);
			FileObject = NULL;
		}
		if (NULL != FileHandle) {
			NkFltCloseFile(FileHandle);
			FileHandle = NULL;
		}
	}

	return Status;
}


_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLCreateEmptyFile(
				   _In_ PFLT_INSTANCE Instance,
				   _In_ PFILE_OBJECT FileObject,
				   _In_ PCNXL_KEKEY_BLOB PrimaryKey,
				   _In_opt_ PCNXL_KEKEY_BLOB RecoveryKey,
				   _In_opt_ PCUNICODE_STRING Extension,
				   _Out_writes_opt_(32) PUCHAR ContentKey
				   )
{
	NTSTATUS      Status = STATUS_SUCCESS;
	PNXL_HEADER   Header = NULL;
	LARGE_INTEGER FileSize = { 0, 0, };
	UCHAR         AesKey[32];

	PAGED_CODE();

	__try {

		LARGE_INTEGER Offset = { 0, 0 };
		ULONG         BytesWritten = 0;

		if (NULL != ContentKey) {
			RtlZeroMemory(ContentKey, 32);
		}

		Header = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}

		// Set File Size
		FileSize.QuadPart = 0;
		Status = NkFltSetFileSize(Instance, FileObject, FileSize);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		FileSize.QuadPart = NXL_MIN_SIZE;
		Status = NkFltSetFileSize(Instance, FileObject, FileSize);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		// Fill Default Header
		RtlZeroMemory(Header, sizeof(NXL_HEADER));
		RtlZeroMemory(AesKey, sizeof(AesKey));

		Status = NkAesGenerateKey(AesKey, 32);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = FillDefaultHeader(PrimaryKey, RecoveryKey, AesKey, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		// If extension exists, set extension
		if (NULL != Extension && 0 != Extension->Length) {
			PUCHAR SectionAttrs = NULL;
			ULONG  Pos = 0;
			SectionAttrs = ExAllocatePoolWithTag(PagedPool, 2048, TAG_TEMP);
			if (NULL != SectionAttrs) {
				RtlZeroMemory(SectionAttrs, 2048);
				RtlCopyMemory(SectionAttrs, L"$FileExt=", 18); Pos = 18;
				if (L'.' != Extension->Buffer[0]) {
					RtlCopyMemory(SectionAttrs + Pos, L".", 2); Pos += 2;
				}
				RtlCopyMemory(SectionAttrs + Pos, Extension->Buffer, Extension->Length);

				Offset.QuadPart = sizeof(NXL_HEADER);
#pragma prefast(suppress: 6385, "somehow prefast report a nonsense warning here")
				if (NT_SUCCESS(FltWriteFile(Instance, FileObject, &Offset, 2048, SectionAttrs, FLTFL_IO_OPERATION_NON_CACHED, &BytesWritten, NULL, NULL))) {
					// Update checksum
					Header->Sections.Sections[0].Checksum = NkCrc32(0, SectionAttrs, 2048);
				}
				// Update section table checksum
				(VOID)SetSectionTableChecksum(AesKey, Header);
				ExFreePool(SectionAttrs);
			}
		}

		// Write Header
		Offset.QuadPart = 0;
		Status = FltWriteFile(Instance, FileObject, &Offset, sizeof(NXL_HEADER), Header, FLTFL_IO_OPERATION_NON_CACHED, &BytesWritten, NULL, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		if (sizeof(NXL_HEADER) != BytesWritten) {
			try_return(Status = STATUS_UNSUCCESSFUL);
		}

		if (NULL != ContentKey) {
			RtlCopyMemory(ContentKey, AesKey, 32);
		}



try_exit: NOTHING;
	}
	__finally {

		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLReadHeader(
			  _In_ PFLT_INSTANCE Instance,
			  _In_ PFILE_OBJECT FileObject,
			  _Out_ PNXL_HEADER Header
			  )
{
	LARGE_INTEGER   Offset = { 0, 0 };
	ULONG           BytesRead = 0;

	PAGED_CODE();

	RtlZeroMemory(Header, sizeof(NXL_HEADER));
	return NkFltSyncReadFile(Instance,
							 FileObject,
							 &Offset,
							 sizeof(NXL_HEADER),
							 Header,
							 FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET,
							 &BytesRead
							 );
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLWriteHeader(
			   _In_ PFLT_INSTANCE Instance,
			   _In_ PFILE_OBJECT FileObject,
			   _In_ PCNXL_HEADER Header
			   )
{
	LARGE_INTEGER   Offset = { 0, 0 };
	ULONG           BytesRead = 0;

	PAGED_CODE();

	return NkFltSyncWriteFile(Instance,
							  FileObject,
							  &Offset,
							  sizeof(NXL_HEADER),
							  (PVOID)Header,
							  FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET,
							  &BytesRead
							  );
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLGetContentKey(
				 _In_ PCNXL_HEADER Header,
				 _In_reads_(32) PUCHAR PrimaryKey,
				 _Out_writes_(32) PUCHAR ContentKey
				 )
{
	NTSTATUS Status = STATUS_SUCCESS;
	UCHAR    CipherKey[NX_CEK_MAX_LEN];
	HANDLE   AesObject = NULL;

	PAGED_CODE();

	RtlZeroMemory(ContentKey, 32);

	try {

		Status = NkCreateAesObject(PrimaryKey, 32, NXL_CBC_SIZE, &AesObject);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		RtlCopyMemory(CipherKey, Header->Crypto.PrimaryKey.CeKey, NX_CEK_MAX_LEN);
		Status = NkAesDecrypt(AesObject, 0, CipherKey, NX_CEK_MAX_LEN);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		RtlCopyMemory(ContentKey, CipherKey, 32);

try_exit: NOTHING;
	}
	finally {
		if (NULL != AesObject) {
			NkDestroyAesObject(AesObject);
			AesObject = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLReadData(
			_In_ PFLT_INSTANCE Instance,
			_In_ PFILE_OBJECT FileObject,
			_In_ const ULONG HeaderSize,
			_In_ const UCHAR* ContentKey,
			_In_ const LARGE_INTEGER ContentLength,
			_In_ const LARGE_INTEGER AllocationLength,
			_In_ const LARGE_INTEGER Offset,
			_Out_writes_(BytesToRead) PVOID Data,
			_In_ const ULONG BytesToRead,
			_Out_ PULONG BytesRead
			)
{
	NTSTATUS        Status = STATUS_SUCCESS;
	LARGE_INTEGER   ContentOffset = { 0, 0 };
	HANDLE          AesObject = NULL;

	PAGED_CODE();


	// We don't know the sector size, at least we should make sure
	// the size is aligned with 512
	if (0 != (BytesToRead % 512)) {
		return STATUS_INVALID_PARAMETER;
	}
	if (0 != (Offset.QuadPart % 512)) {
		return STATUS_INVALID_PARAMETER;
	}

	*BytesRead = 0;

	if (Offset.QuadPart >= ContentLength.QuadPart) {
		return STATUS_SUCCESS;
	}

	__try {

		ULONG   CipherLength = 0;
		
		ContentOffset.QuadPart = Offset.QuadPart + HeaderSize;
		Status = NkFltSyncReadFile(Instance, FileObject, (PLARGE_INTEGER)&Offset, BytesToRead, Data, FLTFL_IO_OPERATION_NON_CACHED, BytesRead);
		if (!NT_SUCCESS(Status)) {
			*BytesRead = 0;
			try_return(Status);
		}

		if (0 == *BytesRead) {
			try_return(Status);
		}

		if ((Offset.QuadPart + *BytesRead) >= ContentLength.QuadPart) {
			*BytesRead = (ULONG)(ContentLength.QuadPart - Offset.QuadPart);
		}

		CipherLength = *BytesRead;
		ASSERT(0 == (CipherLength%NXL_CBC_SIZE));

		// Decrypt
		Status = NkCreateAesObject(ContentKey, 32, NXL_CBC_SIZE, &AesObject);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = NkAesDecrypt(AesObject, Offset.QuadPart, Data, CipherLength);


try_exit: NOTHING;
	}
	__finally {
		
		if (NULL != AesObject) {
			NkDestroyAesObject(AesObject);
			AesObject = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLWriteData(
			 _In_ PFLT_INSTANCE Instance,
			 _In_ PFILE_OBJECT FileObject,
			 _In_ const ULONG HeaderSize,
			 _In_ const UCHAR* ContentKey,
			 _In_ const LARGE_INTEGER ContentLength,
			 _In_ const LARGE_INTEGER AllocateLength,
			 _In_ const LARGE_INTEGER Offset,
			 _Inout_updates_(BytesToWrite) PVOID Data,
			 _In_ const ULONG BytesToWrite,
			 _Out_ PULONG BytesWritten
			 )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	LARGE_INTEGER   ContentOffset = { 0, 0 };
	HANDLE          AesObject = NULL;

	PAGED_CODE();


	// We don't know the sector size, at least we should make sure
	// the size is aligned with 512
	if (0 != (BytesToWrite % 512)) {
		return STATUS_INVALID_PARAMETER;
	}
	if (0 != (Offset.QuadPart % 512)) {
		return STATUS_INVALID_PARAMETER;
	}

	*BytesWritten = 0;

	if (Offset.QuadPart >= AllocateLength.QuadPart) {
		return STATUS_INVALID_PARAMETER;
	}

	__try {
		
		// Encrypt
		Status = NkCreateAesObject(ContentKey, 32, NXL_CBC_SIZE, &AesObject);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = NkAesEncrypt(AesObject, Offset.QuadPart, Data, BytesToWrite);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		ContentOffset.QuadPart = Offset.QuadPart + HeaderSize;
		Status = NkFltSyncReadFile(Instance, FileObject, (PLARGE_INTEGER)&Offset, BytesToWrite, Data, FLTFL_IO_OPERATION_NON_CACHED, BytesWritten);
		if (!NT_SUCCESS(Status)) {
			*BytesWritten = 0;
		}

try_exit: NOTHING;
	}
	__finally {

		if (NULL != AesObject) {
			NkDestroyAesObject(AesObject);
			AesObject = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLGetFileSize(
			   _In_ PFLT_INSTANCE Instance,
			   _In_ PFILE_OBJECT FileObject,
			   _Out_ PLARGE_INTEGER FileSize
			   )
{
	NTSTATUS    Status = STATUS_SUCCESS;
	PNXL_HEADER Header = NULL;

	PAGED_CODE();

	__try {

		Header = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}
		RtlZeroMemory(Header, sizeof(NXL_HEADER));

		Status = NXLReadHeader(Instance, FileObject, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		FileSize->QuadPart = Header->Crypto.ContentLength;
		Status = STATUS_SUCCESS;

try_exit: NOTHING;
	}
	__finally {
		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLSetFileSize(
			   _In_ PFLT_INSTANCE Instance,
			   _In_ PFILE_OBJECT FileObject,
			   _In_ const UCHAR* ContentKey,
			   _In_ LARGE_INTEGER FileSize
			   )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	PNXL_HEADER     Header = NULL;
	HANDLE          AesObject = NULL;
	PUCHAR          ZeroBuffer = NULL;
	static const ULONG ZeroBufferSize = 16384;   // 16KB
	LARGE_INTEGER   OnDiskFileSize = { 0, 0 };
	LARGE_INTEGER   NewOnDiskFileSize = { 0, 0 };

	PAGED_CODE();

	__try {

		Header = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}
		RtlZeroMemory(Header, sizeof(NXL_HEADER));

		Status = NXLReadHeader(Instance, FileObject, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		if (FileSize.QuadPart == Header->Crypto.ContentLength) {
			try_return(Status = STATUS_SUCCESS);
		}

		Status = NkFltSyncGetFileSize(Instance, FileObject, &OnDiskFileSize);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		NewOnDiskFileSize.QuadPart = Header->Basic.PointerOfContent + NKROUND_TO_SIZE(LONGLONG, FileSize.QuadPart, NXL_PAGE_SIZE);
		if (NewOnDiskFileSize.QuadPart != OnDiskFileSize.QuadPart) {
			(VOID)NkFltSyncSetFileSize(Instance, FileObject, NewOnDiskFileSize);

			if (NewOnDiskFileSize.QuadPart > OnDiskFileSize.QuadPart) {

				LARGE_INTEGER   BytesToZero = { 0, 0 };
				LARGE_INTEGER   WriteOffset = { 0, 0 };
				LARGE_INTEGER   ContentOffset = { 0, 0 };

				// Encrypt
				Status = NkCreateAesObject(ContentKey, 32, NXL_CBC_SIZE, &AesObject);
				if (!NT_SUCCESS(Status)) {
					try_return(Status);
				}

				ZeroBuffer = ExAllocatePoolWithTag(PagedPool, ZeroBufferSize, TAG_TEMP);
				if (NULL == ZeroBuffer) {
					try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
				}

				BytesToZero.QuadPart = NewOnDiskFileSize.QuadPart - OnDiskFileSize.QuadPart;
				WriteOffset.QuadPart = OnDiskFileSize.QuadPart;
				ContentOffset.QuadPart = OnDiskFileSize.QuadPart - Header->Basic.PointerOfContent;
				while (0 != BytesToZero.QuadPart) {

					ULONG   BytesToWrite = 0;
					ULONG   BytesWritten = 0;

					BytesToWrite = (0 != BytesToZero.HighPart) ? ZeroBufferSize : ((BytesToZero.LowPart > ZeroBufferSize) ? ZeroBufferSize : BytesToZero.LowPart);
					RtlZeroMemory(ZeroBuffer, BytesToWrite);

					Status = NkAesEncrypt(AesObject, ContentOffset.QuadPart, ZeroBuffer, BytesToWrite);
					if (!NT_SUCCESS(Status)) {
						try_return(Status);
					}

					Status = NkFltSyncWriteFile(Instance, FileObject, &WriteOffset, BytesToWrite, ZeroBuffer, FLTFL_IO_OPERATION_NON_CACHED, &BytesWritten);
					if (!NT_SUCCESS(Status)) {
						try_return(Status);
					}

					BytesToZero.QuadPart -= BytesToWrite;
					WriteOffset.QuadPart += BytesToWrite;
					ContentOffset.QuadPart += BytesToWrite;
				}
			}
		}


		Header->Crypto.ContentLength = FileSize.QuadPart;
		Status = NXLWriteHeader(Instance, FileObject, Header);

try_exit: NOTHING;
	}
	__finally {

		if (NULL != AesObject) {
			NkDestroyAesObject(AesObject);
			AesObject = NULL;
		}

		if (NULL != ZeroBuffer) {
			ExFreePool(ZeroBuffer);
			ZeroBuffer = NULL;
		}

		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLReadSectionData(
				   _In_ PFLT_INSTANCE Instance,
				   _In_ PFILE_OBJECT FileObject,
				   _In_ const CHAR* Name,
				   _Out_writes_opt_(*Size) PVOID Data,
				   _Inout_ PULONG Size
				   )
{
	NTSTATUS    Status = STATUS_REQUEST_NOT_ACCEPTED;
	PNXL_HEADER Header = NULL;

	PAGED_CODE();


	try {
	
		const NXL_SECTION*  Section = NULL;
		LARGE_INTEGER       Offset = { 0, 0 };
		const ULONG  BufferSize = *Size;
		ULONG        BytesToRead = 0;
		ULONG        BytesRead = 0;

		*Size = 0;

		Header = (PNXL_HEADER)ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}
		RtlZeroMemory(Header, sizeof(NXL_HEADER));

		Status = NXLReadHeader(Instance, FileObject, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		if (0 != NXLValidateHeader(Header)) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		Section = NXLFindSection(Header, Name, &(Offset.LowPart));
		if (NULL == Section) {
			try_return(Status = STATUS_NOT_FOUND);
		}

		ASSERT(0 != Offset.QuadPart);
		if (0 == Offset.QuadPart) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		// Only query size?
		if (NULL == Data || 0 == BufferSize) {
			*Size = Section->Size;
			try_return(Status = STATUS_BUFFER_TOO_SMALL);
		}

		// How many bytes can be read
		BytesToRead = min(Section->Size, BufferSize);
		// Read data
		Status = NkFltSyncReadFile(Instance, FileObject, &Offset, BytesToRead, Data, 0, &BytesRead);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		*Size = BytesRead;

		// Do we read all the data?
		if (BufferSize < Section->Size) {
			Status = STATUS_BUFFER_OVERFLOW;
		}
 
try_exit: NOTHING;
	}
	finally {

		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NXLWriteSectionData(
					_In_ PFLT_INSTANCE Instance,
					_In_ PFILE_OBJECT FileObject,
					_In_ const UCHAR* ContentKey,
					_In_ const CHAR* Name,
					_In_reads_(Size) const VOID* Data,
					_In_ ULONG Size
					)
{
	NTSTATUS    Status = STATUS_REQUEST_NOT_ACCEPTED;
	PNXL_HEADER Header = NULL;
	PUCHAR      SectionData = NULL;

	PAGED_CODE();


	try {

		NXL_SECTION*    Section = NULL;
		LARGE_INTEGER   Offset = { 0, 0 };
		ULONG           BytesWritten = 0;

		Header = (PNXL_HEADER)ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}
		RtlZeroMemory(Header, sizeof(NXL_HEADER));

		Status = NXLReadHeader(Instance, FileObject, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		if (0 != NXLValidateHeader(Header)) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		Section = NXLFindSection(Header, Name, &(Offset.LowPart));
		if (NULL == Section) {
			try_return(Status = STATUS_NOT_FOUND);
		}

		ASSERT(0 != Offset.QuadPart);
		if (0 == Offset.QuadPart) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		if (Section->Size < Size) {
			try_return(Status = STATUS_DATA_OVERRUN);
		}

		SectionData = ExAllocatePoolWithTag(PagedPool, Section->Size, TAG_TEMP);
		if (NULL == SectionData) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}
		RtlCopyMemory(SectionData, Data, Size);
		if (Size < Section->Size) {
			RtlZeroMemory(SectionData + Size, Section->Size - Size);
		}

		Status = NkFltSyncWriteFile(Instance, FileObject, &Offset, Section->Size, SectionData, 0, &BytesWritten);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		// Calculate checksum
		Section->Checksum = NkCrc32(0, SectionData, Section->Size);
		Status = SetSectionTableChecksum(ContentKey, Header);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		// Update header because we updated the checksum
		Status = NXLWriteHeader(Instance, FileObject, Header);


try_exit: NOTHING;
	}
	finally {

		if (NULL != SectionData) {
			ExFreePool(SectionData);
			SectionData = NULL;
		}

		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLGetSectionBlock(
				   _In_ PFLT_INSTANCE Instance,
				   _In_ PFILE_OBJECT FileObject,
				   _Outptr_result_maybenull_ PNXL_SECTION_TABLE* Buffer,
				   _Out_ PULONG Size
				   )
{
	NTSTATUS Status = STATUS_SUCCESS;
	PNXL_HEADER Header = NULL;
	PNXL_SECTION_TABLE Block = NULL;
	ULONG DesiredSize = 0;


	PAGED_CODE();

	*Buffer = NULL;
	*Size = 0;


	try {

		LARGE_INTEGER   Offset = { 0, 0 };
		ULONG           BytesRead = 0;


		Header = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}

		Status = FltReadFile(Instance, FileObject, &Offset, sizeof(NXL_HEADER), Header, FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, &BytesRead, NULL, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		if (sizeof(NXL_HEADER) != BytesRead) {
			try_return(Status = STATUS_FILE_INVALID);
		}
		if (0 != NXLValidateHeader(Header)) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		DesiredSize = Header->Basic.PointerOfContent - FIELD_OFFSET(NXL_HEADER, Sections);
		Block = ExAllocatePoolWithTag(PagedPool, DesiredSize, TAG_TEMP);
		if (NULL == Block) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}

		Offset.QuadPart = FIELD_OFFSET(NXL_HEADER, Sections);
		Status = FltReadFile(Instance, FileObject, &Offset, DesiredSize, Block, FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, &BytesRead, NULL, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		if (DesiredSize != BytesRead) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		*Size = DesiredSize;
		*Buffer = Block;
		Block = NULL;   // Transfer the ownership
		Status = STATUS_SUCCESS;

	try_exit: NOTHING;
	}
	finally {
		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
		if (NULL != Block) {
			ExFreePool(Block);
			Block = NULL;
		}
	}

	return Status;
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLSetSectionBlock(
				   _In_ PFLT_INSTANCE Instance,
				   _In_ PFILE_OBJECT FileObject,
				   _In_ const UCHAR* ContentKey,
				   _In_reads_(Size) PCNXL_SECTION_TABLE Buffer,
				   _In_ const ULONG Size
				   )
{
	PAGED_CODE();
	return NXLSetSectionBlockEx(Instance, FileObject, ContentKey, Buffer, Size, NULL);
}

_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NXLSetSectionBlockEx(
					 _In_ PFLT_INSTANCE Instance,
					 _In_ PFILE_OBJECT FileObject,
					 _In_ const UCHAR* ContentKey,
					 _In_reads_(Size) PCNXL_SECTION_TABLE Buffer,
					 _In_ const ULONG Size,
					 _In_opt_ PCUNICODE_STRING NewExtension
					 )
{
	NTSTATUS    Status = STATUS_SUCCESS;
	PNXL_HEADER Header = NULL;
	PNXL_SECTION_TABLE Block = NULL;
	ULONG       BlockSize = 0;
	BOOLEAN     Overflow = FALSE;

	PAGED_CODE();


	if (Size < sizeof(NXL_SECTION_TABLE)) {
		return STATUS_INVALID_PARAMETER;
	}

	try {

		LARGE_INTEGER   Offset = { 0, 0 };
		ULONG           BytesRead = 0;
		ULONG           BytesWritten = 0;
		ULONG           i = 0;
		PUCHAR          Data = NULL;
		ULONG           DataSize = 0;
		PUCHAR          InData = NULL;


		for (i = 0; i < Buffer->Count; i++) {
			DataSize += Buffer->Sections[i].Size;
		}
		if (DataSize > (Size - sizeof(NXL_SECTION_TABLE))) {
			try_return(Status = STATUS_INVALID_PARAMETER);
		}

		Header = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_HEADER), TAG_TEMP);
		if (NULL == Header) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}

		Status = FltReadFile(Instance, FileObject, &Offset, sizeof(NXL_HEADER), Header, FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, &BytesRead, NULL, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}
		if (sizeof(NXL_HEADER) != BytesRead) {
			try_return(Status = STATUS_FILE_INVALID);
		}
		if (0 != NXLValidateHeader(Header)) {
			try_return(Status = STATUS_FILE_INVALID);
		}

		BlockSize = Header->Basic.PointerOfContent - FIELD_OFFSET(NXL_HEADER, Sections);
		Block = ExAllocatePoolWithTag(PagedPool, BlockSize, TAG_TEMP);
		if (NULL == Block) {
			try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
		}

		Data = (PUCHAR)Block + sizeof(NXL_SECTION_TABLE);
		InData = (PUCHAR)Buffer + sizeof(NXL_SECTION_TABLE);
		DataSize = BlockSize - sizeof(NXL_SECTION_TABLE);
		RtlZeroMemory(Block, BlockSize);

		for (i = 0; i < Buffer->Count; i++) {

			if (Buffer->Sections[i].Size > DataSize) {
				Overflow = TRUE;
				break;
			}

			if (NULL != NewExtension && 0 != NewExtension->Length && NkCompareStringA(Buffer->Sections[i].Name, NXL_SECTION_ATTRIBUTES, TRUE)) {
				RtlCopyMemory(Data, L"$FileExt=", 18);
				RtlCopyMemory(Data + 18, NewExtension->Buffer, NewExtension->Length);
				RtlCopyMemory(&Block->Sections[i], &Buffer->Sections[i], sizeof(NXL_SECTION));
				Block->Sections[i].Checksum = NkCrc32(0, Data, Buffer->Sections[i].Size);
			}
			else {
				RtlCopyMemory(Data, InData, Buffer->Sections[i].Size);
				RtlCopyMemory(&Block->Sections[i], &Buffer->Sections[i], sizeof(NXL_SECTION));
			}
			Data += Buffer->Sections[i].Size;
			InData += Buffer->Sections[i].Size;
			Block->Count++;
			DataSize -= Buffer->Sections[i].Size;
		}

		Status = SetSectionTableChecksumEx(ContentKey, Block);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Offset.QuadPart = FIELD_OFFSET(NXL_HEADER, Sections);
		Status = FltWriteFile(Instance, FileObject, &Offset, BlockSize, Block, FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, &BytesWritten, NULL, NULL);
		if (!NT_SUCCESS(Status)) {
			try_return(Status);
		}

		Status = Overflow ? STATUS_BUFFER_OVERFLOW : STATUS_SUCCESS;

try_exit: NOTHING;
	}
	finally {
		if (NULL != Header) {
			ExFreePool(Header);
			Header = NULL;
		}
		if (NULL != Block) {
			ExFreePool(Block);
			Block = NULL;
		}
	}

	return Status;
}