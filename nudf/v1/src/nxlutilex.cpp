

#include <Windows.h>
#include <assert.h>
#include <winternl.h>
#include <nudf\shared\nxlfmt.h>
#include <nudf\shared\fltdef.h>
#include <nudf\exception.hpp>
#include <nudf\convert.hpp>
#include <nudf\crypto.hpp>
#include <nudf\nxlutilex.hpp>



using namespace nudf::util::nxl;


//
//  CFileEx
//

CFileEx::CFileEx() : CFile()
{
}

CFileEx::CFileEx(_In_ LPCWSTR path) : CFile(path)
{
}

CFileEx::~CFileEx()
{
}

bool CFileEx::DecodeContentKey(_In_ const std::vector<UCHAR>& kekey, _Out_ std::vector<UCHAR>& cekey)
{
	PCNXL_HEADER header = GetNHeader();
    if(header->Crypto.PrimaryKey.KeKeyId.Algorithm == NXL_ALGORITHM_AES128 && kekey.size() != 16) {
        return false;
    }
    if(header->Crypto.PrimaryKey.KeKeyId.Algorithm == NXL_ALGORITHM_AES256 && kekey.size() != 32) {
        return false;
    }

    nudf::crypto::CAesKeyBlob kekeyblob;
    kekeyblob.SetKey(&kekey[0], (ULONG)kekey.size());

    if(!nudf::crypto::AesDecrypt(kekeyblob, header->Crypto.PrimaryKey.CeKey, (header->Crypto.Algorithm==NXL_ALGORITHM_AES256)?32:16, 0, cekey)) {
        cekey.clear();
        return false;
    }

    return true;
}

bool CFileEx::VerifySectionChecksum(int index, _Out_opt_ PULONG crc_in_header, _Out_opt_ PULONG crc_calculated) throw()
{
	PCNXL_HEADER header = GetNHeader();
	if(NULL == header) {
		return false;
	}

    if(index >= (int)header->Sections.Count) {
        SetLastError(ERROR_NOT_FOUND);
        return false;
    }

	// Find target section's offset
	ULONG offset   = NXL_SCNDATA_OFFSET;
	for(int i=0; i<index; i++) {
        offset += header->Sections.Sections[i].Size;
	}

    std::vector<UCHAR> data;
    DWORD dwRead = 0;
    data.resize(header->Sections.Sections[index].Size, 0);
	::SetFilePointer(GetFileHandle(), offset, NULL, FILE_BEGIN);
	if(!::ReadFile(GetFileHandle(), &data[0], header->Sections.Sections[index].Size, &dwRead, NULL) || dwRead != header->Sections.Sections[index].Size) {
		return false;
	}

    ULONG checksum = nudf::crypto::ToCrc32(0, &data[0], (ULONG)data.size());
    if(NULL != crc_in_header) {
        *crc_in_header = header->Sections.Sections[index].Checksum;
    }
    if(NULL != crc_calculated) {
        *crc_calculated = checksum;
    }
    return (checksum == header->Sections.Sections[index].Checksum) ? true : false;
}

bool CFileEx::VerifySectionTableChecksum(_In_reads_(16) const UCHAR* key, _Out_opt_ PULONG crc_in_table, _Out_opt_ PULONG crc_calculated)
{
	PCNXL_HEADER header = GetNHeader();
	if(NULL == header) {
		return false;
	}

    ULONG checksum = nudf::crypto::ToCrc32(0, &header->Sections.Count, sizeof(ULONG));
    if(0 != header->Sections.Count) {
        checksum = nudf::crypto::ToCrc32(checksum, &header->Sections.Sections[0], sizeof(NXL_SECTION) * header->Sections.Count);
    }

    nudf::crypto::CAesKeyBlob keyblob;
    keyblob.SetKey(key, 16);
    std::vector<UCHAR> buf;
    if(!nudf::crypto::AesDecrypt(keyblob, header->Sections.Checksum, 16, 0, buf) || buf.size()!=16) {
        return false;
    }
    ULONG checksum2 = 0;
    memcpy(&checksum2, &buf[0], sizeof(ULONG));

    if(NULL != crc_in_table) {
        *crc_in_table = checksum2;
    }
    if(NULL != crc_calculated) {
        *crc_calculated = checksum;
    }
    return (checksum == checksum2) ? true : false;
}