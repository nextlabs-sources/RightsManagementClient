

#include <Windows.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include <nudf\exception.hpp>
#include <nudf\nxlutil.hpp>
#include <nudf\convert.hpp>
#include <nudf\user.hpp>
#include <nudf\string.hpp>
#include <nudf\asyncpipe.hpp>
#include <nudf\web\json.hpp>
#include <nudf\nxrmuihlp.hpp>

#include "rightsbox.hpp"
#include "dlgclassify.hpp"
#include "dlgclassify2.hpp"

HINSTANCE _hInstance = NULL;


BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD dwReason, _In_ LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        _hInstance = hInstance;
        ::DisableThreadLibraryCalls((HMODULE)hInstance);
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}


BOOL WINAPI RmuInitialize()
{
    return TRUE;
}

VOID WINAPI RmuDestroy()
{
    return;
}

LONG WINAPI RmuShowAllPropPages(_In_opt_ HWND hParent, _In_ LPCWSTR file, _In_ PCNXL_HEADER header, _In_ ULONGLONG rights, _In_opt_ LPCWSTR pwzClassifyData)
{
    CRightsBox rb;
    std::vector<std::pair<std::wstring,std::wstring>> classifydata;

    if(NULL != pwzClassifyData) {
        nudf::util::convert::MultiStringsToPairVector<wchar_t>(pwzClassifyData, classifydata, L'=');
    }

    return rb.ShowAll(hParent, file, header, rights, classifydata);
}

LONG WINAPI RmuShowGeneralPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ PCNXL_HEADER pHeader)
{
    CRightsBox rb;
    return rb.ShowGeneral(hParent, wzFile, pHeader);
}

LONG WINAPI RmuShowPermissionPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ const ULONGLONG uRrights, _Out_opt_ PULONGLONG puNewRights)
{
    CRightsBox rb;
    LONG lResult = rb.ShowPermission(hParent, wzFile, uRrights);
    if(-1 != lResult && NULL != puNewRights) {
        *puNewRights = rb.GetPermissionPage().GetRights();
    }
    return lResult;
}

LONG WINAPI RmuShowClassifyPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_opt_ LPCWSTR pwzClassifyData)
{
    CRightsBox rb;
    std::vector<std::pair<std::wstring,std::wstring>> classifydata;

    if(NULL != pwzClassifyData) {
        nudf::util::convert::MultiStringsToPairVector<wchar_t>(pwzClassifyData, classifydata, L'=');
    }

    return rb.ShowClassification(hParent, wzFile, classifydata);
}

LONG WINAPI RmuShowDetailsPropPage(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ const ULONGLONG uRrights, _In_opt_ LPCWSTR pwzClassifyData)
{
    CRightsBox rb;
    std::vector<std::pair<std::wstring,std::wstring>> classifydata;

    if(NULL != pwzClassifyData) {
        nudf::util::convert::MultiStringsToPairVector<wchar_t>(pwzClassifyData, classifydata, L'=');
    }

    return rb.ShowDetails(hParent, wzFile, uRrights, classifydata);
}

LONG WINAPI RmuShowAllPropPagesSimple(_In_opt_ HWND hParent, _In_ LPCWSTR file, _In_ ULONGLONG rights)
{
    CRightsBox rb;
    std::vector<std::pair<std::wstring, std::wstring>> tags;

    nudf::util::nxl::CFile nf(file);
    nudf::util::nxl::NXLSTATUS status = nudf::util::nxl::NXL_UNKNOWN;

    try {
        nf.Open(&status);
        nf.GetNTags(tags);
        nf.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        nf.Reset();
    }

    return rb.ShowAll(hParent, file, nf.GetNHeader(), rights, tags);
}

LONG WINAPI RmuShowGeneralPropPageSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile)
{
    CRightsBox rb;
    nudf::util::nxl::CFile nf(wzFile);
    nudf::util::nxl::NXLSTATUS status = nudf::util::nxl::NXL_UNKNOWN;

    try {
        nf.Open(&status);
        nf.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        nf.Reset();
    }

    return rb.ShowGeneral(hParent, wzFile, nf.GetNHeader());
}

LONG WINAPI RmuShowClassifyPropPageSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile)
{
    CRightsBox rb;
    std::vector<std::pair<std::wstring, std::wstring>> tags;

    nudf::util::nxl::CFile nf(wzFile);
    nudf::util::nxl::NXLSTATUS status = nudf::util::nxl::NXL_UNKNOWN;

    try {
        nf.Open(&status);
        nf.GetNTags(tags);
        nf.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        nf.Reset();
    }

    return rb.ShowClassification(hParent, wzFile, tags);
}

LONG WINAPI RmuShowDetailsPropPageSimple(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ const ULONGLONG uRrights)
{
    CRightsBox rb;
    std::vector<std::pair<std::wstring, std::wstring>> tags;

    nudf::util::nxl::CFile nf(wzFile);
    nudf::util::nxl::NXLSTATUS status = nudf::util::nxl::NXL_UNKNOWN;

    try {
        nf.Open(&status);
        nf.GetNTags(tags);
        nf.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        nf.Reset();
    }

    return rb.ShowDetails(hParent, wzFile, uRrights, tags);
}

VOID WINAPI RmuFreeResource(_In_ PVOID mem)
{
    if(NULL != mem) {
        free(mem);
    }
}

LONG WINAPI RmuShowClassifyDialog2(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzXml, _Out_ LPWSTR* ppwzClassifyTags)
{
    CDlgClassify dlg(wzFile, wzXml);
    std::vector<std::pair<std::wstring, std::wstring>> tags;

    *ppwzClassifyTags = NULL;

    int nResult = dlg.DoModal(hParent);
    if(IDOK == nResult || IDYES == nResult) {

        dlg.GetClassificationTags(tags);
        if(tags.size() != 0) {
            // Calculate buffer
            std::vector<std::pair<std::wstring, std::wstring>>::const_iterator it;
            size_t cchcount = 0;
            WCHAR* pwzItem = NULL;
            size_t nItem = 0;
            for(it=tags.begin(); it!=tags.end(); ++it) {
                cchcount += ((*it).first.length() + 1 + (*it).second.length() + 1);
            }
            cchcount += 1; // end with double '\0'

            *ppwzClassifyTags = (LPWSTR)malloc(cchcount*sizeof(WCHAR));
            if(NULL == *ppwzClassifyTags) {
                return ERROR_OUTOFMEMORY;
            }

            pwzItem = *ppwzClassifyTags;
            nItem = cchcount;
            memset(pwzItem, 0, nItem*sizeof(WCHAR));

            for(it=tags.begin(); it!=tags.end(); ++it) {
                std::wstring wsItem = (*it).first + L"=" + (*it).second;
                memcpy(pwzItem, wsItem.c_str(), wsItem.length()*sizeof(WCHAR));
                // Move to next
                pwzItem += (wsItem.length() + 1);
                nItem   -= (wsItem.length() + 1);
            }
        }
    }
    else if(IDCANCEL == nResult || IDABORT == nResult || IDNO == nResult || IDCLOSE==nResult) {
        return ERROR_CANCELLED;
    }
    else {
        return ERROR_INTERNAL_ERROR;
    }

    return 0L;
}

LONG WINAPI RmuShowClassifyDialogEx2(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzXmlFile, _In_ ULONG langId, _Out_ LPWSTR* ppwzClassifyTags)
{
    LONG        lRet = 0;

    try {

        CLabelConf  conf;
        CComPtr<IXMLDOMNode> spLabels;
        WCHAR       wzLocal[MAX_PATH] = {0};

        conf.LoadFromFile(wzXmlFile);
        
        // Get Locale Name
        LCIDToLocaleName(MAKELCID(langId, SORT_DEFAULT), wzLocal, MAX_PATH, 0);

        // Load
        if(!conf.FindLabelGroup(wzLocal, &spLabels)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        CComBSTR bstrXml;
        HRESULT hr = spLabels->get_xml(&bstrXml);
        if(FAILED(hr) || NULL==bstrXml.m_str) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        std::wstring xml = bstrXml.m_str;
        if(xml.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        spLabels.Release();
        conf.Close();

        lRet = RmuShowClassifyDialog2(hParent, wzFile, xml.c_str(), ppwzClassifyTags);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

LONG WINAPI RmuShowClassifyDialog(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzJson, _In_opt_ LPCWSTR wzGroupName, _In_opt_ LPCWSTR pwzInittags, _In_ BOOL ReadOnly, _Out_ LPWSTR* ppwzClassifyTags)
{
    std::wstring group(wzGroupName?wzGroupName:L"default");
    CDlgClassify2 dlg(wzFile, wzJson, group);
    std::vector<std::pair<std::wstring, std::wstring>> tags;
    std::vector<std::pair<std::wstring, std::wstring>> init_tags;

    *ppwzClassifyTags = NULL;

    // Is a readOnly Dialog?
    dlg.SetReadOnly(ReadOnly?true:false);

    if(NULL != pwzInittags) {
        nudf::util::CRmuObject::BufferToPair(pwzInittags, init_tags);
    }

    if(!init_tags.empty()) {
        dlg.SetInitialData(init_tags);
    }


    int nResult = dlg.DoModal(hParent);
    if(IDOK == nResult || IDYES == nResult) {

        dlg.GetClassificationTags(tags);
        if(!tags.empty()) {
            std::vector<WCHAR> vTagsBuf;
            nudf::util::CRmuObject::PairToBuffer(tags, vTagsBuf);
            if(!vTagsBuf.empty()) {
                *ppwzClassifyTags = (LPWSTR)malloc(vTagsBuf.size()*sizeof(WCHAR));
                if(NULL == *ppwzClassifyTags) {
                    return ERROR_OUTOFMEMORY;
                }
                memcpy(*ppwzClassifyTags, &vTagsBuf[0], vTagsBuf.size()*sizeof(WCHAR));
            }
        }
    }
    else if(IDCANCEL == nResult || IDABORT == nResult || IDNO == nResult || IDCLOSE==nResult) {
        return ERROR_CANCELLED;
    }
    else {
        return ERROR_INTERNAL_ERROR;
    }

    return 0L;
}

LONG WINAPI RmuShowClassifyDialogEx(_In_opt_ HWND hParent, _In_ LPCWSTR wzFile, _In_ LPCWSTR wzJsonFile, _In_opt_ LPCWSTR wzGroupName, _In_ ULONG langId, _In_opt_ LPCWSTR pwzInittags, _In_ BOOL ReadOnly, _Out_ LPWSTR* ppwzClassifyTags)
{
    LONG        lRet = 0;

    try {

        std::ifstream fs;
        fs.open(wzJsonFile, std::ifstream::binary);
        if (!fs.is_open() || !fs.good()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // get length of file:
        fs.seekg(0, fs.end);
        size_t length = (size_t)fs.tellg();
        fs.seekg(0, fs.beg);

        std::vector<char> buf;
        buf.resize(length + 1, 0);
        fs.read(&buf[0], length);
        if (fs.eof() || fs.bad() || fs.fail()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        fs.close();

        std::wstring ws = NX::utility::conversions::utf8_to_utf16(buf.data());
        
        lRet = RmuShowClassifyDialog(hParent, wzFile, ws.c_str(), wzGroupName, pwzInittags, ReadOnly, ppwzClassifyTags);
    }
    catch(const nudf::CException& e) {
        lRet = e.GetCode();
    }

    return lRet;
}

void get_logon_user(std::wstring& user_name, std::wstring& user_id)
{
    std::wstring logon_user_name;
    std::wstring logon_user_id;

    NX::async_pipe::client c(4096);

    try {
        unsigned long session_id = 0;
        if (!ProcessIdToSessionId(GetCurrentProcessId(), &session_id)) {
            throw std::exception("fail to get session");
        }

        std::wstring sport;
        std::wstringstream ss;
        ss << L"nxrmtray-" << (int)session_id;
        sport = ss.str();

        if (c.connect(sport)) {
            NX::web::json::value v = NX::web::json::value::object();
            v[L"code"] = NX::web::json::value::string(L"query_logon_user");
            std::wstring ws = v.serialize();
            std::string s = NX::utility::conversions::utf16_to_utf8(ws);
            std::vector<unsigned char> data;
            data.resize(s.length() + 1, 0);
            memcpy(&data[0], s.c_str(), s.length());
            c.write(data);
            data.clear();
            if (c.read(data) && !data.empty()) {
                s = (const char*)data.data();
                ws = NX::utility::conversions::utf8_to_utf16(s);
                NX::web::json::value response = NX::web::json::value::parse(ws);
                if (response.is_object() && response.has_field(L"result")) {
                    if (response.has_field(L"user_name")) {
                        user_name = response[L"user_name"].as_string();
                    }
                    if (response.has_field(L"user_id")) {
                        user_id = response[L"user_id"].as_string();
                    }
                }
            }
            c.disconnect();
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}