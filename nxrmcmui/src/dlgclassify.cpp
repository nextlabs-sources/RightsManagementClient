

#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <assert.h>

#include <list>

#include <nudf\exception.hpp>

#include "commonui.hpp"
#include "resource.h"
#include "dlgtemplate.hpp"
#include "dlgclassify.hpp"



CDlgClassify::CDlgClassify() : CDlgTemplate(IDD_DIALOG_CLASSIFY), _hIcon(NULL), _curLabel(NULL), _hFontTitle0(NULL), _hFontTitle(NULL), _summary(false)
{
}

CDlgClassify::CDlgClassify(_In_ const std::wstring& file, _In_ const std::wstring& xml) : CDlgTemplate(IDD_DIALOG_CLASSIFY), _hIcon(NULL), _curLabel(NULL), _hFontTitle0(NULL), _hFontTitle(NULL), _summary(false)
{
    SetFile(file);
    SetXml(xml);
}

CDlgClassify::~CDlgClassify()
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
}

void CDlgClassify::SetXml(_In_ const std::wstring& xml) throw()
{
    try {
        
        nudf::util::CXmlDocument    xmldoc;
        CComPtr<IXMLDOMElement>     spRoot;
        CComPtr<IXMLDOMNode>        spLang;
        CComPtr<IXMLDOMNode>        spRootLabel;
        std::wstring                wsDefaultLang;
        std::wstring                wsRootName;
        WCHAR                       wzLcName[128] = {0};

        xmldoc.LoadFromXml(xml.c_str());
        if(!xmldoc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        wsRootName = nudf::util::XmlUtil::GetNodeName(spRoot);
        if(0 != _wcsicmp(wsRootName.c_str(), L"LABEL")) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        _labels.reset(new CLabel());
        if(!_labels->Load(spRoot, NULL)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _curLabel = _labels.get();
        
        xmldoc.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        _labels.reset();
    }
}

BOOL CDlgClassify::OnInitialize()
{
    HRESULT hr = S_OK;
    SHFILEINFOW sfi = {0};

    if(_file.empty() || NULL == _labels.get()) {
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

    NONCLIENTMETRICS metrics = {0};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &metrics, 0);
    
    _hFontTitle0 = CreateFontW(18, 0, 0, 0, FW_BOLD,
                         0, FALSE, FALSE,
                         ANSI_CHARSET,
                         OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE,
                         metrics.lfMessageFont.lfFaceName);
    ::SendMessageW(GetDlgItem(GetHwnd(), IDC_STATIC_CLASSIFY_TITLE), WM_SETFONT, (WPARAM)_hFontTitle0, TRUE);

    _hFontTitle = CreateFontW(16, 0, 0, 0, FW_BOLD,
                         0, FALSE, FALSE,
                         ANSI_CHARSET,
                         OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE,
                         metrics.lfMessageFont.lfFaceName);
    ::SendMessageW(GetDlgItem(GetHwnd(), IDC_STATIC_LABLE_TITLE), WM_SETFONT, (WPARAM)_hFontTitle, TRUE);

    // Init Summary List
    LVCOLUMNW   col;
    RECT        rc;
    HWND        hList = NULL;

    hList = GetDlgItem(GetHwnd(), IDC_SUMMARY_LIST);
    GetClientRect(hList, &rc);
    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt = LVCFMT_LEFT;
    col.cx = (rc.right - rc.left) / 3 - 4;
    col.pszText = L"Name";
    ListView_InsertColumn(hList, 0, &col);
    col.cx = (2*(rc.right - rc.left)) / 3 - 16;
    col.pszText = L"Value";
    ListView_InsertColumn(hList, 1, &col);

    hList = GetDlgItem(GetHwnd(), IDC_MULTISEL_LIST);
    GetClientRect(hList, &rc);
    ListView_SetExtendedListViewStyle(hList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt = LVCFMT_LEFT;
    col.cx = (rc.right - rc.left) - 20;
    col.pszText = L"Values";
    ListView_InsertColumn(hList, 0, &col);

    // SHow first page
    SetPage();

    return TRUE;
}

BOOL CDlgClassify::OnNotify(_In_ LPNMHDR lpnmhdr)
{
    if(IDC_MULTISEL_LIST == lpnmhdr->idFrom) {
        HWND hList = GetDlgItem(GetHwnd(), IDC_MULTISEL_LIST);
        if(NM_CLICK == lpnmhdr->code) {
            LVHITTESTINFO itestinfo;

            assert(hList == lpnmhdr->hwndFrom);
            memset(&itestinfo, 0, sizeof(itestinfo));
            itestinfo.pt = ((LPNMITEMACTIVATE)lpnmhdr)->ptAction;
            if(-1 != ListView_SubItemHitTest(hList, &itestinfo)) {
                if(0 != ((LVHT_ONITEMICON|LVHT_ONITEMSTATEICON)&itestinfo.flags)) {
                    // Change image
                    UINT state = ListView_GetItemState(hList, itestinfo.iItem, LVIS_STATEIMAGEMASK);
                    state >>= 12;
                    if(0 == (state -1)) {
                        _curLabel->SelectValue(itestinfo.iItem);
                    }
                    else {
                        _curLabel->UnselectValue(itestinfo.iItem);
                    }
                }
            }
        }
    }
    else {
        ;
    }

    return CDlgTemplate::OnNotify(lpnmhdr);
}

BOOL CDlgClassify::OnCommand(WORD notify, WORD id, HWND hwnd)
{
    if(IDC_BUTTON_BACK == id) {
        OnBtnClickBack();
    }
    else if(IDC_BUTTON_NEXT == id) {
        OnBtnClickNext();
    }
    else if(IDC_COMBO_VALUE == id) {
        switch(notify)
        {
        case CBN_SELCHANGE:
            OnComboBoxSelChanged();
            break;
        default:
            break;
        }
    }
    else {
        ;//
    }

    return CDlgTemplate::OnCommand(notify, id, hwnd);
}

void CDlgClassify::OnBtnClickNext()
{
    // Mandatory Checking
    if(_curLabel->IsMultiSelect()) {
        if(_curLabel->IsMandatory() && 0 == _curLabel->GetSelectedValueId()) {
            MessageBoxW(GetHwnd(), L"This attribute is mandatory, you must select one valid value.", L"Error", MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }
    else {
        if(_curLabel->IsMandatory() && _curLabel->GetSelectedValue()->GetValue().empty()) {
            MessageBoxW(GetHwnd(), L"This attribute is mandatory, you must select one valid value.", L"Error", MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

    // Move to Next
    if(NULL != _curLabel->GetSelectedValue() && NULL != _curLabel->GetSelectedValue()->GetSubLabel().get()) {
        _curLabel = _curLabel->GetSelectedValue()->GetSubLabel().get();
        assert(NULL != _curLabel);
        SetPage();
    }
    else {
        ShowSummary();
    }
}

void CDlgClassify::OnBtnClickBack()
{
    if(!_summary) {
        _curLabel = _curLabel->GetParent();
    }
    assert(NULL != _curLabel);
    SetPage();
}

void CDlgClassify::OnComboBoxSelChanged()
{
    int id = ComboBox_GetCurSel(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE));
    assert(CB_ERR != id);
    _curLabel->SelectValue(id);
    ShowWindow(GetDlgItem(GetHwnd(), IDC_BUTTON_NEXT), _curLabel->GetSelectedValue()->HasSubLabels() ? TRUE : FALSE);
    ShowWindow(GetDlgItem(GetHwnd(), IDOK), _curLabel->GetSelectedValue()->HasSubLabels() ? FALSE : TRUE);    
}

void CDlgClassify::OnOk()
{
    CDlgTemplate::OnOk();
}

void CDlgClassify::SetPage()
{
    HWND hValueList = GetDlgItem(GetHwnd(), IDC_MULTISEL_LIST);

    assert(NULL != _curLabel);

    // Disable Summary
    _summary = false;
    ShowWindow(GetDlgItem(GetHwnd(), IDC_SUMMARY_LIST), FALSE);
    ShowWindow(GetDlgItem(GetHwnd(), IDOK), FALSE);

    // Set button
    ShowWindow(GetDlgItem(GetHwnd(), IDC_BUTTON_BACK), _curLabel->IsFirstPage() ? FALSE : TRUE);
    ShowWindow(GetDlgItem(GetHwnd(), IDC_BUTTON_NEXT), TRUE);

    // Set Text
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_LABLE_TITLE), _curLabel->GetDisplayName().c_str());
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_LABEL_DESCRIPTION), _curLabel->GetDescription().c_str());

    // Set Values
    int value_count = (int)_curLabel->GetValues().size();
    // Remove existing values in combobox
    while(ComboBox_GetCount(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE)) > 0) {
        ComboBox_DeleteString(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE), 0);
    }
    if(_curLabel->IsMultiSelect()) {
        // Allow multi-selection
        ShowWindow(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE), FALSE);
        ShowWindow(hValueList, TRUE);
        ListView_DeleteAllItems(hValueList);
        // Add Values
        LVITEMW     item;
        memset(&item, 0, sizeof(item));
        item.mask   = LVIF_TEXT;
        for(int i=0; i<(int)_curLabel->GetValues().size(); i++) {
            if(i > 30) {
                break;  // This is because the _selected_id is integer, we only support 31 bits at most.
            }
            BOOL bChecked = (_curLabel->GetSelectedValueId() & (0x00000001 << i)) ? TRUE : FALSE;
            item.iItem   = i;
            item.iSubItem= 0;
            item.pszText = (LPWSTR)_curLabel->GetValues()[i]->GetValue().c_str();
            ListView_InsertItem(hValueList, &item);
            ListView_SetItemState(hValueList, i, UINT((int(bChecked) + 1) << 12), LVIS_STATEIMAGEMASK);
        }
    }
    else {
        // Single selection
        ShowWindow(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE), TRUE);
        ShowWindow(GetDlgItem(GetHwnd(), IDC_MULTISEL_LIST), FALSE);
        for(int i=0; i<value_count; i++) {
            ComboBox_AddString(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE), _curLabel->GetValues()[i]->GetValue().c_str());
        }
        ComboBox_SelectString(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE), 0, _curLabel->GetSelectedValue()->GetValue().c_str());
    }
}

void CDlgClassify::ShowSummary()
{
    HWND hList = GetDlgItem(GetHwnd(), IDC_SUMMARY_LIST);

    // Show Controls
    ShowWindow(GetDlgItem(GetHwnd(), IDC_BUTTON_BACK), TRUE);
    ShowWindow(GetDlgItem(GetHwnd(), IDC_BUTTON_NEXT), FALSE);
    ShowWindow(GetDlgItem(GetHwnd(), IDOK), TRUE);
    ShowWindow(GetDlgItem(GetHwnd(), IDC_COMBO_VALUE), FALSE);
    ShowWindow(GetDlgItem(GetHwnd(), IDC_MULTISEL_LIST), FALSE);
    ListView_DeleteAllItems(hList);
    ShowWindow(hList, TRUE);
    
    // Set Text
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_LABLE_TITLE), L"Summary");
    SetWindowTextW(GetDlgItem(GetHwnd(), IDC_STATIC_LABEL_DESCRIPTION), L"This is all the classification tags selected by you:");

    std::vector<std::pair<std::wstring,std::wstring>>   tags;
    LVITEMW     item;

    _labels->GetClassificationTags(tags);
    memset(&item, 0, sizeof(item));
    item.mask   = LVIF_TEXT;
    for(int i=0; i<(int)tags.size(); i++) {
        item.iItem   = i;
        item.iSubItem= 0;
        item.pszText = (LPWSTR)tags[i].first.c_str();
        ListView_InsertItem(hList, &item);
        item.iSubItem= 1;
        item.pszText = (LPWSTR)tags[i].second.c_str();
        ListView_SetItem(hList, &item);
    }

    _summary = true;
}

