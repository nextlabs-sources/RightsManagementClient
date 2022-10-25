

#ifndef __NXRM_COMMONUI_DLGCLASSIFY_HPP__
#define __NXRM_COMMONUI_DLGCLASSIFY_HPP__

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>

#include <nudf\xmlparser.hpp>

#include "labelinfo.hpp"
#include "dlgtemplate.hpp"




class CDlgClassify : public CDlgTemplate
{
public:
    CDlgClassify();
    CDlgClassify(_In_ const std::wstring& file, _In_ const std::wstring& xml);
    virtual ~CDlgClassify();
    
    inline void GetClassificationTags(_Out_ std::vector<std::pair<std::wstring,std::wstring>>& tags) {_labels->GetClassificationTags(tags);}

    inline void SetFile(_In_ const std::wstring& file) throw() {_file = file;}
    void SetXml(_In_ const std::wstring& xml) throw();
    
    virtual BOOL OnInitialize();
    virtual BOOL OnNotify(_In_ LPNMHDR lpnmhdr);
    virtual BOOL OnCommand(WORD notify, WORD id, HWND hwnd);

    void OnBtnClickNext();
    void OnBtnClickBack();
    void OnComboBoxSelChanged();
    virtual void OnOk();


protected:
    void SetPage();
    void ShowSummary();

private:
    std::wstring             _file;
    std::shared_ptr<CLabel>  _labels;
    CLabel*                  _curLabel;
    HICON                    _hIcon;
    HFONT                    _hFontTitle0;
    HFONT                    _hFontTitle;
    bool                     _summary;
};


#endif