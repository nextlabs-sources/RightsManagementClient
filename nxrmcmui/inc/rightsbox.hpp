

#ifndef __NXRM_COMMONUI_RIGHTSBOX_HPP__
#define __NXRM_COMMONUI_RIGHTSBOX_HPP__

#include <Windows.h>
#include <Commctrl.h>

#include <string>
#include <vector>

#include <nudf\shared\nxlfmt.h>

#include "propsheet.hpp"
#include "dlgtemplate.hpp"



class CGeneralPage : public CPropPageDlgTemplate
{
public:
    CGeneralPage();
    virtual ~CGeneralPage();

    virtual BOOL OnInitialize();
    virtual void OnPsnSetActive(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnKillActive(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnApply(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnReset(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnHelp(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizBack(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizNext(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnGetObject(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn);

    inline const std::wstring& GetFile() const throw() {return _file;}
    inline void SetFile(_In_ const std::wstring& file) throw() {_file = file;}
    inline const NXL_HEADER& GetHeader() const throw() {return _header;}
    inline void SetHeader(_In_opt_ PCNXL_HEADER header) throw() {(NULL==header) ? memset(&_header, 0, sizeof(_header)) : memcpy(&_header, header, sizeof(_header));}
    inline void SetCenterParent(_In_ BOOL bCenter) throw() {_center_parent = bCenter;}

private:
    std::wstring    _file;
    NXL_HEADER      _header;
    HICON           _hIcon;
    BOOL            _center_parent;
};

class CPermissionPage : public CPropPageDlgTemplate
{
public:
    CPermissionPage();
    virtual ~CPermissionPage();
    
    inline const std::wstring& GetFile() const throw() {return _file;}
    inline void SetFile(_In_ const std::wstring& file) throw() {_file = file;}
    inline ULONGLONG GetRights() const throw() {return _InitialRights;}
    inline void SetRights(ULONGLONG rights) throw() {_InitialRights=rights;_NewRights=_InitialRights;}
    inline void SetCenterParent(_In_ BOOL bCenter) throw() {_center_parent = bCenter;}

    virtual BOOL OnInitialize();
    virtual BOOL OnNotify(_In_ LPNMHDR lpnmhdr);
    virtual BOOL OnCommand(WORD notify, WORD id, HWND hwnd);

    virtual void OnPsnSetActive(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnKillActive(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnApply(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnReset(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnHelp(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizBack(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizNext(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnGetObject(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn);

protected:
    void EnableEditMode(BOOL bEnabled);
    void UpdateRightsView();
    void CheckItem(int iItem, BOOL bCheck);
    ULONGLONG ItemToRights(int iItem);
    int RightsToItem(ULONGLONG rights);
    BOOL IsItemChecked(int iItem);

private:
    std::wstring    _file;
    HIMAGELIST      _hEditImgList;
    HIMAGELIST      _hViewImgList;
    BOOL            _bEditMode;
    ULONGLONG       _InitialRights;
    ULONGLONG       _NewRights;
    BOOL            _center_parent;
};

typedef std::vector<std::pair<std::wstring,std::wstring>> ATTRIBUTEPAIR;
class CClassifyPage : public CPropPageDlgTemplate
{
public:
    CClassifyPage();
    virtual ~CClassifyPage();

    virtual BOOL OnInitialize();
    virtual void OnPsnSetActive(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnKillActive(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnApply(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnReset(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnHelp(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizBack(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizNext(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnWizFinish(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnQueryCancel(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnGetObject(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnTranslateAccelerator(_In_ LPPSHNOTIFY lppsn);
    virtual void OnPsnQueryInitialFocus(_In_ LPPSHNOTIFY lppsn);
    
    inline const std::wstring& GetFile() const throw() {return _file;}
    inline void SetFile(_In_ const std::wstring& file) throw() {_file = file;}
    inline const ATTRIBUTEPAIR& GetClassifications() const throw() {return _classifydata;}
    inline void SetClassifications(const ATTRIBUTEPAIR& classifydata) throw() {_classifydata=classifydata;}
    inline void SetCenterParent(_In_ BOOL bCenter) throw() {_center_parent = bCenter;}

private:
    std::wstring    _file;
    ATTRIBUTEPAIR   _classifydata;
    HWND            _tooltip;
    HICON           _hIcon;
    BOOL            _center_parent;
};


class CRightsBox : public CPropSheet
{
public:
    CRightsBox();
    virtual ~CRightsBox();

    inline const CGeneralPage& GetGeneralPage() const throw() {return _genPage;}
    inline CGeneralPage& GetGeneralPage() throw() {return _genPage;}
    inline const CPermissionPage& GetPermissionPage() const throw() {return _pemPage;}
    inline CPermissionPage& GetPermissionPage() throw() {return _pemPage;}
    inline const CClassifyPage& GetClassifyPage() const throw() {return _clsPage;}
    inline CClassifyPage& GetClassifyPage() throw() {return _clsPage;}

    LONG ShowGeneral(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_opt_ PCNXL_HEADER pHeader);
    LONG ShowPermission(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ ULONGLONG uRights);
    LONG ShowClassification(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ const ATTRIBUTEPAIR& classifydata);
    LONG ShowDetails(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_ ULONGLONG uRights, _In_ const ATTRIBUTEPAIR& classifydata);
    LONG ShowAll(_In_ HWND hParent, _In_ const std::wstring& wzFile, _In_opt_ PCNXL_HEADER pHeader, _In_ ULONGLONG uRights, _In_ const ATTRIBUTEPAIR& classifydata);

private:
    CGeneralPage    _genPage;
    CPermissionPage _pemPage;
    CClassifyPage   _clsPage;
    PROPSHEETPAGEW  _psps[3];
};



#endif