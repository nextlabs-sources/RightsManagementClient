

#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <assert.h>

#include <list>

#include <nudf\exception.hpp>

#include "commonui.hpp"
#include "resource.h"
#include "dlgtemplate.hpp"
#include "dlgclassify2.hpp"



CDlgClassify2::CDlgClassify2() : CDlgTemplate(IDD_DIALOG_CLASSIFY2), _hIcon(NULL), _hFontTitle0(NULL), _hFontTitle(NULL), _readonly(false)
{
}

CDlgClassify2::CDlgClassify2(_In_ const std::wstring& file, _In_ const std::wstring& xml, _In_ const std::wstring& group) : CDlgTemplate(IDD_DIALOG_CLASSIFY2),
    _hIcon(NULL),
    _hFontTitle0(NULL),
    _hFontTitle(NULL),
    _readonly(false)
{
    SetFile(file);
    if (xml.at(0) == L'{') {
        SetJson(xml, group);
    }
    else {
        SetXml(xml, group);
    }
}

CDlgClassify2::~CDlgClassify2()
{
    if(NULL != _hIcon) {
        DestroyIcon(_hIcon);
        _hIcon = NULL;
    }
    if(NULL != _hFontTitle0) {
        ::DeleteObject(_hFontTitle0);
        _hFontTitle0 = NULL;
    }
    if(NULL != _hFontTitle) {
        ::DeleteObject(_hFontTitle);
        _hFontTitle = NULL;
    }
    if(NULL != _tooltip) {
        ::DestroyWindow(_tooltip);
        _tooltip = NULL;
    }
}

void CDlgClassify2::SetXml(_In_ const std::wstring& xml, _In_ const std::wstring& group) throw()
{
    _clsdata.LoadFromXml(xml, group);
}

void CDlgClassify2::SetJson(_In_ const std::wstring& ws, _In_ const std::wstring& group) throw()
{
    _clsdata.LoadFromJson(ws, group);
}

void CDlgClassify2::SetInitialData(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& tags) throw()
{
    assert(!_clsdata.IsEmpty());
    _unMatchTags.clear();
    _finalTags.clear();
    for(int i=0; i<(int)tags.size(); i++) {
        if(!InitClassifyItem(tags[i].first, tags[i].second)) {
            _unMatchTags.push_back(tags[i]);
        }
    }
}

BOOL CDlgClassify2::InitClassifyItem(const std::wstring& name, const std::wstring& value)
{
    assert(!_clsdata.IsEmpty());
    std::vector<classify::CItem>& vItems = _clsdata.GetItemList();

    for(std::vector<classify::CItem>::iterator it=vItems.begin(); it!=vItems.end(); ++it) {

        if(0 == _wcsicmp(name.c_str(), (*it).GetName().c_str())) {

            std::vector<classify::CItemValue>& values = (*it).GetValues();
            for(int i=0; i<(int)values.size(); i++) {
                if(0 == _wcsicmp(value.c_str(), values[i].GetData().c_str())) {
                    // Good , found it
                    int nPriority = values[i].GetPriority();
                    if((*it).IsMultiSelection()) {
                        (*it).Select(i);
                    }
                    else {
                        std::vector<classify::CItemValue> newValues;
                        int nSelId = 0;
                        // Remove all values whose priority is lower than input value's
                        for(int j=0; j<(int)values.size(); j++) {
                            if(values[j].GetPriority() >= nPriority) {
                                newValues.push_back(values[j]);
                                if(0 == _wcsicmp(value.c_str(), values[j].GetData().c_str())) {
                                    nSelId = (int)newValues.size() - 1;
                                }
                            }
                        }
                        (*it).GetValues() = newValues;
                        (*it).Select(nSelId);
                        // We force this item to be mandatory because
                        (*it).SetMandatory(true);
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void CDlgClassify2::GetClassificationTags(_Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags)
{
    tags = _finalTags;
}

VOID CDlgClassify2::GetDialogFontInfo(LOGFONTW* plFont)
{
    TEXTMETRIC tm = {0};
    HDC hDC = ::GetDC(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME));
    HFONT hFont = GetWindowFont(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME));
    
    HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
    GetTextFaceW(hDC, 32, plFont->lfFaceName);
    GetTextMetricsW(hDC, &tm);
    ::SelectObject(hDC, hOldFont);
    
    plFont->lfHeight = tm.tmHeight;
    plFont->lfWidth = tm.tmAveCharWidth;
    plFont->lfEscapement = 0;
    plFont->lfOrientation = 0;
    plFont->lfWeight = tm.tmWeight;
    plFont->lfItalic = 0;
    plFont->lfUnderline = 0;
    plFont->lfStrikeOut = 0;
    plFont->lfCharSet = tm.tmCharSet;
    plFont->lfOutPrecision = OUT_DEFAULT_PRECIS;
    plFont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    plFont->lfQuality = PROOF_QUALITY;
    plFont->lfPitchAndFamily = tm.tmPitchAndFamily;
}

BOOL CDlgClassify2::OnInitialize()
{
    HRESULT hr = S_OK;
    SHFILEINFOW sfi = {0};
    LOGFONTW lFont = {0};

    if(_file.empty() || _clsdata.IsEmpty()) {
        return FALSE;
    }
    
	CoInitialize(NULL);
    hr = (BOOL)SHGetFileInfoW(_file.c_str(), -1, &sfi, sizeof(sfi), SHGFI_ICON);
    CoUninitialize();
    if(hr) {
        _hIcon = sfi.hIcon;
        Static_SetIcon(GetDlgItem(GetHwnd(), IDC_FILE_ICON), _hIcon);
    }
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME), _file.c_str());
    
    GetDialogFontInfo(&lFont);
    
    _hFontTitle0 = CreateFontW(lFont.lfHeight,
                               0,
                               lFont.lfEscapement,
                               lFont.lfOrientation,
                               lFont.lfWeight + 150,
                               FALSE,
                               FALSE,
                               FALSE,
                               lFont.lfCharSet,
                               lFont.lfOutPrecision,
                               lFont.lfClipPrecision,
                               lFont.lfQuality,
                               lFont.lfPitchAndFamily,
                               lFont.lfFaceName);
    ::SendMessageW(GetDlgItem(GetHwnd(), IDC_STATIC_CLASSIFY_TITLE), WM_SETFONT, (WPARAM)_hFontTitle0, TRUE);

    _hFontTitle = CreateFontW( lFont.lfHeight,
                               0,
                               lFont.lfEscapement,
                               lFont.lfOrientation,
                               lFont.lfWeight + 150,
                               FALSE,
                               FALSE,
                               FALSE,
                               lFont.lfCharSet,
                               lFont.lfOutPrecision,
                               lFont.lfClipPrecision,
                               lFont.lfQuality,
                               lFont.lfPitchAndFamily,
                               lFont.lfFaceName);

    _tooltip = CreateWindowExW(0, L"tooltips_class32", NULL, 
             WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
             CW_USEDEFAULT, NULL, NULL, NULL, NULL);

    TOOLINFOW    toolinfo = {0};
    memset(&toolinfo, 0, sizeof(toolinfo));    
    toolinfo.cbSize = sizeof(toolinfo);
    toolinfo.uFlags = TTF_SUBCLASS | TTF_TRANSPARENT;
    toolinfo.hwnd = GetDlgItem(GetHwnd(), IDC_EDIT_OBJECTNAME);
    toolinfo.uId = IDC_EDIT_OBJECTNAME;
    toolinfo.hinst = NULL;
    toolinfo.lpszText = (LPWSTR)_file.c_str();
    toolinfo.lParam = NULL;
    GetClientRect(toolinfo.hwnd, &toolinfo.rect);
    SendMessageW(_tooltip, TTM_ADDTOOL, 0, (LPARAM)&toolinfo);


    // Init Summary List
    _listCtl.Attach(GetDlgItem(GetHwnd(), IDC_CLASSIFY_LIST));
    _listCtl.SetData(&_clsdata);
    _listCtl.SetTitleFont(_hFontTitle);
    _listCtl.Init();

    return TRUE;
}

BOOL CDlgClassify2::OnNotify(_In_ LPNMHDR lpnmhdr)
{
    if(IDC_SUMMARY_LIST == lpnmhdr->idFrom) {
        HWND hList = GetDlgItem(GetHwnd(), IDC_SUMMARY_LIST);
        if(NM_CLICK == lpnmhdr->code) {
            LVHITTESTINFO itestinfo;

            assert(hList == lpnmhdr->hwndFrom);
            memset(&itestinfo, 0, sizeof(itestinfo));
            itestinfo.pt = ((LPNMITEMACTIVATE)lpnmhdr)->ptAction;
        }
    }
    else {
        ;
    }

    return CDlgTemplate::OnNotify(lpnmhdr);
}

BOOL CDlgClassify2::OnCommand(WORD notify, WORD id, HWND hwnd)
{
    return CDlgTemplate::OnCommand(notify, id, hwnd);
}

BOOL CDlgClassify2::OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct)
{
    UNREFERENCED_PARAMETER(lpDrawItemStruct);
    if(IDC_CLASSIFY_LIST == lpDrawItemStruct->CtlID) {
        _listCtl.OnOwnerDraw(lpDrawItemStruct);
        return TRUE;
    }
    return FALSE;
}

BOOL CDlgClassify2::OnMeasureItem(MEASUREITEMSTRUCT* lpMeasureItemStruct)
{
    if(IDC_CLASSIFY_LIST == lpMeasureItemStruct->CtlID) {
        // Adjust list item size        
        _listCtl.OnMeasureItem(lpMeasureItemStruct);
        return TRUE;
    }

    return FALSE;
}

void CDlgClassify2::OnComboBoxSelChanged()
{
}

void CDlgClassify2::OnOk()
{
    // Make sure all the mandatory label s have value
    const classify::CItem* pItem = NULL;
    if(!_listCtl.MandatoryCheck(&pItem)) {
        std::wstring wsInfo = L"You must select a value for this label: ";
        wsInfo += pItem->GetDisplayName();
        MessageBoxW(GetHwnd(), wsInfo.c_str(), L"Error", MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    // get & Return result
    _listCtl.GetResult(_finalTags);
    for(int i=0; i<(int)_unMatchTags.size(); i++) {
        _finalTags.push_back(_unMatchTags[i]);
    }
    CDlgTemplate::OnOk();
}