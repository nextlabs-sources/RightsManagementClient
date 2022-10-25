

#include <Windows.h>
#include <Windowsx.h>
#include <assert.h>

#include "gui\windlg.hpp"


using namespace NXGUI;


//
//  class dialog_template
//

dialog_template* dialog_template::create(unsigned int template_id, HINSTANCE instance)
{
    return dialog_template::create(MAKEINTRESOURCE(template_id), instance);
}

dialog_template* dialog_template::create(const wchar_t* template_name, HINSTANCE instance)
{
    HRSRC   hrsrc   = NULL;
    HGLOBAL hres    = NULL;
    dialog_template* p = NULL;

    hrsrc = ::FindResourceW(instance, template_name, RT_DIALOG);
    if (NULL == hrsrc) {
        return NULL;
    }

    hres = ::LoadResource(instance, hrsrc);
    if (NULL == hres) {
        return NULL;
    }

    p = create(hres);
    FreeResource(hres);

    return p;
}

dialog_template* dialog_template::create(HGLOBAL h)
{
    dialog_template* p = NULL;
    void*   pres = LockResource(h);
    if (NULL == pres) {
        return NULL;
    }

    const unsigned short* magic = (const WORD*)pres;
    if (magic[0] == 1 && magic[1] == 0xFFFF) {
        // extended template
        p = new dialog_template_ext(pres);
    }
    else {
        // standard template
        p = new dialog_template_std(pres);
    }

    return p;
}

dialog_template::~dialog_template()
{
    clear();
}

void dialog_template::clear() noexcept
{
    _style = 0;
    _extended_style = 0;
    _cdit = 0;
    _x = 0;
    _y = 0;
    _cx = 0;
    _cy = 0;
}

bool dialog_template::empty() const noexcept
{
    return (0 == _cdit && 0 == _style);
}

dialog_template_std::dialog_template_std() : dialog_template()
{
}

dialog_template_std::dialog_template_std(const void* p) : dialog_template()
{
    const DLGTEMPLATE* t = (const DLGTEMPLATE*)p;
    _style = t->style;
    _extended_style = t->dwExtendedStyle;
    _cdit = t->cdit;
    _x = t->x;
    _y = t->y;
    _cx = t->cx;
    _cy = t->cy;

    const unsigned char* menu_p = (const unsigned char*)(t + 1);
    _menu = sz_or_ord(t + 1);
    const unsigned char* class_p = menu_p + _menu.size();
    _class = sz_or_ord(class_p);
    const unsigned char* title_p = class_p + _class.size();
    _title = sz_or_ord(title_p);
}

dialog_template_std::~dialog_template_std()
{
}

size_t dialog_template_std::size() const noexcept
{
    if (empty()) {
        return 0;
    }

    size_t size = sizeof(DLGTEMPLATE) + _menu.size() + _class.size() + _title.size() + sizeof(unsigned short)/*point size*/ + (_typeface.length() + 1)*sizeof(wchar_t);
    return size;
}

dialog_template_ext::dialog_template_ext() : dialog_template()
{
}

typedef struct {
    WORD      dlgVer;
    WORD      signature;
    DWORD     helpID;
    DWORD     exStyle;
    DWORD     style;
    WORD      cDlgItems;
    short     x;
    short     y;
    short     cx;
    short     cy;
} DLGTEMPLATEEX_HEADER;
dialog_template_ext::dialog_template_ext(const void* p) : dialog_template()
{
    const DLGTEMPLATEEX_HEADER* t = (const DLGTEMPLATEEX_HEADER*)p;
    _style = t->style;
    _extended_style = t->exStyle;
    _cdit = t->cDlgItems;
    _x = t->x;
    _y = t->y;
    _cx = t->cx;
    _cy = t->cy;
    _help_id = t->helpID;
    _dlg_ver = t->dlgVer;
    _signature = t->signature;

    const unsigned char* menu_p = (const unsigned char*)(t + 1);
    _menu = sz_or_ord(t+1);
    const unsigned char* class_p = menu_p + _menu.size();
    _class = sz_or_ord(class_p);
    const unsigned char* title_p = class_p + _class.size();
    _title = sz_or_ord(title_p);
    const unsigned short* ptsize_p = (const unsigned short*)(title_p + _title.size());
    _pointsize = *ptsize_p;
    const unsigned short* _font_weight_p = (const unsigned short*)(title_p + 1);
    _font_weight = *_font_weight_p;
    const unsigned char* _font_italic_p = (const unsigned char*)(_font_weight_p + 1);
    _font_italic = *_font_italic_p;
    const unsigned char* _font_charset_p = (const unsigned char*)(_font_italic_p + 1);
    _font_charset = *_font_charset_p;
    const wchar_t* typeface_p = (const wchar_t*)(_font_charset_p + 1);
    _typeface = typeface_p;
}

dialog_template_ext::~dialog_template_ext()
{
}

size_t dialog_template_ext::size() const noexcept
{
    if (empty()) {
        return 0;
    }

    size_t size = sizeof(DLGTEMPLATEEX_HEADER) + _menu.size() + _class.size() + _title.size()
                    + sizeof(unsigned short)    /*point size*/
                    + sizeof(unsigned short)    /*weight*/
                    + sizeof(unsigned char)     /*italic*/
                    + sizeof(unsigned char)     /*charset*/
                    + (_typeface.length() + 1)*sizeof(wchar_t);
    return size;
}


//
//  class window
//      - dialog related functions
//
bool window::create_dlg(const wchar_t* template_name, window* parent_wnd)
{
}

bool window::create_dlg_indirect(LPCDLGTEMPLATE dialog_template, window* parent_wnd, HINSTANCE instance)
{
    assert(dialog_template != NULL);

    //if (NULL == instance)
    //    instance = AfxGetResourceHandle();


    HGLOBAL template_handle = NULL;
    HWND    dialog_hwnd = NULL;

    dialog_template->


    TRY
    {
        VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
    AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);

#ifdef _UNICODE
    AfxInitNetworkAddressControl();
#endif

    AfxRegisterMFCCtrlClasses();

    // separately create OLE controls in the dialog template
    if (pOccManager != NULL)
    {
        if (!SetOccDialogInfo(&occDialogInfo))
            return FALSE;

        lpDialogTemplate = pOccManager->PreCreateDialog(&occDialogInfo,
            lpDialogTemplate);
    }

    if (lpDialogTemplate == NULL)
        return FALSE;

    // If no font specified, set the system font.
    CString strFace;
    WORD wSize = 0;
    BOOL bSetSysFont = !CDialogTemplate::GetFont(lpDialogTemplate, strFace,
        wSize);

    if (bSetSysFont)
    {
        CDialogTemplate dlgTemp(lpDialogTemplate);
        dlgTemp.SetSystemFont(wSize);
        hTemplate = dlgTemp.Detach();
    }

    if (hTemplate != NULL)
        lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

    // setup for modal loop and creation
    m_nModalResult = -1;
    m_nFlags |= WF_CONTINUEMODAL;

    // create modeless dialog
    AfxHookWindowCreate(this);
    hWnd = ::CreateDialogIndirect(hInst, lpDialogTemplate,
        pParentWnd->GetSafeHwnd(), AfxDlgProc);
#ifdef _DEBUG
    dwError = ::GetLastError();
#endif
    }
        CATCH_ALL(e)
    {
        DELETE_EXCEPTION(e);
        m_nModalResult = -1;
    }
    END_CATCH_ALL

        /* This is a bit tricky.  At this point, 1 of 3 things has happened:
        * 1) ::CreateDialogIndirect() created successfully and hWnd != NULL.
        * 2) ::CreateDialogIndirect() did create a window and then send the appropiate
        *    creation messages (ie. WM_CREATE).  However, the user handled WM_CREATE and
        *    returned -1.  This causes windows to send WM_DESTROY and WM_NCDESTROY to the
        *    newly created window.  Since WM_NCDESTROY has been sent, the destructor of this
        *    CWnd object has been called.  And ::CreateDialogIndirect() returns NULL.
        * 3) ::CreateDialogIndirect() did NOT create the window (ie. due to error in template)
        *    and returns NULL.
        *
        * (Note: In 3, this object is still valid; whereas in 2, this object has been deleted).
        *
        * Adding to the complexity, this function needs to do 2 memory clean up (call
        * pOccManager->PostCreateDialog() and delete occDialogInfo) if the destructor of
        * this object hasn't been called.  If the destructor has been called, the clean up is done
        * in the destructor.
        *
        * We can use the return valid of AfxUnhookWindowCreate() to differentiate between 2 and 3.
        *  - If AfxUnhookWindowCreate() returns true and hWnd==NULL, this means that (2) has happened
        *    and we don't have to clean up anything. (Cleanup should be done in the destructor).
        *  - If AfxUnhookWindowCreate() returns false and hWnd== NULL, this means that (3) has happened
        *    and we need to call PostNcDestroy().
        *
        * Note: hWnd != NULL implies that AfxUnhookWindowCreate() return TRUE.
        *
        * Note2: From this point on, don't access any member variables without checking hWnd.  If
        *        hWnd == NULL, the object has been destroyed already.
        */

        if (pOccManager != NULL)
        {
            pOccManager->PostCreateDialog(&occDialogInfo);

            if (hWnd != NULL)
            {
                SetOccDialogInfo(NULL);
            }
        }

    if (!AfxUnhookWindowCreate())
        PostNcDestroy();        // cleanup if Create fails too soon

                                // handle EndDialog calls during OnInitDialog
#ifdef _DEBUG
                                // Saving the old flag for checking WF_CONTINUEMODAL (if the object has not been destroyed)
                                // This needs to be after ::CreateDialogIndirect().
    DWORD dwOldFlags = 0;
    if (hWnd != NULL)
        dwOldFlags = m_nFlags;
#endif
    if (hWnd != NULL && !(m_nFlags & WF_CONTINUEMODAL))
    {
        ::DestroyWindow(hWnd);
        hWnd = NULL;
    }

    if (hTemplate != NULL)
    {
        GlobalUnlock(hTemplate);
        GlobalFree(hTemplate);
    }

    // help with error diagnosis (only if WM_INITDIALOG didn't EndDialog())
    if (hWnd == NULL)
    {
#ifdef _DEBUG
        if (dwOldFlags & WF_CONTINUEMODAL)
        {
            if (afxOccManager == NULL)
            {
                TRACE(traceAppMsg, 0, ">>> If this dialog has OLE controls:\n");
                TRACE(traceAppMsg, 0, ">>> AfxEnableControlContainer has not been called yet.\n");
                TRACE(traceAppMsg, 0, ">>> You should call it in your app's InitInstance function.\n");
            }
            else if (dwError != 0)
            {
                TRACE(traceAppMsg, 0, "Warning: Dialog creation failed!  GetLastError returns 0x%8.8X\n", dwError);
            }
        }
#endif //_DEBUG
        return FALSE;
    }

    ASSERT(hWnd == m_hWnd);
    return TRUE;
}

BOOL CWnd::CreateRunDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, window* pParentWnd, HINSTANCE hInst)
{
    BOOL bRet = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);

    if (bRet)
    {
        if (m_nFlags & WF_CONTINUEMODAL)
        {
            // enter modal loop
            DWORD dwFlags = MLF_SHOWONIDLE;
            if (GetStyle() & DS_NOIDLEMSG)
                dwFlags |= MLF_NOIDLEMSG;
            VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
        }

        // hide the window before enabling the parent, etc.
        if (m_hWnd != NULL)
            SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW |
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    return bRet;
}

bool window::create_run_dlg_indirect(LPCDLGTEMPLATE dialog_template, window* parent_wnd, HINSTANCE instance)
{
}



dialog::dialog()
{
    assert(NULL == hwnd());
    initialize();
}

dialog::~dialog()
{
    if (hwnd() != NULL) {
        destroy_window();
    }
}

void dialog::initialize()
{
    _id_help = 0;
    _template_name = NULL;
    _template_handle = NULL;
    _template = NULL;
    _dlg_init = NULL;
    _parent = NULL;
    _topwnd = NULL;
    _close_by_enddlg = FALSE;
}

bool dialog::on_paint(WPARAM wparam, LPARAM lparam, LRESULT& result)
{
    //::BeginPaint()
    //CPaintDC dc(this);
    //if (PaintWindowlessControls(&dc))
    //    return;
    result = wnd_proc(hwnd(), WM_PAINT, wparam, lparam);
}

bool dialog::on_query_end_session(WPARAM wparam, LPARAM lparam, LRESULT& result)
{
    return false;
}

bool dialog::on_end_session(WPARAM wparam, LPARAM lparam, LRESULT& result)
{
    bool ending = (bool)wparam;
    if (!ending)
        return;

    if (lparam & ENDSESSION_CLOSEAPP) {
        // do something
    }

    return true;
}

bool dialog::create(const wchar_t* template_name, window* parent)
{
    //assert(IS_INTRESOURCE(lpszTemplateName) || AfxIsValidString(lpszTemplateName));

    _template_name = template_name;  // used for help
    if (IS_INTRESOURCE(_template_name) && _id_help == 0) {
        _id_help = LOWORD((DWORD_PTR)_template_name);
    }

    HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
    HRSRC hResource = ::FindResourceW(hInst, lpszTemplateName, RT_DIALOG);
    HGLOBAL hTemplate = LoadResource(hInst, hResource);
    bool result = create_indirect(hTemplate, parent, hInst);
    FreeResource(hTemplate);

    return result;
}

// for backward compatibility
bool dialog::create_indirect(HGLOBAL hDialogTemplate, window* pParentWnd)
{
    return create_indirect(hDialogTemplate, pParentWnd, NULL);
}

bool dialog::create_indirect(HGLOBAL hDialogTemplate, window* parent, HINSTANCE instance)
{
    assert(hDialogTemplate != NULL);
    LPCDLGTEMPLATE dlg_template = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
    bool result = create_indirect(dlg_template, parent, NULL, instance);
    UnlockResource(hDialogTemplate);
    return result;
}

// for backward compatibility
bool dialog::create_indirect(LPCDLGTEMPLATE lpDialogTemplate, window* pParentWnd, void* lpDialogInit)
{
    return create_indirect(lpDialogTemplate, pParentWnd, lpDialogInit, NULL);
}

bool dialog::create_indirect(LPCDLGTEMPLATE lpDialogTemplate, window* pParentWnd, void* lpDialogInit, HINSTANCE hInst)
{
    assert(_template != NULL);

    if (pParentWnd == NULL) {
        pParentWnd = AfxGetMainWnd();
    }
    _dlg_init = lpDialogInit;

    return CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);
}

bool window::create_dlg(const std::wstring& template_name, window* pParentWnd)
{
    // load resource
    LPCDLGTEMPLATE lpDialogTemplate = NULL;
    HGLOBAL hDialogTemplate = NULL;
    HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
    HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
    hDialogTemplate = LoadResource(hInst, hResource);
    if (hDialogTemplate != NULL)
        lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
    ASSERT(lpDialogTemplate != NULL);

    // create a modeless dialog
    BOOL bSuccess = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);

    // free resource
    UnlockResource(hDialogTemplate);
    FreeResource(hDialogTemplate);

    return bSuccess;
}

BOOL CWnd::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd, HINSTANCE hInst)
{

#ifdef _DEBUG
    if ((AfxGetApp() != NULL) && (AfxGetApp()->IsKindOf(RUNTIME_CLASS(COleControlModule))))
    {
        TRACE(traceAppMsg, 0, "Warning: Creating dialog from within a COleControlModule application is not a supported scenario.\n");
    }
#endif

    ASSERT(lpDialogTemplate != NULL);
    if (pParentWnd != NULL)
        ASSERT_VALID(pParentWnd);

    if (!hInst)
        hInst = AfxGetResourceHandle();

    _AFX_OCC_DIALOG_INFO occDialogInfo;
    COccManager* pOccManager = afxOccManager;

    HGLOBAL hTemplate = NULL;

    HWND hWnd = NULL;
#ifdef _DEBUG
    DWORD dwError = 0;
#endif

    TRY
    {
        VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
    AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);

#ifdef _UNICODE
    AfxInitNetworkAddressControl();
#endif

    AfxRegisterMFCCtrlClasses();

    // separately create OLE controls in the dialog template
    if (pOccManager != NULL)
    {
        if (!SetOccDialogInfo(&occDialogInfo))
            return FALSE;

        lpDialogTemplate = pOccManager->PreCreateDialog(&occDialogInfo,
            lpDialogTemplate);
    }

    if (lpDialogTemplate == NULL)
        return FALSE;

    // If no font specified, set the system font.
    CString strFace;
    WORD wSize = 0;
    BOOL bSetSysFont = !CDialogTemplate::GetFont(lpDialogTemplate, strFace,
        wSize);

    if (bSetSysFont)
    {
        CDialogTemplate dlgTemp(lpDialogTemplate);
        dlgTemp.SetSystemFont(wSize);
        hTemplate = dlgTemp.Detach();
    }

    if (hTemplate != NULL)
        lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

    // setup for modal loop and creation
    m_nModalResult = -1;
    m_nFlags |= WF_CONTINUEMODAL;

    // create modeless dialog
    AfxHookWindowCreate(this);
    hWnd = ::CreateDialogIndirect(hInst, lpDialogTemplate,
        pParentWnd->GetSafeHwnd(), AfxDlgProc);
#ifdef _DEBUG
    dwError = ::GetLastError();
#endif
    }
        CATCH_ALL(e)
    {
        DELETE_EXCEPTION(e);
        m_nModalResult = -1;
    }
    END_CATCH_ALL

        /* This is a bit tricky.  At this point, 1 of 3 things has happened:
        * 1) ::CreateDialogIndirect() created successfully and hWnd != NULL.
        * 2) ::CreateDialogIndirect() did create a window and then send the appropiate
        *    creation messages (ie. WM_CREATE).  However, the user handled WM_CREATE and
        *    returned -1.  This causes windows to send WM_DESTROY and WM_NCDESTROY to the
        *    newly created window.  Since WM_NCDESTROY has been sent, the destructor of this
        *    CWnd object has been called.  And ::CreateDialogIndirect() returns NULL.
        * 3) ::CreateDialogIndirect() did NOT create the window (ie. due to error in template)
        *    and returns NULL.
        *
        * (Note: In 3, this object is still valid; whereas in 2, this object has been deleted).
        *
        * Adding to the complexity, this function needs to do 2 memory clean up (call
        * pOccManager->PostCreateDialog() and delete occDialogInfo) if the destructor of
        * this object hasn't been called.  If the destructor has been called, the clean up is done
        * in the destructor.
        *
        * We can use the return valid of AfxUnhookWindowCreate() to differentiate between 2 and 3.
        *  - If AfxUnhookWindowCreate() returns true and hWnd==NULL, this means that (2) has happened
        *    and we don't have to clean up anything. (Cleanup should be done in the destructor).
        *  - If AfxUnhookWindowCreate() returns false and hWnd== NULL, this means that (3) has happened
        *    and we need to call PostNcDestroy().
        *
        * Note: hWnd != NULL implies that AfxUnhookWindowCreate() return TRUE.
        *
        * Note2: From this point on, don't access any member variables without checking hWnd.  If
        *        hWnd == NULL, the object has been destroyed already.
        */

        if (pOccManager != NULL)
        {
            pOccManager->PostCreateDialog(&occDialogInfo);

            if (hWnd != NULL)
            {
                SetOccDialogInfo(NULL);
            }
        }

    if (!AfxUnhookWindowCreate())
        PostNcDestroy();        // cleanup if Create fails too soon

                                // handle EndDialog calls during OnInitDialog
#ifdef _DEBUG
                                // Saving the old flag for checking WF_CONTINUEMODAL (if the object has not been destroyed)
                                // This needs to be after ::CreateDialogIndirect().
    DWORD dwOldFlags = 0;
    if (hWnd != NULL)
        dwOldFlags = m_nFlags;
#endif
    if (hWnd != NULL && !(m_nFlags & WF_CONTINUEMODAL))
    {
        ::DestroyWindow(hWnd);
        hWnd = NULL;
    }

    if (hTemplate != NULL)
    {
        GlobalUnlock(hTemplate);
        GlobalFree(hTemplate);
    }

    // help with error diagnosis (only if WM_INITDIALOG didn't EndDialog())
    if (hWnd == NULL)
    {
#ifdef _DEBUG
        if (dwOldFlags & WF_CONTINUEMODAL)
        {
            if (afxOccManager == NULL)
            {
                TRACE(traceAppMsg, 0, ">>> If this dialog has OLE controls:\n");
                TRACE(traceAppMsg, 0, ">>> AfxEnableControlContainer has not been called yet.\n");
                TRACE(traceAppMsg, 0, ">>> You should call it in your app's InitInstance function.\n");
            }
            else if (dwError != 0)
            {
                TRACE(traceAppMsg, 0, "Warning: Dialog creation failed!  GetLastError returns 0x%8.8X\n", dwError);
            }
        }
#endif //_DEBUG
        return FALSE;
    }

    ASSERT(hWnd == m_hWnd);
    return TRUE;
}

BOOL CWnd::CreateRunDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, window* pParentWnd, HINSTANCE hInst)
{
    BOOL bRet = CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst);

    if (bRet)
    {
        if (m_nFlags & WF_CONTINUEMODAL)
        {
            // enter modal loop
            DWORD dwFlags = MLF_SHOWONIDLE;
            if (GetStyle() & DS_NOIDLEMSG)
                dwFlags |= MLF_NOIDLEMSG;
            VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
        }

        // hide the window before enabling the parent, etc.
        if (m_hWnd != NULL)
            SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW |
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    return bRet;
}

BOOL CDialog::SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo)
{
    m_pOccDialogInfo = pOccDialogInfo;
    return TRUE;
}

_AFX_OCC_DIALOG_INFO* CDialog::GetOccDialogInfo()
{
    return m_pOccDialogInfo;
}

/////////////////////////////////////////////////////////////////////////////
// modal dialogs

// Modal Constructors just save parameters
dialog::dialog(const wchar_t* template_name, window* parent)
{
    ASSERT(IS_INTRESOURCE(lpszTemplateName) ||
        AfxIsValidString(lpszTemplateName));

    initialize();

    _parent = parent;
    _template_name = template_name;
    if (IS_INTRESOURCE(_template_name)) {
        _id_help = LOWORD((DWORD_PTR)_template_name);
    }
}

dialog::dialog(unsigned int template_id, window* parent)
{
    initialize();

    _parent = parent;
    _template_name = MAKEINTRESOURCE(template_id);
    _id_help = template_id;
}

bool dialog::init_modal_indirect(HGLOBAL h, window* parent)
{
    // must be called on an empty constructed CDialog
    ASSERT(m_lpszTemplateName == NULL);
    ASSERT(m_hDialogTemplate == NULL);
    ASSERT(hDialogTemplate != NULL);

    if (m_pParentWnd == NULL)
        m_pParentWnd = pParentWnd;

    m_hDialogTemplate = hDialogTemplate;

    return TRUE;    // always ok (DoModal actually brings up dialog)
}

BOOL CDialog::InitModalIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd,
    void* lpDialogInit)
{
    // must be called on an empty constructed CDialog
    ASSERT(m_lpszTemplateName == NULL);
    ASSERT(m_lpDialogTemplate == NULL);
    ASSERT(lpDialogTemplate != NULL);

    if (m_pParentWnd == NULL)
        m_pParentWnd = pParentWnd;

    m_lpDialogTemplate = lpDialogTemplate;
    m_lpDialogInit = lpDialogInit;

    return TRUE;    // always ok (DoModal actually brings up dialog)
}

HWND dialog::pre_modal()
{
    // cannot call DoModal on a dialog already constructed as modeless
    ASSERT(m_hWnd == NULL);

    // allow OLE servers to disable themselves
    CWinApp* pApp = AfxGetApp();
    if (pApp != NULL)
        pApp->EnableModeless(FALSE);

    // find parent HWND
    HWND hWnd = CWnd::GetSafeOwner_(m_pParentWnd->GetSafeHwnd(), &m_hWndTop);

    // hook for creation of dialog
    AfxHookWindowCreate(this);

    // return window to use as parent for dialog
    return hWnd;
}

void dialog::post_modal()
{
    AfxUnhookWindowCreate();   // just in case
    Detach();               // just in case

                            // re-enable windows
    if (::IsWindow(m_hWndTop))
        ::EnableWindow(m_hWndTop, TRUE);
    m_hWndTop = NULL;
    CWinApp* pApp = AfxGetApp();
    if (pApp != NULL)
        pApp->EnableModeless(TRUE);
}

INT_PTR dialog::do_modal()
{
    // can be constructed with a resource template or InitModalIndirect
    assert(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL || m_lpDialogTemplate != NULL);

    // load resource as necessary
    LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
    HGLOBAL hDialogTemplate = m_hDialogTemplate;
    HINSTANCE hInst = instance();
    if (m_lpszTemplateName != NULL) {
        hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
        HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
        hDialogTemplate = LoadResource(hInst, hResource);
    }
    if (hDialogTemplate != NULL) {
        lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
    }

    // return -1 in case of failure to load the dialog template resource
    if (lpDialogTemplate == NULL) {
        return -1;
    }

    // disable parent (before creating dialog)
    HWND hWndParent = pre_modal();
    AfxUnhookWindowCreate();
    BOOL bEnableParent = FALSE;
    CWnd* pMainWnd = NULL;
    BOOL bEnableMainWnd = FALSE;
    if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent)) {

        ::EnableWindow(hWndParent, FALSE);
        bEnableParent = TRUE;
        pMainWnd = AfxGetMainWnd();
        if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled()) {

            // We are hosted by non-MFC container
            pMainWnd->EnableWindow(FALSE);
            bEnableMainWnd = TRUE;
        }
    }

    try {

        // create modeless dialog
        AfxHookWindowCreate(this);
        if (!CreateRunDlgIndirect(lpDialogTemplate, CWnd::FromHandle(hWndParent), hInst) && !m_bClosedByEndDialog)
        {
            // If the resource handle is a resource-only DLL, the dialog may fail to launch. Use the
            // module instance handle as the fallback dialog creator instance handle if necessary.
            CreateRunDlgIndirect(lpDialogTemplate, CWnd::FromHandle(hWndParent), AfxGetInstanceHandle());
        }

        m_bClosedByEndDialog = FALSE;
    }
    catch(std::exception e) {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed.\n");
        DELETE_EXCEPTION(e);
        m_nModalResult = -1;
    }
    catch (...) {
        ;
    }

    if (bEnableMainWnd) {
        pMainWnd->EnableWindow(TRUE);
    }

    if (bEnableParent) {
        ::EnableWindow(hWndParent, TRUE);
    }
    if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd) {
        ::SetActiveWindow(hWndParent);
    }

    // destroy modal window
    destroy_window();
    post_modal();

    // unlock/free resources as necessary
    if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
        UnlockResource(hDialogTemplate);
    if (m_lpszTemplateName != NULL)
        FreeResource(hDialogTemplate);

    return m_nModalResult;
}

void dialog::end_dialog(int result)
{
    assert(::IsWindow(hwnd()));

    m_bClosedByEndDialog = TRUE;
    if (m_nFlags & (WF_MODALLOOP | WF_CONTINUEMODAL)) {
        EndModalLoop(nResult);
    }
    ::EndDialog(hwnd(), result);
}

/////////////////////////////////////////////////////////////////////////////
// Standard CDialog implementation

void CDialog::OnSetFont(CFont* pFont, BOOL /*bRedraw*/)
{
    OnSetFont(pFont);
    Default();
}

void CDialog::PreInitDialog()
{
    // ignore it
}

LRESULT CDialog::HandleInitDialog(WPARAM, LPARAM)
{
    PreInitDialog();

    // create OLE controls
    COccManager* pOccManager = afxOccManager;
    if ((pOccManager != NULL) && (m_pOccDialogInfo != NULL))
    {
        BOOL bDlgInit;
        if (m_lpDialogInit != NULL)
            bDlgInit = pOccManager->CreateDlgControls(this, m_lpDialogInit,
                m_pOccDialogInfo);
        else
            bDlgInit = pOccManager->CreateDlgControls(this, m_lpszTemplateName,
                m_pOccDialogInfo);

        if (!bDlgInit)
        {
            TRACE(traceAppMsg, 0, "Warning: CreateDlgControls failed during dialog init.\n");
            EndDialog(-1);
            return FALSE;
        }
    }

    // Default will call the dialog proc, and thus OnInitDialog
    LRESULT bResult = Default();

    if (bResult && (m_nFlags & WF_OLECTLCONTAINER))
    {
        if (m_pCtrlCont != NULL)
        {
            m_pCtrlCont->m_pSiteFocus = NULL;
        }

        CWnd* pWndNext = GetNextDlgTabItem(NULL);
        if (pWndNext != NULL)
        {
            pWndNext->SetFocus();   // UI Activate OLE control
            bResult = FALSE;

        }
    }

    return bResult;
}

BOOL AFXAPI AfxHelpEnabled()
{
    if (AfxGetApp() == NULL)
        return FALSE;

    // help is enabled if the app has a handler for ID_HELP
    AFX_CMDHANDLERINFO info;

    // check main window first
    CWnd* pWnd = AfxGetMainWnd();
    if (pWnd != NULL && pWnd->OnCmdMsg(ID_HELP, CN_COMMAND, NULL, &info))
        return TRUE;

    // check app last
    return AfxGetApp()->OnCmdMsg(ID_HELP, CN_COMMAND, NULL, &info);
}

void CDialog::OnSetFont(CFont*)
{
    // ignore it
}

BOOL CDialog::OnInitDialog()
{
    // execute dialog RT_DLGINIT resource
    BOOL bDlgInit;
    if (m_lpDialogInit != NULL)
        bDlgInit = ExecuteDlgInit(m_lpDialogInit);
    else
        bDlgInit = ExecuteDlgInit(m_lpszTemplateName);

    if (!bDlgInit)
    {
        TRACE(traceAppMsg, 0, "Warning: ExecuteDlgInit failed during dialog init.\n");
        EndDialog(-1);
        return FALSE;
    }

    LoadDynamicLayoutResource(m_lpszTemplateName);

    // transfer data into the dialog from member variables
    if (!UpdateData(FALSE))
    {
        TRACE(traceAppMsg, 0, "Warning: UpdateData failed during dialog init.\n");
        EndDialog(-1);
        return FALSE;
    }

    // enable/disable help button automatically
    CWnd* pHelpButton = GetDlgItem(ID_HELP);
    if (pHelpButton != NULL)
        pHelpButton->ShowWindow(AfxHelpEnabled() ? SW_SHOW : SW_HIDE);

    return TRUE;    // set focus to first one
}

void dialog::on_oK()
{
    if (!UpdateData(TRUE))
    {
        TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
        // the UpdateData routine will set focus to correct item
        return;
    }
    end_dialog(IDOK);
}

void dialog::on_cancel()
{
    end_dialog(IDCANCEL);
}

BOOL dialog::check_auto_center()
{
    // load resource as necessary
    LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
    HGLOBAL hDialogTemplate = m_hDialogTemplate;
    if (m_lpszTemplateName != NULL)
    {
        HINSTANCE hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
        HRSRC hResource = ::FindResourceW(hInst, m_lpszTemplateName, RT_DIALOG);
        hDialogTemplate = LoadResource(hInst, hResource);
    }
    if (hDialogTemplate != NULL)
        lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

    // determine if dialog should be centered
    BOOL bResult = TRUE;

    if (lpDialogTemplate != NULL)
    {
        DWORD dwStyle = lpDialogTemplate->style;
        short x;
        short y;

        if (((DLGTEMPLATEEX*)lpDialogTemplate)->signature == 0xFFFF)
        {
            // it's a DIALOGEX resource
            dwStyle = ((DLGTEMPLATEEX*)lpDialogTemplate)->style;
            x = ((DLGTEMPLATEEX*)lpDialogTemplate)->x;
            y = ((DLGTEMPLATEEX*)lpDialogTemplate)->y;
        }
        else
        {
            // it's a DIALOG resource
            x = lpDialogTemplate->x;
            y = lpDialogTemplate->y;
        }

        bResult = !(dwStyle & (DS_CENTER | DS_CENTERMOUSE | DS_ABSALIGN)) && x == 0 && y == 0;
    }

    // unlock/free resources as necessary
    if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
        UnlockResource(hDialogTemplate);
    if (m_lpszTemplateName != NULL)
        FreeResource(hDialogTemplate);

    return bResult; // TRUE if auto-center is ok
}

/////////////////////////////////////////////////////////////////////////////
// CDialog support for context sensitive help.

LRESULT dialog::on_command_help(WPARAM, LPARAM lParam)
{
    if (lParam == 0 && m_nIDHelp != 0)
        lParam = HID_BASE_RESOURCE + m_nIDHelp;
    if (lParam != 0)
    {
        CWinApp* pApp = AfxGetApp();
        if (pApp != NULL)
            pApp->WinHelpInternal(lParam);
        return TRUE;
    }
    return FALSE;
}

LRESULT dialog::on_help_hit_test(WPARAM, LPARAM)
{
    if (_id_help != 0)
        return HID_BASE_RESOURCE + m_nIDHelp;
    return 0;
}