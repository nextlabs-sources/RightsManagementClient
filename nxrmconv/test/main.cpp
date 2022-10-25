
#include <Windows.h>
#include <stdio.h>  
#include <stdlib.h>  

#include <string>
#include <vector>
#include <strstream>
#include <iostream>
#include <algorithm>

#include <boost\algorithm\string.hpp>


int get_tags(const std::wstring& file, std::vector<std::pair<std::wstring, std::wstring>>& tags)
{
    char   psBuffer[1024];
    FILE   *pPipe = NULL;
    std::wstring wsCmd = L"nxrmconv.exe readtag \"";

    wsCmd += file;
    wsCmd += L"\"";

    if ((pPipe = _wpopen(wsCmd.c_str(), L"rt")) == NULL)
        return 1;


    memset(psBuffer, 0, 1024);
    while (fgets(psBuffer, 1024, pPipe)) {
        WCHAR wzBuf[1024] = { 0 };
        memset(wzBuf, 0, 1024);
        if (0 == MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, psBuffer, -1, wzBuf, 1024)) {
            continue;
        }

        std::wstring s(wzBuf);
        boost::algorithm::trim(s);
        std::wstring::size_type pos = s.find(L'=');
        if (pos == std::wstring::npos) {
            continue;
        }

        std::wstring tag_name = s.substr(0, pos);
        std::wstring tag_value = s.substr(pos + 1);
        boost::algorithm::trim(tag_name);
        boost::algorithm::trim(tag_value);
        tags.push_back(std::pair<std::wstring, std::wstring>(tag_name, tag_value));
    }

    if (feof(pPipe))
    {
        _pclose(pPipe);
    }

    return 0;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc != 2)
    {
        std::cout << "Invalid parameter" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }

    std::vector<std::pair<std::wstring, std::wstring>> tags;

    int ret = get_tags(argv[1], tags);
    if (0 == ret)
    {
        std::for_each(tags.begin(), tags.end(), [](const std::pair<std::wstring, std::wstring>& it) {
            std::wcout << it.first << L" = " << it.second << std::endl;
        });
    }

    return ret;
}