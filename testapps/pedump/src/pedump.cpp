

#include <Windows.h>
#include <stdio.h>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\pe.hpp>


static bool decode(const std::wstring& pe_file);

int main(int argc, char** argv)
{
    while (true) {
        if (!decode(L"C:\\Program Files (x86)\\Microsoft Office\\Office15\\EXCEL.EXE")) {
            printf("Error! Do you want to stop? [Y/N]: ");
            char c = getchar();
            if (c == 'Y' || c == 'y') {
                break;
            }
        }
    }

    return 0;
}

bool decode(const std::wstring& pe_file)
{
    bool result = false;

    try {
        nudf::win::CPEFile pef;
        pef.Load(pe_file.c_str());
        result = pef.IsValid();
        //nudf::win::CPECert cert;
        //cert.Load(pe_file.c_str());
        //result = cert.IsValid();
        if (!result) {
            printf("Not a valid PE file\n");
        }
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }

    return result;

    //printf("PE Info\n");
    //printf("  .FileHeader\n");
    //printf("    - Machine: %s\n", (IMAGE_FILE_MACHINE_I386==pef.GetFileHeader()->Machine) ? "x86" : ((IMAGE_FILE_MACHINE_IA64==pef.GetFileHeader()->Machine) ? "IA64" : "x64"));
    //printf("    - NumberOfSections: %d\n", pef.GetFileHeader()->NumberOfSections);
    //printf("    - TimeDateStamp: 0x%08X\n", pef.GetFileHeader()->TimeDateStamp);
    //printf("    - PointerToSymbolTable: 0x%08X\n", pef.GetFileHeader()->PointerToSymbolTable);
    //printf("    - NumberOfSymbols: 0x%08X\n", pef.GetFileHeader()->NumberOfSymbols);
    //printf("    - SizeOfOptionalHeader: 0x%08X\n", pef.GetFileHeader()->SizeOfOptionalHeader);
    //printf("    - Characteristics: 0x%08X\n", pef.GetFileHeader()->Characteristics);
    /*        
    WORD        Magic;
    BYTE        MajorLinkerVersion;
    BYTE        MinorLinkerVersion;
    DWORD       SizeOfCode;
    DWORD       SizeOfInitializedData;
    DWORD       SizeOfUninitializedData;
    DWORD       AddressOfEntryPoint;
    DWORD       BaseOfCode;
    ULONGLONG   ImageBase;
    DWORD       SectionAlignment;
    DWORD       FileAlignment;
    WORD        MajorOperatingSystemVersion;
    WORD        MinorOperatingSystemVersion;
    WORD        MajorImageVersion;
    WORD        MinorImageVersion;
    WORD        MajorSubsystemVersion;
    WORD        MinorSubsystemVersion;
    DWORD       Win32VersionValue;
    DWORD       SizeOfImage;
    DWORD       SizeOfHeaders;
    DWORD       CheckSum;
    WORD        Subsystem;
    WORD        DllCharacteristics;
    ULONGLONG   SizeOfStackReserve;
    ULONGLONG   SizeOfStackCommit;
    ULONGLONG   SizeOfHeapReserve;
    ULONGLONG   SizeOfHeapCommit;
    DWORD       LoaderFlags;
    DWORD       NumberOfRvaAndSizes;
    */
    //printf("  .OptionalHeader\n");
    //printf("    - Magic: 0x%08X\n", pef.GetOptionalHeader()->Magic);
    //printf("    - CheckSum: 0x%08X\n", pef.GetOptionalHeader()->CheckSum);
    //printf("    - Subsystem: 0x%08X\n", pef.GetOptionalHeader()->Subsystem);
    //printf("    - SizeOfCode: 0x%08X\n", pef.GetOptionalHeader()->SizeOfCode);
    //printf("    - SizeOfInitializedData: 0x%08X\n", pef.GetOptionalHeader()->SizeOfInitializedData);
    //printf("    - SizeOfUninitializedData: 0x%08X\n", pef.GetOptionalHeader()->SizeOfUninitializedData);
    //printf("    - AddressOfEntryPoint: 0x%08X\n", pef.GetOptionalHeader()->AddressOfEntryPoint);
    //printf("    - BaseOfCode: 0x%08X\n", pef.GetOptionalHeader()->BaseOfCode);
    //printf("    - ImageBase: 0x%08X%08X\n", (ULONG)(pef.GetOptionalHeader()->ImageBase>>32), (ULONG)(pef.GetOptionalHeader()->ImageBase));
    //printf("    - SectionAlignment: 0x%08X\n", pef.GetOptionalHeader()->SectionAlignment);
    //printf("    - FileAlignment: 0x%08X\n", pef.GetOptionalHeader()->FileAlignment);
    //printf("    - Win32VersionValue: 0x%08X\n", pef.GetOptionalHeader()->Win32VersionValue);
    //printf("    - MajorLinkerVersion: %d\n", pef.GetOptionalHeader()->MajorLinkerVersion);
    //printf("    - MinorLinkerVersion: %d\n", pef.GetOptionalHeader()->MinorLinkerVersion);
    //printf("    - MajorOperatingSystemVersion: %d\n", pef.GetOptionalHeader()->MajorOperatingSystemVersion);
    //printf("    - MinorOperatingSystemVersion: %d\n", pef.GetOptionalHeader()->MinorOperatingSystemVersion);
    //printf("    - MajorImageVersion: %d\n", pef.GetOptionalHeader()->MajorImageVersion);
    //printf("    - MinorImageVersion: %d\n", pef.GetOptionalHeader()->MinorImageVersion);
    //printf("    - DllCharacteristics: 0x%08X\n", pef.GetOptionalHeader()->DllCharacteristics);
    //printf("    - SizeOfStackReserve: 0x%08X%08X\n", (ULONG)(pef.GetOptionalHeader()->SizeOfStackReserve>>32), (ULONG)(pef.GetOptionalHeader()->SizeOfStackReserve));
    //printf("    - SizeOfStackCommit: 0x%08X%08X\n", (ULONG)(pef.GetOptionalHeader()->SizeOfStackCommit>>32), (ULONG)(pef.GetOptionalHeader()->SizeOfStackCommit));
    //printf("    - SizeOfHeapReserve: 0x%08X%08X\n", (ULONG)(pef.GetOptionalHeader()->SizeOfHeapReserve>>32), (ULONG)(pef.GetOptionalHeader()->SizeOfHeapReserve));
    //printf("    - SizeOfHeapCommit: 0x%08X%08X\n", (ULONG)(pef.GetOptionalHeader()->SizeOfHeapCommit>>32), (ULONG)(pef.GetOptionalHeader()->SizeOfHeapCommit));
    //printf("    - LoaderFlags: 0x%08X\n", pef.GetOptionalHeader()->LoaderFlags);
    //printf("    - NumberOfRvaAndSizes: 0x%08X\n", pef.GetOptionalHeader()->NumberOfRvaAndSizes);
    //printf("  .Signature\n");
    //if(!pef.GetCert().IsValid()) {
    //    printf("    - N/A\n");
    //}
    //else {
    //    printf("    - Subject: %S\n", pef.GetCert().GetSubject().c_str());
    //    printf("    - Issuer: %S\n", pef.GetCert().GetIssuer().c_str());
    //    printf("    - Serial: %S\n", pef.GetCert().GetSerial().c_str());
    //    std::string thumbprint;
    //    if(!pef.GetCert().GetThumbprint().empty()) {
    //        thumbprint = nudf::string::FromBytes<char>(&pef.GetCert().GetThumbprint()[0], (ULONG)pef.GetCert().GetThumbprint().size());
    //    }
    //    printf("    - Thumbprint: %s\n", thumbprint.c_str());
    //    printf("    - ThumbprintAlg: %S\n", pef.GetCert().GetThumbprintAlg().c_str());
    //    printf("    - SigatureAlg: %S\n", pef.GetCert().GetSignatureAlg().c_str());
    //    const SYSTEMTIME& vf = pef.GetCert().GetValidFromDate();
    //    printf("    - ValidFrom: %04d-%02d-%02d %02d:%02d:%02d\n", vf.wYear, vf.wMonth, vf.wDay, vf.wHour, vf.wMinute, vf.wSecond);
    //    const SYSTEMTIME& vt = pef.GetCert().GetValidThruDate();
    //    printf("    - ValidThru: %04d-%02d-%02d %02d:%02d:%02d\n", vt.wYear, vt.wMonth, vt.wDay, vt.wHour, vt.wMinute, vt.wSecond);
    //}

    //return true;
}