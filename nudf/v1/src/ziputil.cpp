

#include <Windows.h>
#include <Shldisp.h>
#include <AccCtrl.h>
#include <Aclapi.h>

#include <atlbase.h>
#include <atlcomcli.h>

#include <nudf\exception.hpp>
#include <nudf\ziputil.hpp>

using namespace nudf::util;



CZip::CZip()
{
}

CZip::~CZip()
{
}

bool CZip::Zip(_In_ const std::wstring& source, _In_ const std::wstring& zipfile, _In_opt_ LPSECURITY_ATTRIBUTES sa)
{
    bool result = false;

	CoInitialize(NULL);

    try {

        HRESULT hr;
        CComPtr<IShellDispatch> spISD;
        CComPtr<Folder>         spToFolder;
        CComVariant vSource;
        CComVariant vZipFile;
        CComVariant vOpt;
        
        if(!CreateEmptyZip(zipfile, sa)) {
            throw WIN32ERROR();
        }

        vZipFile.vt = VT_BSTR;
        vZipFile.bstrVal = ::SysAllocStringLen(NULL, (UINT)zipfile.length() + 3);
        RtlSecureZeroMemory(vZipFile.bstrVal, sizeof(WCHAR)*(zipfile.length() + 3));
        memcpy(vZipFile.bstrVal, zipfile.c_str(), sizeof(WCHAR)*zipfile.length());

        vSource.vt = VT_BSTR;
        vSource.bstrVal = ::SysAllocStringLen(NULL, (UINT)source.length() + 3);
        RtlSecureZeroMemory(vSource.bstrVal, sizeof(WCHAR)*(source.length() + 3));
        memcpy(vSource.bstrVal, source.c_str(), sizeof(WCHAR)*source.length());

        vOpt.vt = VT_I4;
        vOpt.lVal = 0x0614; //FOF_NO_UI;  //Do not display a progress dialog box, not useful in compression
        
        hr = ::CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&spISD);
        if (SUCCEEDED(hr) && NULL!=spISD.p) {

            // Destination is our zip file
            hr= spISD->NameSpace(vZipFile, &spToFolder);
            if (SUCCEEDED(hr) && NULL != spToFolder.p) {
                
                // Copying and compressing the source files to our zip
                hr = spToFolder->CopyHere(vSource, vOpt);
                result = SUCCEEDED(hr) ? true : false;

                // CopyHere() creates a separate thread to copy files and 
                // it may happen that the main thread exits before the 
                // copy thread is initialized. So we put the main thread to sleep 
                // for a second to give time for the copy thread to start.
                if(result) {
                    Sleep(1000);
                    // Done
                    spToFolder.Release();
                    (void)VerifyFileOpIsDone(zipfile);
                }
            }

            spISD.Release();
            result = true;
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        ::DeleteFileW(zipfile.c_str());
        result = false;
    }

    CoUninitialize();
    return result;
}

bool CZip::Unzip(_In_ const std::wstring& zipfile, _In_ const std::wstring& targetdir)
{
    bool result = false;

	CoInitialize(NULL);

    try {

        HRESULT hr;
        CComPtr<IShellDispatch> spISD;
        CComPtr<Folder>         spZipFolder;
        CComPtr<Folder>         spToFolder;
        CComVariant vTarget;
        CComVariant vZipFile;
        CComVariant vOpt;
        
        vZipFile.vt = VT_BSTR;
        vZipFile.bstrVal = ::SysAllocStringLen(NULL, (UINT)zipfile.length() + 3);
        RtlSecureZeroMemory(vZipFile.bstrVal, sizeof(WCHAR)*(zipfile.length() + 3));
        memcpy(vZipFile.bstrVal, zipfile.c_str(), sizeof(WCHAR)*zipfile.length());

        vTarget.vt = VT_BSTR;
        vTarget.bstrVal = ::SysAllocStringLen(NULL, (UINT)targetdir.length() + 3);
        RtlSecureZeroMemory(vTarget.bstrVal, sizeof(WCHAR)*(targetdir.length() + 3));
        memcpy(vTarget.bstrVal, targetdir.c_str(), sizeof(WCHAR)*targetdir.length());

        vOpt.vt = VT_I4;
        vOpt.lVal = 0x0614; //FOF_NO_UI;  //Do not display a progress dialog box, not useful in compression
        
        hr = ::CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&spISD);
        if (SUCCEEDED(hr) && NULL!=spISD.p) {

            // Source is our zip file
            hr= spISD->NameSpace(vZipFile, &spZipFolder);
            if (SUCCEEDED(hr) && NULL != spZipFolder.p) {

                CComPtr<FolderItems> spZipItems;

                hr = spZipFolder->Items(&spZipItems);
                if(SUCCEEDED(hr) && NULL != spZipItems) {

                    hr= spISD->NameSpace(vTarget, &spToFolder);
                    if (SUCCEEDED(hr) && NULL != spToFolder.p) {
                        // Creating a new Variant with pointer to FolderItems to be copied
                        VARIANT newV;
                        VariantInit(&newV);
                        newV.vt = VT_DISPATCH;
                        newV.pdispVal = spZipItems.p;
                        hr = spToFolder->CopyHere(newV, vOpt);
                        result = SUCCEEDED(hr) ? true : false;
                        // Done
                        if(result) {
                            Sleep(1000);
                        }

                        spToFolder.Release();
                    }

                    spZipItems.Release();
                }

                spZipFolder.Release();
            }

            spISD.Release();
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        ::DeleteFileW(zipfile.c_str());
        result = false;
    }

    CoUninitialize();
    return result;
}

bool CZip::CreateEmptyZip(_In_ const std::wstring& zipfile, _In_opt_ LPSECURITY_ATTRIBUTES sa)
{
    static const UCHAR emptyZip[] = {80, 75, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwWritten = 0;

    hFile = ::CreateFileW(zipfile.c_str(), GENERIC_READ|GENERIC_WRITE, 0, sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        return false;
    }

    if(!WriteFile(hFile, emptyZip, sizeof(emptyZip), &dwWritten, NULL)) {
        CloseHandle(hFile);
        ::DeleteFileW(zipfile.c_str());
        return false;
    }

    CloseHandle(hFile);
    return true;
}

bool CZip::VerifyFileOpIsDone(_In_ const std::wstring& file, _In_ DWORD dwWait)
{
    do {

        HANDLE h = ::CreateFileW(file.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE != h) {
            CloseHandle(h);
            return true;
        }
        // Failed
        if(ERROR_ACCESS_DENIED != GetLastError()) {
            return true;
        }

        // It is access denied, this might because the file is opened by another program
        // Wait and try again
        Sleep(200);
        if(INFINITE != dwWait) {
            dwWait = (dwWait > 200) ? (dwWait-200) : 0;
        }

    } while(0 != dwWait);

    return false;
}