

#include <Windows.h>
#include <assert.h>
#include <winternl.h>
#include <nudf\shared\nxlfmt.h>
#include <nudf\shared\fltdef.h>
#include <nudf\exception.hpp>
#include <nudf\convert.hpp>
#include <nudf\nxlutil.hpp>



using namespace nudf::util::nxl;

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

typedef NTSTATUS (WINAPI *ZWQUERYEAFILE) (
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_Out_writes_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length,
	_In_ BOOLEAN ReturnSingleEntry,
	_In_reads_bytes_opt_(EaListLength) PVOID EaList,
	_In_ ULONG EaListLength,
	_In_opt_ PULONG EaIndex,
	_In_ BOOLEAN RestartScan
	);

typedef NTSTATUS (WINAPI *ZWSETEAFILE) (
	_In_ HANDLE FileHandle,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_reads_bytes_(Length) PVOID Buffer,
	_In_ ULONG Length
	);

#pragma pack(push, 4)

typedef struct _FILE_GET_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR EaNameLength;
	CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION {
	ULONG NextEntryOffset;
	UCHAR Flags;
	UCHAR EaNameLength;
	USHORT EaValueLength;
	CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

#pragma pack(pop)

HRESULT __stdcall nudf::util::nxl::NxrmEncryptFile(const WCHAR *FileName)
{
	//HRESULT hr = S_OK;

	//HANDLE hFile = INVALID_HANDLE_VALUE;

	//UCHAR EaBuf[64] = {0};
	//ULONG EaInputLength = 0;
	//ULONG EaOutputLength = 0;

	//UCHAR EaValue = 0;

	//FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	//FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	//ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;
	//ZWSETEAFILE fn_ZwSetEaFile = NULL;

	//NTSTATUS Status = STATUS_SUCCESS;
	//IO_STATUS_BLOCK	IoStatus = {0};

	//fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");
	//fn_ZwSetEaFile = (ZWSETEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetEaFile");

	//do 
	//{
	//	hFile = CreateFileW(FileName,
	//						GENERIC_READ|GENERIC_WRITE,
	//						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	//						NULL,
	//						OPEN_EXISTING,
	//						FILE_ATTRIBUTE_NORMAL,
	//						NULL);

	//	if (hFile == INVALID_HANDLE_VALUE)
	//	{
	//		hr = HRESULT_FROM_WIN32(GetLastError());
	//		break;
	//	}

	//	pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
	//	pEaInfo->NextEntryOffset = 0;
	//	pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
	//	memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

	//	EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
	//	EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

	//	Status = fn_ZwQueryEaFile(hFile,
	//							  &IoStatus,
	//							  &EaBuf,
	//							  EaOutputLength,
	//							  TRUE,
	//							  EaBuf,
	//							  EaInputLength,
	//							  NULL,
	//							  TRUE);

	//	if (Status != STATUS_SUCCESS)
	//	{
	//		hr = HRESULT_FROM_NT(Status);
	//		break;
	//	}

	//	pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

	//	if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
	//	{
	//		//
	//		// nxrmflt.sys is not running or rights management service is not running
	//		//
	//		hr = E_UNEXPECTED;
	//		break;
	//	}

	//	EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

	//	if (EaValue == NXRM_CONTENT_IS_ENCRYPTED)
	//	{
	//		break;
	//	}

	//	memset(EaBuf, 0, sizeof(EaBuf));

	//	pFullEaInfo->NextEntryOffset = 0;
	//	pFullEaInfo->Flags = 0;
	//	pFullEaInfo->EaNameLength	= (UCHAR)strlen(NXRM_EA_ENCRYPT_CONTENT);
	//	pFullEaInfo->EaValueLength	= sizeof(UCHAR);

	//	memcpy(pFullEaInfo->EaName, NXRM_EA_ENCRYPT_CONTENT, sizeof(NXRM_EA_ENCRYPT_CONTENT));

	//	*(UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1) = 1;

	//	EaInputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_ENCRYPT_CONTENT) + sizeof(UCHAR);

	//	Status = fn_ZwSetEaFile(hFile,
	//							&IoStatus,
	//							EaBuf,
	//							EaInputLength);

	//	if (Status != STATUS_SUCCESS)
	//	{
	//		hr = E_UNEXPECTED;
	//		break;
	//	}

	//	if (IoStatus.Status != STATUS_SUCCESS)
	//	{
	//		hr = HRESULT_FROM_NT(IoStatus.Status);
	//		break;
	//	}

	//	//
	//	// verify the file is encrypted in case driver was stopped between the first QueryEa call and SetEa call
	//	//
	//	memset(EaBuf, 0, sizeof(EaBuf));

	//	pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
	//	pEaInfo->NextEntryOffset = 0;
	//	pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
	//	memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

	//	EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
	//	EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

	//	Status = fn_ZwQueryEaFile(hFile,
	//							 &IoStatus,
	//							 EaBuf,
	//							 EaOutputLength,
	//							 TRUE,
	//							 EaBuf,
	//							 EaInputLength,
	//							 NULL,
	//							 TRUE);

	//	if (Status != STATUS_SUCCESS)
	//	{
	//		hr = HRESULT_FROM_NT(Status);
	//		break;
	//	}

	//	pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

	//	if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
	//	{
	//		//
	//		// nxrmflt.sys is not running or rights management service is not running
	//		//
	//		hr = E_UNEXPECTED;
	//		break;
	//	}

	//	EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

	//	if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
	//	{
	//		hr = E_UNEXPECTED;
	//		break;
	//	}

	//} while (FALSE);

	//if (hFile != INVALID_HANDLE_VALUE)
	//{
	//	CloseHandle(hFile);
	//	hFile = INVALID_HANDLE_VALUE;
	//}

	//return hr;

	return NxrmEncryptFileEx(FileName, NULL, NULL, 0);
}
HRESULT __stdcall nudf::util::nxl::NxrmEncryptFileEx(const WCHAR *FileName, const char *tag, UCHAR *data, USHORT datalen)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	do 
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ|FILE_WRITE_EA,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		hr = NxrmEncryptFileEx2(hFile, tag, data, datalen);

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmEncryptFileEx2(HANDLE hFile, const char *tag, UCHAR *data, USHORT datalen)
{
	HRESULT hr = S_OK;

	UCHAR EaBuf[64] = {0};
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;
	ZWSETEAFILE fn_ZwSetEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = {0};

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");
	fn_ZwSetEaFile = (ZWSETEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetEaFile");

	do 
	{
		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue == NXRM_CONTENT_IS_ENCRYPTED)
		{
			break;
		}

		memset(EaBuf, 0, sizeof(EaBuf));

		pFullEaInfo->NextEntryOffset = 0;
		pFullEaInfo->Flags = 0;
		pFullEaInfo->EaNameLength	= (UCHAR)strlen(NXRM_EA_ENCRYPT_CONTENT);
		pFullEaInfo->EaValueLength	= sizeof(UCHAR);

		memcpy(pFullEaInfo->EaName, NXRM_EA_ENCRYPT_CONTENT, sizeof(NXRM_EA_ENCRYPT_CONTENT));

		*(UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1) = 1;

		EaInputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_ENCRYPT_CONTENT) + sizeof(UCHAR);

		Status = fn_ZwSetEaFile(hFile,
								&IoStatus,
								EaBuf,
								EaInputLength);

		if (Status != STATUS_SUCCESS)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (IoStatus.Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(IoStatus.Status);
			break;
		}

		//
		// verify the file is encrypted in case driver was stopped between the first QueryEa call and SetEa call
		//
		memset(EaBuf, 0, sizeof(EaBuf));

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (data && datalen && strlen(tag))
		{
			hr = NxrmSyncNXLHeader(hFile, tag, data, datalen);
		}

	} while (FALSE);

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmSyncNXLHeader(HANDLE hFile, const char *tag, UCHAR *data, USHORT datalen)
{
	HRESULT hr = S_OK;

	UCHAR EaBuf[64] = {0};
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;
	ZWSETEAFILE fn_ZwSetEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = {0};

	UCHAR *pEaData = NULL;
	UCHAR *pEaValue = NULL;

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");
	fn_ZwSetEaFile = (ZWSETEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetEaFile");

	do 
	{
		//
		// only accept one of three sections
		//
		if (strcmp(tag, NXL_SECTION_TAGS) &&
			strcmp(tag, NXL_SECTION_ATTRIBUTES) &&
			strcmp(tag, NXL_SECTION_TEMPLATES))
		{
			hr = E_UNEXPECTED;
			break;
		}

		//
		// step 1: making sure 1) driver is running 2) file is a encrypted NXL file
		//
		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

		//
		// step 2:  Sending command to kernel
		//

		//
		// calc input length
		//
		EaInputLength = (ULONG)(sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_TAG) + strlen(tag) + sizeof('\0') + \
								sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_SYNC_HEADER) + datalen);

		pEaData = (UCHAR*)malloc(EaInputLength);

		if (!pEaData)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		memset(pEaData, 0 , EaInputLength);

		//
		// EA 1: TAG
		pFullEaInfo = (FILE_FULL_EA_INFORMATION *) pEaData;

		pFullEaInfo->NextEntryOffset	= (ULONG)(sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_TAG) + strlen(tag) + sizeof('\0'));
		pFullEaInfo->Flags				= 0;
		pFullEaInfo->EaNameLength		= (UCHAR)(strlen(NXRM_EA_TAG));
		pFullEaInfo->EaValueLength		= (USHORT)(strlen(tag) + sizeof('\0'));

		memcpy(pFullEaInfo->EaName, NXRM_EA_TAG, sizeof(NXRM_EA_TAG));

		pEaValue = ((UCHAR*)pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1);
		
		memcpy(pEaValue, tag, strlen(tag) + sizeof('\0'));

		//
		// EA 2: SYN header
		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)((UCHAR*)pFullEaInfo + pFullEaInfo->NextEntryOffset);

		pFullEaInfo->NextEntryOffset	= 0;
		pFullEaInfo->Flags				= 0;
		pFullEaInfo->EaNameLength		= (UCHAR)strlen(NXRM_EA_SYNC_HEADER);
		pFullEaInfo->EaValueLength		= datalen;

		memcpy(pFullEaInfo->EaName, NXRM_EA_SYNC_HEADER, sizeof(NXRM_EA_SYNC_HEADER));
		
		pEaValue = ((UCHAR*)pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1);

		memcpy(pEaValue, data, datalen);

		Status = fn_ZwSetEaFile(hFile,
								&IoStatus,
								pEaData,
								EaInputLength);

		if (Status != STATUS_SUCCESS)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (IoStatus.Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(IoStatus.Status);
			break;
		}

	} while (FALSE);


	if (pEaData)
	{
		free(pEaData);
		pEaData = NULL;
	}

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmReadTags(const WCHAR *FileName, UCHAR *data, USHORT *datalen)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	do 
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		hr = NxrmReadTagsEx(hFile, data, datalen);

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmReadTagsEx(HANDLE hFile, UCHAR *data, USHORT *datalen)
{
	HRESULT hr = S_OK;

	UCHAR EaBuf[64] = {0};
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;
	FILE_GET_EA_INFORMATION	 *pQueryTagEa = NULL;

	UCHAR *pEaData = NULL;
	UCHAR *pTagData = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = {0};

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");

	do 
	{

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

		pEaData = (UCHAR*)malloc(sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_TAG) + sizeof(NXL_SECTION_TAGS) + \
								 sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_SYNC_HEADER) + 4096);

		if (!pEaData)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		pQueryTagEa = (FILE_GET_EA_INFORMATION *)pEaData;

		pQueryTagEa->NextEntryOffset	= 0;
		pQueryTagEa->EaNameLength		= sizeof(NXRM_EA_TAG) - sizeof('\0');

		memcpy(pQueryTagEa->EaName, 
			   NXRM_EA_TAG, 
			   sizeof(NXRM_EA_TAG));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_TAG);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_TAG) + sizeof(NXL_SECTION_TAGS) + \
						 sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_SYNC_HEADER) + 4096;

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  pEaData,
								  EaOutputLength,
								  TRUE,
								  pEaData,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}
		
		if (IoStatus.Information < sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_TAG) + sizeof(NXL_SECTION_TAGS) + \
								   sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_SYNC_HEADER))
		{
			hr = E_UNEXPECTED;
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)pEaData;

		if (pFullEaInfo->NextEntryOffset != (ULONG)(sizeof(FILE_FULL_EA_INFORMATION) - 1 + sizeof(NXRM_EA_TAG) + sizeof(NXL_SECTION_TAGS)) ||
			pFullEaInfo->Flags != 0x80 ||
			pFullEaInfo->EaNameLength != sizeof(NXRM_EA_TAG) - sizeof(char) ||
			pFullEaInfo->EaValueLength != sizeof(NXL_SECTION_TAGS))
		{
			hr = E_UNEXPECTED;
			break;
		}
		
		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)((UCHAR*)pFullEaInfo + pFullEaInfo->NextEntryOffset);

		if (pFullEaInfo->NextEntryOffset != 0 ||
			pFullEaInfo->Flags != 0x80 ||
			pFullEaInfo->EaNameLength != sizeof(NXRM_EA_SYNC_HEADER) - sizeof(char))
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (*datalen < pFullEaInfo->EaValueLength)
		{
			hr = E_UNEXPECTED;
			break;
		}

		pTagData = ((UCHAR*)pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1);

		memcpy(data,
			   pTagData,
			   pFullEaInfo->EaValueLength);

		*datalen = pFullEaInfo->EaValueLength;

	} while (FALSE);

	if (pEaData)
	{
		free(pEaData);
		pEaData = NULL;
	}

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmCheckRights(const WCHAR *FileName, ULONGLONG *RightMask, ULONGLONG *CustomRightsMask, ULONGLONG *EvaluationId)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	UCHAR EaBuf[128] = {0};
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = {0};

	ULONGLONG *p = NULL;

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");

	do 
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

		//
		//
		//
		memset(EaBuf, 0, sizeof(EaBuf));

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_CHECK_RIGHTS);
		memcpy(pEaInfo->EaName, NXRM_EA_CHECK_RIGHTS, sizeof(NXRM_EA_CHECK_RIGHTS));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_CHECK_RIGHTS);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_CHECK_RIGHTS) + sizeof(ULONGLONG) + sizeof(ULONGLONG) + sizeof(ULONGLONG);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != (sizeof(ULONGLONG) + sizeof(ULONGLONG) + sizeof(ULONGLONG)))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		p = (ULONGLONG*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1);

		*RightMask			= *(p + 0);
		*CustomRightsMask	= *(p + 1);
		*EvaluationId		= *(p + 2);

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmDecryptFile(const WCHAR *FileName)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	do 
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ|FILE_WRITE_EA,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		hr = NxrmDecryptFile2(hFile);

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;

}

HRESULT __stdcall nudf::util::nxl::NxrmDecryptFile2(HANDLE hFile)
{
	HRESULT hr = S_OK;

	UCHAR EaBuf[64] = {0};
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;
	ZWSETEAFILE fn_ZwSetEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = {0};

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");
	fn_ZwSetEaFile = (ZWSETEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetEaFile");

	do 
	{
		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			break;
		}

		memset(EaBuf, 0, sizeof(EaBuf));

		pFullEaInfo->NextEntryOffset = 0;
		pFullEaInfo->Flags = 0;
		pFullEaInfo->EaNameLength	= (UCHAR)strlen(NXRM_EA_ENCRYPT_CONTENT);
		pFullEaInfo->EaValueLength	= sizeof(UCHAR);

		memcpy(pFullEaInfo->EaName, NXRM_EA_ENCRYPT_CONTENT, sizeof(NXRM_EA_ENCRYPT_CONTENT));

		*(UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1) = 0;

		EaInputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_ENCRYPT_CONTENT) + sizeof(UCHAR);

		Status = fn_ZwSetEaFile(hFile,
								&IoStatus,
								EaBuf,
								EaInputLength);

		if (Status != STATUS_SUCCESS)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (IoStatus.Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(IoStatus.Status);
			break;
		}

	} while (FALSE);

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmIsDecryptedFile(const WCHAR *FileName)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	UCHAR EaBuf[128] = {0};
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = {0};

	ULONGLONG *p = NULL;

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");

	do 
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

HRESULT __stdcall nudf::util::nxl::NxrmSetSourceFileName(const WCHAR *FileName, const WCHAR *SrcNTFileName)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	UCHAR EaBuf[4096] = { 0 };
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;
	ZWSETEAFILE fn_ZwSetEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = { 0 };

	ULONGLONG *p = NULL;

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");
	fn_ZwSetEaFile = (ZWSETEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetEaFile");

	do
	{
		if (wcslen(SrcNTFileName)*sizeof(WCHAR) > 2048)
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
			break;
		}

		hFile = CreateFileW(FileName,
							GENERIC_READ | FILE_WRITE_EA,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

		//
		//
		//
		memset(EaBuf, 0, sizeof(EaBuf));

		pFullEaInfo->NextEntryOffset = 0;
		pFullEaInfo->Flags = 0;
		pFullEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_SET_SOURCE);
		pFullEaInfo->EaValueLength = (USHORT)(min(wcslen(SrcNTFileName) * sizeof(WCHAR), 2048));

		memcpy(pFullEaInfo->EaName, NXRM_EA_SET_SOURCE, sizeof(NXRM_EA_SET_SOURCE));

		memcpy((pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1),
			   SrcNTFileName,
			   min(wcslen(SrcNTFileName) * sizeof(WCHAR), 2048));

		EaInputLength = (ULONG)(sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_SET_SOURCE) + min(wcslen(SrcNTFileName) * sizeof(WCHAR), 2048));

		Status = fn_ZwSetEaFile(hFile,
								&IoStatus,
								EaBuf,
								EaInputLength);

		if (Status != STATUS_SUCCESS)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (IoStatus.Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(IoStatus.Status);
			break;
		}

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

LPCWSTR nudf::util::nxl::NxlStatus2Message(_In_ NXLSTATUS status)
{
	/*    
	NXL_UNKNOWN = -1,
	NXL_OK = 0,
	NXL_BAD_SIGNATURE,
	NXL_BAD_VERSION,
	NXL_BAD_ALIGNMENT,
	NXL_BAD_DATAOFFSET,
	NXL_BAD_ENCRYPT_ALG,
	NXL_BAD_CBC_SIZE,
	NXL_BAD_KEK_ALG,
	NXL_BAD_KEK_ID_SIZE,
	NXL_BAD_SECTION_SIZE
	*/

	switch(status)
	{
	case NXL_OK:
		return L"OK";
	case NXL_BAD_SIGNATURE:
		return L"Bad signature";
	case NXL_BAD_VERSION:
		return L"Bad version";
	case NXL_BAD_ALIGNMENT:
		return L"Bad alignment";
	case NXL_BAD_DATAOFFSET:
		return L"Bad data offset";
	case NXL_BAD_ENCRYPT_ALG:
		return L"Bad encrypt algorithm";
	case NXL_BAD_CBC_SIZE:
		return L"Bad CBC size";
	case NXL_BAD_KEK_ALG:
		return L"Bad KEK algorithm";
	case NXL_BAD_KEK_ID_SIZE:
		return L"Bad KEK id size";
	case NXL_BAD_SECTION_COUNT:
		return L"Bad section count";
	case NXL_UNKNOWN:
	default:
		break;
	}

	return L"Unknown";
}


//
//  class
//

CFile::CFile() : _handle(INVALID_HANDLE_VALUE)
{
}

CFile::CFile(_In_ LPCWSTR path) : _path(path), _handle(INVALID_HANDLE_VALUE)
{
}

CFile::~CFile()
{
	Reset();
}

NXLSTATUS CFile::Validate() throw()
{
	NXLSTATUS status = NXL_UNKNOWN;

	if(IsOpened()) {
		return ValidateHeader();
	}

	try {
		Open(&status);
		Close();
	}
	catch(const nudf::CException& e) {
		UNREFERENCED_PARAMETER(e);
	}

	return status;
}

void CFile::Open(_Out_opt_ NXLSTATUS* status)
{
	OpenEx(true, status);
}

void CFile::OpenEx(_In_ bool readonly, _Out_opt_ NXLSTATUS* status)
{
    if(NULL != status) {
        *status = NXL_UNKNOWN;
    }

	if(_path.empty()) {
		throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
	}

	if(INVALID_HANDLE_VALUE == (_handle = ::CreateFileW(_path.c_str(), readonly ? GENERIC_READ : (GENERIC_READ|GENERIC_WRITE), FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {
		throw WIN32ERROR();
	}

	try {

		DWORD dwRead = 0;
		NXLSTATUS dwValidate = NXL_UNKNOWN;

		_header.resize(sizeof(NXL_HEADER), 0);
		if(!::ReadFile(_handle, &_header[0], (DWORD)sizeof(NXL_HEADER), &dwRead, NULL) || dwRead != (DWORD)sizeof(NXL_HEADER)) {
			throw WIN32ERROR2(ERROR_BAD_FILE_TYPE);
		}

        dwValidate = ValidateHeader();
        if(NULL != status) {
            *status = dwValidate;
        }
		if(0 != dwValidate) {
			throw WIN32ERROR2(ERROR_BAD_FILE_TYPE);
		}
	}
	catch(const nudf::CException& e) {
		_header.clear();
		throw e;
	}
}

void CFile::Close() throw()
{
	if(IsOpened()) {
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;
	}
}

void CFile::Reset() throw()
{
	Close();
	_header.clear();
	_path = L"";
}

void CFile::GetNAttributes(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& attributes)
{
	GetNPairDataW(NXL_SECTION_ATTRIBUTES, attributes);
}

void CFile::SetNAttributes(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& attributes, _Out_opt_ PULONG paires_written)
{
	SetNPairDataW(NXL_SECTION_ATTRIBUTES, attributes, paires_written);
}

void CFile::GetNTags(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& tags)
{
	GetNPairDataW(NXL_SECTION_TAGS, tags);
}

void CFile::SetNTags(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& tags, _Out_opt_ PULONG paires_written)
{
	SetNPairDataW(NXL_SECTION_TAGS, tags, paires_written);
}

NXLSTATUS CFile::ValidateHeader() const throw()
{
	if(_header.empty() || _header.size() != sizeof(NXL_HEADER)) {
		assert(false);
		return NXL_UNKNOWN;
	}

	PCNXL_HEADER header = (PCNXL_HEADER)&_header[0];

	if(header->Signature.Code.HighPart != NXL_SIGNATURE_HIGH || header->Signature.Code.LowPart != NXL_SIGNATURE_LOW) {
		return NXL_BAD_SIGNATURE;
	}

	if(header->Basic.Version != NXL_VERSION_10) {
		return NXL_BAD_VERSION;
	}

	if(header->Basic.Alignment != NXL_PAGE_SIZE) {
		return NXL_BAD_ALIGNMENT;
	}

	if(header->Basic.PointerOfContent < NXL_MIN_SIZE) {
		return NXL_BAD_DATAOFFSET;
	}

	if(header->Crypto.Algorithm != NXL_ALGORITHM_AES128 && header->Crypto.Algorithm != NXL_ALGORITHM_AES256) {
		return NXL_BAD_ENCRYPT_ALG;
	}

	if(header->Crypto.CbcSize != NXL_CBC_SIZE) {
		return NXL_BAD_CBC_SIZE;
	}

	if(header->Crypto.PrimaryKey.KeKeyId.Algorithm != NXL_ALGORITHM_AES128 && header->Crypto.PrimaryKey.KeKeyId.Algorithm != NXL_ALGORITHM_AES256) {
		return NXL_BAD_KEK_ALG;
	}

	if(header->Crypto.PrimaryKey.KeKeyId.IdSize == 0) {
		return NXL_BAD_KEK_ID_SIZE;
	}

	if(header->Sections.Count < 3) {
		return NXL_BAD_SECTION_COUNT;
	}

	return NXL_OK;
}

bool CFile::FindSection(_In_ LPCSTR name, _Out_opt_ PULONG offset, _Out_opt_ PULONG size) const throw()
{
	ULONG cur_offset = NXL_SCNDATA_OFFSET;
	ULONG cur_size = 0;

	if(_header.empty() || _header.size() != sizeof(NXL_HEADER)) {
		return false;
	}

	PCNXL_HEADER header = (PCNXL_HEADER)&_header[0];

	// Find target section
	for(int i=0; i<(int)header->Sections.Count; i++) {
		// Not? Move to next
		if(0 != _stricmp(header->Sections.Sections[i].Name, name)) {
			cur_offset += header->Sections.Sections[i].Size;
			continue;
		}
		// Found? record offset and size
		cur_size = header->Sections.Sections[i].Size;
		break;
	}

	if(0 != cur_size) {
		if(NULL != offset) *offset = cur_offset;
		if(NULL != size) *size = cur_size;
		return true;
	}

	return false;
}


void CFile::GetNPairDataW(_In_ LPCSTR name, _Out_ std::vector<std::pair<std::wstring, std::wstring>>& data)
{
	if(!IsOpened()) {
		throw WIN32ERROR2(ERROR_INVALID_HANDLE);
	}

	assert(_header.size() == sizeof(NXL_HEADER));
	PCNXL_HEADER header = (PCNXL_HEADER)&_header[0];
	ULONG        offset = NXL_SCNDATA_OFFSET;
	ULONG        size = 0;
	DWORD        dwRead = 0;

	// Find target section
	if(!FindSection(name, &offset, &size)) {
		throw WIN32ERROR2(ERROR_NOT_FOUND);
	}

	std::vector<WCHAR> buf;
	buf.resize((size+1)/sizeof(WCHAR), 0);
	::SetFilePointer(_handle, offset, NULL, FILE_BEGIN);
	if(!::ReadFile(_handle, &buf[0], size, &dwRead, NULL)) {
		throw WIN32ERROR();
	}
	

	LPCWSTR record = &buf[0];
	while(L'\0' != record[0]) {
		std::wstring spair = record;
		record += (spair.length()+1);

		std::wstring::size_type pos = spair.find_first_of(L'=');
		if(pos == std::wstring::npos) {
			continue;
		}

		std::wstring name = spair.substr(0, pos);
		std::wstring value = spair.substr(pos+1);
		if(!name.empty() && !value.empty()) {
			data.push_back(std::pair<std::wstring, std::wstring>(name, value));
		}
	}
}

void CFile::SetNPairDataW(_In_ LPCSTR name, _In_ const std::vector<std::pair<std::wstring, std::wstring>>& data, _Out_opt_ PULONG paires_written)
{
	if(!IsOpened()) {
		throw WIN32ERROR2(ERROR_INVALID_HANDLE);
	}

	assert(_header.size() == sizeof(NXL_HEADER));
	PCNXL_HEADER header = (PCNXL_HEADER)&_header[0];
	ULONG        offset = NXL_SCNDATA_OFFSET;
	ULONG        size = 0;
	DWORD        dwWritten = 0;
	ULONG        count = 0;

	// Find target section
	if(!FindSection(name, &offset, &size)) {
		throw WIN32ERROR2(ERROR_NOT_FOUND);
	}

	std::vector<WCHAR> buf;
	size = (size+1)/sizeof(WCHAR);
	buf.resize(size, 0);

	LPWSTR record = &buf[0];
	for(ULONG i=0; i<(ULONG)data.size(); i++) {

		if(size <= 2) {
			break;
		}

		std::wstring wspair = data[i].first;
		wspair += L"=";
		wspair += data[i].second;
		if(!wspair.empty()) {

			if(size < (wspair.length()+2)) {  // Have enough space to hold last two NULLs
				break;
			}

			// Copy & Move to Next
			memcpy(record, wspair.c_str(), wspair.length()*sizeof(WCHAR));
			record += (wspair.length() + 1);
			size -= ((ULONG)wspair.length() + 1);
			count ++;
		}
	}
	
	::SetFilePointer(_handle, offset, NULL, FILE_BEGIN);
	if(!::WriteFile(_handle, &buf[0], (DWORD)buf.size(), &dwWritten, NULL)) {
		throw WIN32ERROR();
	}

	if(NULL != paires_written) {
		*paires_written = count;
	}
}

void CFile::GetNPairDataUtf8(_In_ LPCSTR name, _Out_ std::vector<std::pair<std::wstring, std::wstring>>& data)
{
	if(!IsOpened()) {
		throw WIN32ERROR2(ERROR_INVALID_HANDLE);
	}

	assert(_header.size() == sizeof(NXL_HEADER));
	PCNXL_HEADER header = (PCNXL_HEADER)&_header[0];
	ULONG        offset = NXL_SCNDATA_OFFSET;
	ULONG        size = 0;
	DWORD        dwRead = 0;

	// Find target section
	if(!FindSection(name, &offset, &size)) {
		throw WIN32ERROR2(ERROR_NOT_FOUND);
	}

	std::vector<CHAR> buf;
	buf.resize(size, 0);
	::SetFilePointer(_handle, offset, NULL, FILE_BEGIN);
	if(!::ReadFile(_handle, &buf[0], size, &dwRead, NULL)) {
		throw WIN32ERROR();
	}
	

	LPCSTR record = &buf[0];
	while(L'\0' != record[0]) {
		std::string spair = record;
		record += (spair.length()+1);

		std::string::size_type pos = spair.find_first_of("=");
		if(pos == std::string::npos) {
			continue;
		}

		std::string name = spair.substr(0, pos);
		std::string value = spair.substr(pos+1);
		if(!name.empty() && !value.empty()) {
			std::wstring name16 = nudf::util::convert::Utf8ToUtf16(name);
			std::wstring value16 = nudf::util::convert::Utf8ToUtf16(value);
			if(!name16.empty() && !value16.empty()) {
				data.push_back(std::pair<std::wstring, std::wstring>(name16, value16));
			}
		}
	}
}

void CFile::SetNPairDataUtf8(_In_ LPCSTR name, _In_ const std::vector<std::pair<std::wstring, std::wstring>>& data, _Out_opt_ PULONG paires_written)
{
	if(!IsOpened()) {
		throw WIN32ERROR2(ERROR_INVALID_HANDLE);
	}

	assert(_header.size() == sizeof(NXL_HEADER));
	PCNXL_HEADER header = (PCNXL_HEADER)&_header[0];
	ULONG        offset = NXL_SCNDATA_OFFSET;
	ULONG        size = 0;
	DWORD        dwWritten = 0;
	ULONG        count = 0;

	// Find target section
	if(!FindSection(name, &offset, &size)) {
		throw WIN32ERROR2(ERROR_NOT_FOUND);
	}

	std::vector<CHAR> buf;
	buf.resize(size, 0);

	LPSTR record = &buf[0];
	for(ULONG i=0; i<(ULONG)data.size(); i++) {

		if(size <= 2) {
			break;
		}

		std::wstring wspair = data[i].first;
		wspair += L"=";
		wspair += data[i].second;
		std::string spair = nudf::util::convert::Utf16ToUtf8(wspair);
		if(!spair.empty()) {

			if(size < (spair.length()+2)) {  // Have enough space to hold last two NULLs
				break;
			}

			// Copy & Move to Next
			memcpy(record, spair.c_str(), spair.length());
			record += (spair.length() + 1);
			size -= ((ULONG)spair.length() + 1);
			count ++;
		}
	}

	if(!::WriteFile(_handle, &buf[0], (DWORD)buf.size(), &dwWritten, NULL)) {
		throw WIN32ERROR();
	}

	if(NULL != paires_written) {
		*paires_written = count;
	}
}

HRESULT __stdcall nudf::util::nxl::NxrmCheckRightsNoneCache(const WCHAR *FileName, ULONGLONG *RightMask, ULONGLONG *CustomRightsMask, ULONGLONG *EvaluationId)
{
	HRESULT hr = S_OK;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	UCHAR EaBuf[128] = { 0 };
	ULONG EaInputLength = 0;
	ULONG EaOutputLength = 0;

	UCHAR EaValue = 0;

	FILE_GET_EA_INFORMATION *pEaInfo = NULL;
	FILE_FULL_EA_INFORMATION *pFullEaInfo = NULL;

	ZWQUERYEAFILE fn_ZwQueryEaFile = NULL;

	NTSTATUS Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	IoStatus = { 0 };

	ULONGLONG *p = NULL;

	fn_ZwQueryEaFile = (ZWQUERYEAFILE)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwQueryEaFile");

	do
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_IS_CONTENT_ENCRYPTED);
		memcpy(pEaInfo->EaName, NXRM_EA_IS_CONTENT_ENCRYPTED, sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_IS_CONTENT_ENCRYPTED) + sizeof(UCHAR);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != sizeof(UCHAR))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		EaValue = *((UCHAR*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1));

		if (EaValue != NXRM_CONTENT_IS_ENCRYPTED)
		{
			hr = E_UNEXPECTED;
			break;
		}

		//
		//
		//
		memset(EaBuf, 0, sizeof(EaBuf));

		pEaInfo = (FILE_GET_EA_INFORMATION *)EaBuf;
		pEaInfo->NextEntryOffset = 0;
		pEaInfo->EaNameLength = (UCHAR)strlen(NXRM_EA_CHECK_RIGHTS_NONECACHE);
		memcpy(pEaInfo->EaName, NXRM_EA_CHECK_RIGHTS_NONECACHE, sizeof(NXRM_EA_CHECK_RIGHTS_NONECACHE));

		EaInputLength = sizeof(FILE_GET_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_CHECK_RIGHTS_NONECACHE);
		EaOutputLength = sizeof(FILE_FULL_EA_INFORMATION) - sizeof(CHAR) + sizeof(NXRM_EA_CHECK_RIGHTS_NONECACHE) + sizeof(ULONGLONG) + sizeof(ULONGLONG) + sizeof(ULONGLONG);

		Status = fn_ZwQueryEaFile(hFile,
								  &IoStatus,
								  EaBuf,
								  EaOutputLength,
								  TRUE,
								  EaBuf,
								  EaInputLength,
								  NULL,
								  TRUE);

		if (Status != STATUS_SUCCESS)
		{
			hr = HRESULT_FROM_NT(Status);
			break;
		}

		pFullEaInfo = (FILE_FULL_EA_INFORMATION *)EaBuf;

		if (pFullEaInfo->EaValueLength != (sizeof(ULONGLONG) + sizeof(ULONGLONG) + sizeof(ULONGLONG)))
		{
			//
			// nxrmflt.sys is not running or rights management service is not running
			//
			hr = E_UNEXPECTED;
			break;
		}

		p = (ULONGLONG*)(pFullEaInfo->EaName + pFullEaInfo->EaNameLength + 1);

		*RightMask = *(p + 0);
		*CustomRightsMask = *(p + 1);
		*EvaluationId = *(p + 2);

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}
