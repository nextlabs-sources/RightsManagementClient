

#ifndef __NXRM_COMMONUI_DLGCLASSIFY_2_HPP__
#define __NXRM_COMMONUI_DLGCLASSIFY_2_HPP__

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>

#include <nudf\xmlparser.hpp>

#include "classifydata.hpp"
#include "classifylistctrl.hpp"
#include "dlgtemplate.hpp"




class CDlgClassify2 : public CDlgTemplate
{
public:
    CDlgClassify2();
    CDlgClassify2(_In_ const std::wstring& file, _In_ const std::wstring& xml, _In_ const std::wstring& group);
    virtual ~CDlgClassify2();
    
    void GetClassificationTags(_Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags);

    inline void SetFile(_In_ const std::wstring& file) throw() {_file = file;}
    inline void SetReadOnly(_In_ bool readonly) throw() {_readonly=readonly; _listCtl.SetReadOnly(_readonly);}
    void SetXml(_In_ const std::wstring& xml, _In_ const std::wstring& group) throw();
    void SetJson(_In_ const std::wstring& ws, _In_ const std::wstring& group) throw();
    void SetInitialData(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& tags) throw();
    
    virtual BOOL OnInitialize();
    virtual BOOL OnNotify(_In_ LPNMHDR lpnmhdr);
    virtual BOOL OnCommand(WORD notify, WORD id, HWND hwnd);
    virtual BOOL OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct);
    virtual BOOL OnMeasureItem(MEASUREITEMSTRUCT* lpMeasureItemStruct);

    void OnBtnClickNext();
    void OnBtnClickBack();
    void OnComboBoxSelChanged();
    virtual void OnOk();


protected:
    void SetPage();
    void ShowSummary();
    BOOL InitClassifyItem(const std::wstring& name, const std::wstring& value);
    VOID GetDialogFontInfo(LOGFONTW* plFont);

private:
    std::wstring            _file;
    bool                    _readonly;
    classify::CClassifyData _clsdata;
    HICON                   _hIcon;
    HFONT                   _hFontTitle0;
    HFONT                   _hFontTitle;
    HWND                    _tooltip;
    CClassifyListCtrl       _listCtl;
    HIMAGELIST              _listImgList;
    std::vector<std::pair<std::wstring,std::wstring>>   _unMatchTags;
    std::vector<std::pair<std::wstring,std::wstring>>   _finalTags;
};


#endif