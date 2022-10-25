

#include <Windows.h>
#include <stdio.h>


#include <nudf\exception.hpp>
#include <nudf\cert.hpp>
#include <nudf\nxlutilex.hpp>
#include <nudf\string.hpp>
#include <nudf\time.hpp>
#include <nudf\convert.hpp>
#include <nudf\encoding.hpp>
#include <nudf\crypto.hpp>



void usage(const wchar_t* app)
{
    printf("NXLViewer Usage:\n");
    printf("   %S [/k KEY] </f file>\n", app);
}

void test1()
{
    unsigned long checksum = 0x2C6DBE66;
    unsigned char buf[16];
    const unsigned char key[16] = {0x26, 0xDB, 0x01, 0x49, 0xA9, 0x31, 0x43, 0x40, 0xC0, 0x85, 0x38, 0x5D, 0x9B, 0x0B, 0xC1, 0x1E };
    const unsigned char cdata[16] = { 0xAE, 0xEF, 0xBC, 0x8A, 0xC5, 0xAB, 0xC6, 0xD5, 0x76, 0xDC, 0x35, 0x16, 0x1B, 0x37, 0xBA, 0x09 };

    memset(buf, 0, 16);
    memcpy(buf, &checksum, 4);

    nudf::crypto::Initialize();
    nudf::crypto::CAesKeyBlob kb;
    kb.SetKey(key, 16);
    std::vector<unsigned char> cipher;
    printf("\n");
    printf("original buf: ");
    std::for_each(buf, buf+16, [](const unsigned char c) {
        printf("%02X", c);
    });
    printf("\n");
    nudf::crypto::AesEncrypt(kb, buf, 16, 0, cipher);
    printf("encrypted buf: ");
    std::for_each(cipher.begin(), cipher.end(), [](const unsigned char c) {
        printf("%02X", c);
    });
    printf("\n");

    std::vector<unsigned char> plain;
    printf("\n");
    printf("original buf: ");
    std::for_each(cdata, cdata +16, [](const unsigned char c) {
        printf("%02X", c);
    });
    printf("\n");
    nudf::crypto::AesDecrypt(kb, cdata, 16, 0, plain);
    printf("decrypted buf: ");
    std::for_each(plain.begin(), plain.end(), [](const unsigned char c) {
        printf("%02X", c);
    });
    printf("\n");
}

int wmain(int argc, wchar_t** argv)
{
    test1();
    return 0;

    if(argc < 2) {
        printf("NXLViewer Usage:\n");
        printf("   %S [/k KEY] </f file>\n", argv[0]);
        return -1;
    }


    LPCWSTR file;
    std::vector<UCHAR> key;

    nudf::util::encoding::Base64Decode<char>("E4LAo5YnkPRyIohFS7ewk5PcKFbi4wW0NglieQFRBGU=", key);
    if(!key.empty()) {
        std::string sKey = nudf::string::FromBytes<char>(&key[0], (ULONG)key.size());
        printf("Default Key: %s\n", sKey.c_str());
    }

    if(argc == 3) {
        if(0 != _wcsicmp(argv[1], L"/f")) {
            usage(argv[0]);
            return -1;
        }
        file = argv[2];
    }
    else if(argc == 3) {
        // 1st
        if(0 != _wcsicmp(argv[1], L"/k")) {
            nudf::string::ToBytes<wchar_t>(argv[2], key);
        }
        else if(0 != _wcsicmp(argv[1], L"/f")) {
            file = argv[2];
        }
        else {
            usage(argv[0]);
            return -1;
        }
        // 2nd
        if(0 != _wcsicmp(argv[3], L"/k")) {
            nudf::string::ToBytes<wchar_t>(argv[4], key);
        }
        else if(0 != _wcsicmp(argv[3], L"/f")) {
            file = argv[4];
        }
        else {
            usage(argv[0]);
            return -1;
        }
    }
    else {
        usage(argv[0]);
        return -1;
    }

    if(NULL==file || L'\0'==file[0] || INVALID_FILE_ATTRIBUTES==GetFileAttributesW(file)) {
        printf("File not found (%S)\n", file?file:L"");
        return -1;
    }

    nudf::util::nxl::NXLSTATUS status = nudf::util::nxl::NXL_UNKNOWN;

    if(!nudf::crypto::Initialize()) {
        printf("Fail to initialize crypto library (%08X)\n", GetLastError());
        return -1;
    }

    try {

        nudf::util::nxl::CFileEx nf(file);
        nf.OpenEx(true, &status);

        std::wstring wsThumbprint = nudf::string::FromBytes<wchar_t>(nf.GetNHeader()->Basic.Thumbprint, 16);


        printf("NXL File is opened.\n");
        printf("[Signatrue]\n");
        printf("  Magic:      %08X%08X\n", nf.GetNHeader()->Signature.Code.HighPart, nf.GetNHeader()->Signature.Code.LowPart);
        printf("  Message:    %S\n", nf.GetNHeader()->Signature.Message);
        printf("[Basic]\n");
        printf("  Thumbprint: %S\n", wsThumbprint.c_str());
        printf("  Version:    %d.%d\n", HIWORD(nf.GetNHeader()->Basic.Version), LOWORD(nf.GetNHeader()->Basic.Version));
        printf("  Flags:      %08X\n", nf.GetNHeader()->Basic.Flags);
        printf("  Alignment:  %d\n", nf.GetNHeader()->Basic.Alignment);
        printf("  Content:    Offset: %d(0x%08X) \n", nf.GetNHeader()->Basic.PointerOfContent, nf.GetNHeader()->Basic.PointerOfContent);
        printf("[Crypto]\n");
        printf("  Algorithm:  %s\n", (NXL_ALGORITHM_AES256 == nf.GetNHeader()->Crypto.Algorithm) ? "AES 256" : "AES 128");
        printf("  CBC Size:   %d\n", nf.GetNHeader()->Crypto.CbcSize);
        printf("  PrimaryKey:\n");
        printf("      Key Algorithm: %s\n", (NXL_ALGORITHM_AES256 == nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.Algorithm) ? "AES 256" : "AES 128");
        const ULONG expect_key_size = (NXL_ALGORITHM_AES256 == nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.Algorithm) ? 32 : 16;
        if(!key.empty() && key.size() != expect_key_size) {
            key.clear();
            printf("          WARN: Input key size doesn't match algorithm\n");
        }
        std::wstring wsKeyId = nudf::string::FromBytes<wchar_t>(nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.Id, nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.IdSize);
        printf("      Key Id: %S\n",wsKeyId.c_str());
        if(44 == nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.IdSize) {
            // Nextlabs Key
            CHAR szKeyRing[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
            LONG dosTime = 0;
            SYSTEMTIME st = {0};
            std::wstring wsKeyHash = nudf::string::FromBytes<wchar_t>(&nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.Id[8], 32);
            memcpy(szKeyRing, nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.Id, 8);
            memcpy(&dosTime, &nf.GetNHeader()->Crypto.PrimaryKey.KeKeyId.Id[40], 4);
            nudf::time::CTime t;
            t.FromSecondsSince1970Jan1st(dosTime);
            t.ToSystemTime(&st);
            printf("      Key Ring: %s\n", szKeyRing);
            printf("      Key Hash: %S\n", wsKeyHash.c_str());
            printf("      Key Time: %04d-%02d-%02d %02d:%02d:%02d (%08X)\n", st.wYear, st.wMonth, st.wDay, st.wHour,st.wMinute,st.wSecond, dosTime);
        }

        std::string sChecksum = nudf::string::FromBytes<char>(nf.GetNHeader()->Sections.Checksum, 16);
        printf("[Section]\n");
        printf("  Checksum:  %s\n", sChecksum.c_str());
        if(!key.empty()) {
            ULONG crc_in_table = 0;
            ULONG crc_expect = 0;
            std::vector<UCHAR> cekey;
            if(nf.DecodeContentKey(key, cekey) && cekey.size()>=16) {
                if(nf.VerifySectionTableChecksum(&cekey[0], &crc_in_table, &crc_expect)) {
                    printf("      - VERIFIED\n");
                }
                else {
                    printf("      - ERROR: Invalid checksum (InTable: %08X, Expect: %08X)\n", crc_in_table, crc_expect);
                }
            }
            else {
                printf("      - ERROR: Fail to decode content key\n");
            }
        }
        else {
            printf("      - NOT VERIFIED\n");
        }
        printf("  Count:     %d\n", nf.GetNHeader()->Sections.Count);
        
#ifdef _DEBUG
        std::vector<UCHAR> xbuf;
        xbuf.resize(2048, 0);
        ULONG xcrc = nudf::crypto::ToCrc32(0, &xbuf[0], 2048);
        printf("  Empty Checksum (2K):  %08X\n", xcrc);
        xbuf.resize(4096, 0);
        xcrc = nudf::crypto::ToCrc32(0, &xbuf[0], 4096);
        printf("  Empty Checksum (4K):  %08X\n", xcrc);
#endif

        ULONG scdataoff = (ULONG)sizeof(NXL_HEADER);
        for(int i=0; i<(int)nf.GetNHeader()->Sections.Count; i++) {
            ULONG checksum = 0;
            bool verified = nf.VerifySectionChecksum(i, NULL, &checksum);
            printf("  %02d: %s\n", i, nf.GetNHeader()->Sections.Sections[i].Name);
            printf("      size: %d (%08X)\n", nf.GetNHeader()->Sections.Sections[i].Size, nf.GetNHeader()->Sections.Sections[i].Size);
            if(verified) {
                printf("      crc:  %08X (VERIFIED)\n", nf.GetNHeader()->Sections.Sections[i].Checksum);
            }
            else {
                printf("      crc:  %08X (ERROR: expect %08X)\n", nf.GetNHeader()->Sections.Sections[i].Checksum, checksum);
            }
            std::vector<std::pair<std::wstring, std::wstring>> pairdata;
            if(0 == i) {
                nf.GetNAttributes(pairdata);
            }
            else if(i == 2) {
                nf.GetNTags(pairdata);
            }
            if(!pairdata.empty()) {
                printf("      data: %d\n", pairdata.size());
                for(int i=0; i<(int)pairdata.size(); i++) {
                    printf("          %S = %S\n", pairdata[i].first.c_str(), pairdata[i].second.c_str());
                }
            }
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        printf("Invalid NXL file (status=%d)", status);
    }

    return 0;
}