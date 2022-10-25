
#include <Windows.h>
#include <stdio.h>

#include <nudf\exception.hpp>
#include <nudf\nxlutil.hpp>

using namespace nudf::util;

int wmain(int argc, wchar_t** argv)
{
    int nRet = 0;
    std::wstring wsFile = argv[1];
    std::vector<std::pair<std::wstring, std::wstring>> new_tags;

    if(argc < 2) {
        printf("Usage:\n");
        printf("    tagtool.exe <file> [-i] [-p1] [-p2]\n");
    }

    wsFile = argv[1];

    for(int i=2; i<argc; i++) {
        if(0 == _wcsicmp(argv[i], L"-i")) {
            new_tags.push_back(std::pair<std::wstring, std::wstring>(L"itar", L"yes"));
        }
        else if(0 == _wcsicmp(argv[i], L"-p1")) {
            new_tags.push_back(std::pair<std::wstring, std::wstring>(L"project", L"pr-01"));
        }
        else if(0 == _wcsicmp(argv[i], L"-p2")) {
            new_tags.push_back(std::pair<std::wstring, std::wstring>(L"project", L"pr-02"));
        }
        else {
            printf("Usage:\n");
            printf("    tagtool.exe <file> [-i] [-p1] [-p2]\n");
            return -1;;
        }
    }

    try {
        nxl::CFile nf(wsFile.c_str());
        nxl::NXLSTATUS status;
        std::vector<std::pair<std::wstring, std::wstring>> tags;

        nf.OpenEx(false, &status);

        nf.GetNTags(tags);

        if(new_tags.empty()) {
            for(std::vector<std::pair<std::wstring, std::wstring>>::const_iterator it=tags.begin(); it!=tags.end(); ++it) {
                printf("  %S = %S\n", (*it).first.c_str(), (*it).second.c_str());
            }
        }
        else {
            ULONG count=  0;
            nf.SetNTags(new_tags, &count);
            printf("%d tags %s written\n", count, ((count > 1) ? "are" : "is"));
        }
    }
    catch(const nudf::CException& e) {
        printf("Exception: err=%d, function=%s, file=%s, line=%d\n", e.GetCode(), e.GetFunction(), e.GetFunction(), e.GetLine());
        nRet = e.GetCode();
    }

    return nRet;
}