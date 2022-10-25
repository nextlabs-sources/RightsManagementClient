

#include <Windows.h>
#include <assert.h>

#include <string>
#include <vector>


void Usage();
bool ParseParameters(_Out_ int* op, _Out_ bool* force, _Out_ std::wstring& source, _Out_ std::wstring& target, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags);
int  Encrypt(_In_ const std::wstring& source, _In_opt_ const std::wstring& target, _In_ bool force, _In_opt_ const std::vector<std::pair<std::wstring,std::wstring>>& tags);
int  Decrypt(_In_ const std::wstring& source, _In_opt_ const std::wstring& target, _In_ bool force);

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    int nRet = 0;
    int op = -1;
    bool force = false;
    std::wstring source;
    std::wstring target;
    std::vector<std::pair<std::wstring,std::wstring>> tags;

    if(!ParseParameters(&op, &force, source, target, tags)) {
        return ERROR_INVALID_PARAMETER;
    }

    assert(op==0 || op==1);

    switch(op)
    {
    case 0:
        // decrypt
        nRet =  Decrypt(source, target, force);
        break;

    case 1:
        // encrypt
        nRet =  Encrypt(source, target, force, tags);
        break;

    default:
        nRet = ERROR_INVALID_PARAMETER;
        break;
    }

    return nRet;
}

bool ParseParameters(_Out_ int* op, _Out_ bool* force, _Out_ std::wstring& source, _Out_ std::wstring& target, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
{
    LPWSTR*  pwzArglist;
    int      nArgs = 0;

    pwzArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if(NULL == pwzArglist) {
        return false;
    }

    // Init
    *op = -1;
    *force = false;
    source = L"";
    target = L"";
    tags.clear();

    for(int i=0; i<nArgs; i++) {

        // Parameter must start with '/'
        if(L'/' != pwzArglist[i][0]) {
            continue;
        }

        switch(pwzArglist[i][1])
        {
        case L'e':
        case L'E':
            // Flag indicating ENCRYPT operation
            // Mandatory
            // /e
            if(-1 != *op) {
                return false;
            }
            *op = 1;
            break;

        case L'd':
        case L'D':
            // Flag indicating DECRYPT operation
            // Mandatory
            // /d
            if(-1 != *op) {
                return false;
            }
            *op = 0;
            break;

        case L'f':
        case L'F':
            // Flag indicating FORCE operation
            // Optional
            // /f
            *force = true;
            break;

        case L'i':
        case L'I':
            // Flag indicating input file
            // Mandatory
            // /i"C:\test\itar-plain.docx"
            source = &pwzArglist[i][2];
            if(source.c_str()[0] == L'\"') {
                source = source.substr(1);
                if((source.c_str()[source.length()-1] == L'\"')) {
                    source = source.substr(0, source.length()-1);
                }
            }
            break;

        case L'o':
        case L'O':
            // Flag indicating output file
            // Optional
            // /o"C:\test\itar-plain.docx"
            target = &pwzArglist[i][2];
            if(target.c_str()[0] == L'\"') {
                target = target.substr(1);
                if((target.c_str()[target.length()-1] == L'\"')) {
                    target = target.substr(0, target.length()-1);
                }
            }
            break;

        case L't':
        case L'T':
            // Flag to set tags
            // Optional
            // /t"itar"="ITAR" /t"owner"="NextLabs, Inc."
            // Tag
            {
                std::wstring wsTag = &pwzArglist[i][2];
                std::wstring::size_type pos = wsTag.find(L'=');
                if(pos != std::wstring::npos) {
                    std::wstring wsName = wsTag.substr(0, pos);
                    std::wstring wsValue = wsTag.substr(pos+1);
                    if(wsName.c_str()[0] == L'\"') {
                        wsName = wsName.substr(1);
                        if((wsName.c_str()[wsName.length()-1] == L'\"')) {
                            wsName = wsName.substr(0, wsName.length()-1);
                        }
                    }
                    if(wsValue.c_str()[0] == L'\"') {
                        wsValue = wsValue.substr(1);
                        if((wsValue.c_str()[wsValue.length()-1] == L'\"')) {
                            wsValue = wsValue.substr(0, wsValue.length()-1);
                        }
                    }
                    if(!wsName.empty() && !wsValue.empty()) {
                        tags.push_back(std::pair<std::wstring,std::wstring>(wsName, wsValue));
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    if(*op == -1) {
        return false;
    }
    if(source.empty()) {
        return false;
    }

    return true;
}

int Encrypt(_In_ const std::wstring& source, _In_opt_ const std::wstring& target, _In_ bool force, _In_opt_ const std::vector<std::pair<std::wstring,std::wstring>>& tags)
{
    return 0;
}

int Decrypt(_In_ const std::wstring& source, _In_opt_ const std::wstring& target, _In_ bool force)
{
    return 0;
}