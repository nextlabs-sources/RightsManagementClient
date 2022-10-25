

#include <Windows.h>
#include <Windowsx.h>
#include <assert.h>

#include "gui\winbase.hpp"


using namespace NXGUI;


namespace NXGUI {
class global
{
public:
    global() : _instance((HINSTANCE)GetModuleHandleW(NULL)), _tls_id(TLS_OUT_OF_INDEXES)
    {
        _tls_id = TlsAlloc();
    }

    ~global()
    {
        if (_tls_id != TLS_OUT_OF_INDEXES) {
            TlsFree(_tls_id);
        }
    }

    inline HINSTANCE instance() const noexcept { return _instance; }
    inline DWORD tls_id() const noexcept { return _tls_id; }

    inline void set_instance(HINSTANCE instance) noexcept { _instance = instance; }

    bool set_tls_data(void* data)
    {
        if (TLS_OUT_OF_INDEXES == _tls_id) {
            return false;
        }
        return TlsSetValue(_tls_id, data) ? true : false;
    }

    void* get_tls_data()
    {
        if (_tls_id == TLS_OUT_OF_INDEXES) {
            return NULL;
        }
        return TlsGetValue(_tls_id);
    }

private:
    HINSTANCE   _instance;
    DWORD       _tls_id;
};

static global GLOBAL;
// handy utilities
static bool is_ime(HWND h, LPCREATESTRUCT lpcs);
static LRESULT WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static LRESULT CALLBACK cbt_filter_hook(int code, WPARAM wparam, LPARAM lparam);

class hook_thread_state
{
public:
    hook_thread_state() : _hhook(NULL), _wnd(NULL)
    {
    }
    hook_thread_state(window* wnd) : _hhook(NULL), _wnd(wnd)
    {
    }
    ~hook_thread_state()
    {
        uninstall_hook();
        _wnd = NULL;
    }

    inline window* wnd() noexcept { return _wnd; }
    inline HHOOK hhook() { return _hhook; }

    bool install_hook()
    {
        if (_wnd == NULL) {
            return false;
        }

        if (!GLOBAL.set_tls_data(this)) {
            return false;
        }

        _hhook  = ::SetWindowsHookExW(WH_CBT, cbt_filter_hook, NULL, ::GetCurrentThreadId());
        if (_hhook == NULL) {
            GLOBAL.set_tls_data(NULL);
            return false;
        }

        return true;
    }

    void uninstall_hook()
    {
        if (_hhook != NULL) {
#ifdef _DEBUG
            hook_thread_state* hts = (hook_thread_state*)GLOBAL.get_tls_data();
            assert(hts == this);
            assert(hts->wnd() != NULL);
            assert(hts->hhook() != NULL);
#endif
            ::UnhookWindowsHookEx(_hhook);
            _hhook = NULL;
            GLOBAL.set_tls_data(NULL);
        }
    }

private:
    HHOOK   _hhook; // CBT Hook
    window* _wnd;   // window is being created
};



}


window::window() : _h(NULL), _hp(NULL), _flags(0), _super_proc(NULL)
{
}

window::~window()
{
}

bool window::create(const std::wstring& class_name,
                    const std::wstring& window_name,
                    unsigned long style,
                    const RECT& rect,
                    window* parent_wnd,
                    unsigned int menu_id) noexcept
{
    return false;
}

// advanced creation (allows access to extended styles)
bool window::create2(unsigned long style_ex,
                     const std::wstring& class_name,
                     const std::wstring& window_name,
                     unsigned long style,
                     int x, int y, int width, int height,
                     HWND parent_hwnd,
                     HMENU hmenu,
                     LPVOID param) noexcept
{
    CREATESTRUCTW    cs;

    assert(!class_name.empty() && !window_name.empty());

    // allow modification of several common create parameters
    memset(&cs, 0, sizeof(CREATESTRUCT));
    cs.dwExStyle = style_ex;
    cs.lpszClass = class_name.empty() ? NULL : class_name.c_str();
    cs.lpszName = window_name.c_str();
    cs.style = style;
    cs.x = x;
    cs.y = y;
    cs.cx = width;
    cs.cy = height;
    cs.hwndParent = parent_hwnd;
    cs.hMenu = hmenu;
    cs.hInstance = GLOBAL.instance();
    cs.lpCreateParams = param;

    //
    //  Precreate
    //
    if (cs.lpszClass == NULL) {
        // no WNDCLASS provided - use child window default
        assert(cs.style & WS_CHILD);
        cs.lpszClass = L"NXGUI::Default";
    }

    if ((cs.hMenu == NULL) && (cs.style & WS_CHILD)) {
        cs.hMenu = (HMENU)(UINT_PTR)this;
    }

    _h = CreateWindowExW(cs.dwExStyle, cs.lpszClass,
                         cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
                         cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
    if (NULL == _h) {
        return false;
    }

    return true;
}

bool window::create2(unsigned long style_ex,
                     const std::wstring& class_name,
                     const std::wstring& window_name,
                     unsigned long style,
                     const RECT& rect,
                     window* parent_wnd,
                     unsigned int menu_id,
                     LPVOID param) noexcept
{
    return false;
}

void window::attach(HWND h)
{
    _h = h;
    _super_proc = (WNDPROC)SetWindowLongPtrW(_h, GWLP_WNDPROC, (DWORD_PTR)NXGUI::WindowProc);
}

void window::detach(bool force)
{
    if (NULL == _h) {
        assert(NULL == _super_proc);
        return;
    }

    if (force || ((DWORD_PTR)NXGUI::WindowProc) == GetWindowLongPtrW(_h, GWLP_WNDPROC)) {
        if (NULL != _super_proc) {
            SetWindowLongPtrW(_h, GWLP_WNDPROC, (DWORD_PTR)_super_proc);
            _super_proc = NULL;
        }
    }
}

ATOM window::register_wnd_class(const wchar_t* class_name, DWORD style, UINT icon_id, UINT menu_id)
{
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(WNDCLASSEX));   // start with NULL defaults
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = (0 == style) ? style : (CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW);
    wc.lpfnWndProc   = DefWindowProcW;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GLOBAL.instance();
    wc.hIcon         = (0xFFFFFFFF == icon_id) ? LoadIconW(NULL, IDI_APPLICATION) : LoadIconW(GLOBAL.instance(), MAKEINTRESOURCEW(icon_id));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = (0xFFFFFFFF== menu_id) ? NULL : MAKEINTRESOURCEW(menu_id);
    wc.lpszClassName = class_name;
    wc.hIconSm       = (0xFFFFFFFF == icon_id) ? LoadIconW(NULL, IDI_APPLICATION) : LoadIconW(GLOBAL.instance(), MAKEINTRESOURCEW(icon_id));

    return RegisterClassExW(&wc);
}

HWND window::get_parent()
{
    return ::GetParent(_h);
}

HWND window::get_top_level_parent()
{
    assert(::IsWindow(_h));

    HWND hWndParent = _h;
    HWND hWndTmp;
    while ((hWndTmp = ::GetParent(hWndParent)) != NULL)
        hWndParent = hWndTmp;

    return hWndParent;
}

HWND window::get_top_level_window()
{
    assert(::IsWindow(_h));

    HWND hWndParent;
    HWND hWndTmp = _h;

    do {
        hWndParent = hWndTmp;
        hWndTmp = (::GetWindowLongW(hWndParent, GWL_STYLE) & WS_CHILD) ? ::GetParent(hWndParent) : ::GetWindow(hWndParent, GW_OWNER);
    } while (hWndTmp != NULL);

    return hWndParent;
}

void window::destroy_window()
{
    ::PostMessageW(_h, WM_DESTROY, 0, 0);
}

LRESULT window::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    // process msg
    if (on_msg(hwnd, msg, wparam, lparam, result)) {
        // msg has been handled
        return result;
    }

    // call default
    return next_wnd_proc(hwnd, msg, wparam, lparam);
}

LRESULT window::next_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return ((NULL == _super_proc) ? ::DefWindowProcW(hwnd, msg, wparam, lparam) : _super_proc(hwnd, msg, wparam, lparam));
}


bool window::is_dbcs()
{
    static int is_dcbs = -1;

    if (-1 == is_dcbs) {
        CPINFO info;
        GetCPInfo(GetOEMCP(), &info);
        is_dcbs = (info.MaxCharSize > 1) ? 1 : 0;
    }

    return (1 == is_dcbs) ? TRUE : FALSE;
}

bool window::hook_window_create()
{
    hook_thread_state* hts = (hook_thread_state*)GLOBAL.get_tls_data();
    if (hts != NULL) {
        return false;
    }

    hts = new hook_thread_state(this);
    if (NULL == hts) {
        return false;
    }

    if (!hts->install_hook()) {
        delete hts;
        hts = NULL;
        return false;
    }

    return true;
}

void window::unhook_window_create()
{
    hook_thread_state* hts = (hook_thread_state*)GLOBAL.get_tls_data();
    if (NULL != hts) {
        hts->uninstall_hook();
    }
}


#define BEGIN_MSG_PROCESS() \
            switch (msg)    \
            {

#define PROCESS_MSG(MSGID, FUNC)    \
            case MSGID: return FUNC(wparam, lparam, result);

#define END_MSG_PROCESS()   \
            default:        \
                break;      \
            }

bool window::on_msg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT& result)
{
    result = 0;

    // special case for commands
    if (msg == WM_COMMAND) {
        if (on_command(wparam, lparam)) {
            result = 1;
            return true;
        }
        return false;
    }

    // special case for notifies
    if (msg == WM_NOTIFY) {
        NMHDR* lpnmh = (NMHDR*)lparam;
        if (lpnmh->hwndFrom != NULL && on_notify(lpnmh, result)) {
            return true;
        }
        return false;
    }

    // special case for set cursor HTERROR
    if (msg == WM_SETCURSOR && on_set_cursor((short)LOWORD(lparam), HIWORD(lparam), result)) {
        result = 1;
        return true;
    }


    // special case for set cursor WM_INITDIALOG
    if (msg == WM_INITDIALOG) {
        if (!on_init_dialog()) {
            result = 0;
            return true;
        }
        return false;
    }

    // Dispatch Message
    BEGIN_MSG_PROCESS()
        // Window-Management messages
        PROCESS_MSG(WM_ACTIVATE, on_activate)
        PROCESS_MSG(WM_ACTIVATEAPP, on_activate_app)
        PROCESS_MSG(WM_CANCELMODE, on_cancel_mode)
        PROCESS_MSG(WM_CHILDACTIVATE, on_child_activate)
        PROCESS_MSG(WM_CLOSE, on_close)
        PROCESS_MSG(WM_CONTEXTMENU, on_context_menu)
        PROCESS_MSG(WM_COPYDATA, on_copy_data)
        PROCESS_MSG(WM_CREATE, on_create)
        PROCESS_MSG(WM_CTLCOLORBTN, on_ctl_color)
        PROCESS_MSG(WM_CTLCOLOREDIT, on_ctl_color)
        PROCESS_MSG(WM_CTLCOLORDLG, on_ctl_color)
        PROCESS_MSG(WM_CTLCOLORLISTBOX, on_ctl_color)
        PROCESS_MSG(WM_CTLCOLORSCROLLBAR, on_ctl_color)
        PROCESS_MSG(WM_CTLCOLORSTATIC, on_ctl_color)
        PROCESS_MSG(WM_DESTROY, on_destroy)
        PROCESS_MSG(WM_ENABLE, on_enable)
        PROCESS_MSG(WM_ENDSESSION, on_end_session)
        PROCESS_MSG(WM_ENTERIDLE, on_enter_idle)
        PROCESS_MSG(WM_ERASEBKGND, on_erase_bkgnd)
        PROCESS_MSG(WM_GETMINMAXINFO, on_get_minmax_info)
        PROCESS_MSG(WM_HELP, on_help_info)
        PROCESS_MSG(WM_ICONERASEBKGND, on_icon_erase_bkgnd)
        PROCESS_MSG(WM_KILLFOCUS, on_kill_focus)
        PROCESS_MSG(WM_MENUCHAR, on_menu_char)
        PROCESS_MSG(WM_MENUSELECT, on_menu_select)
        PROCESS_MSG(WM_MOVE, on_move)
        PROCESS_MSG(WM_PAINT, on_paint)
        PROCESS_MSG(WM_SYNCPAINT, on_sync_paint)
        PROCESS_MSG(WM_PARENTNOTIFY, on_parent_notify)
        PROCESS_MSG(WM_NOTIFYFORMAT, on_notify_format)
        PROCESS_MSG(WM_QUERYDRAGICON, on_query_drag_icon)
        PROCESS_MSG(WM_QUERYENDSESSION, on_query_end_session)
        PROCESS_MSG(WM_QUERYNEWPALETTE, on_query_new_palette)
        PROCESS_MSG(WM_QUERYOPEN, on_query_open)
        PROCESS_MSG(WM_SETFOCUS, on_set_focus)
        PROCESS_MSG(WM_SHOWWINDOW, on_show_window)
        PROCESS_MSG(WM_SIZE, on_size)
        // Nonclient-Area messages
        PROCESS_MSG(WM_NCACTIVATE, on_nc_activate)
        PROCESS_MSG(WM_NCCALCSIZE, on_nc_calc_size)
        PROCESS_MSG(WM_NCCREATE, on_nc_create)
        PROCESS_MSG(WM_NCDESTROY, on_nc_destroy)
        PROCESS_MSG(WM_NCHITTEST, on_nc_hit_test)
        PROCESS_MSG(WM_NCLBUTTONDBLCLK, on_nc_lbutton_dblclk)
        PROCESS_MSG(WM_NCLBUTTONDOWN, on_nc_lbutton_down)
        PROCESS_MSG(WM_NCLBUTTONUP, on_nc_lbutton_up)
        PROCESS_MSG(WM_NCMBUTTONDBLCLK, on_nc_mbutton_dblclk)
        PROCESS_MSG(WM_NCMBUTTONDOWN, on_nc_mbutton_down)
        PROCESS_MSG(WM_NCMBUTTONUP, on_nc_mbutton_up)
        PROCESS_MSG(WM_NCMOUSEHOVER, on_nc_mouse_hover)
        PROCESS_MSG(WM_NCMOUSELEAVE, on_nc_mouse_leave)
        PROCESS_MSG(WM_NCMOUSEMOVE, on_nc_mouse_move)
        PROCESS_MSG(WM_NCPAINT, on_nc_paint)
        PROCESS_MSG(WM_NCRBUTTONDBLCLK, on_nc_rbutton_dblclk)
        PROCESS_MSG(WM_NCRBUTTONDOWN, on_nc_rbutton_down)
        PROCESS_MSG(WM_NCRBUTTONUP, on_nc_rbutton_up)
        PROCESS_MSG(WM_NCXBUTTONDBLCLK, on_nc_xbutton_dblclk)
        PROCESS_MSG(WM_NCXBUTTONDOWN, on_nc_xbutton_down)
        PROCESS_MSG(WM_NCXBUTTONUP, on_nc_xbutton_up)
        // System message
        PROCESS_MSG(WM_DROPFILES, on_drop_files)
        PROCESS_MSG(WM_PALETTEISCHANGING, on_palette_is_changing)
        PROCESS_MSG(WM_SYSCHAR, on_sys_char)
        PROCESS_MSG(WM_SYSCOMMAND, on_sys_command)
        PROCESS_MSG(WM_SYSDEADCHAR, on_sys_dead_char)
        PROCESS_MSG(WM_SYSKEYDOWN, on_sys_key_down)
        PROCESS_MSG(WM_SYSKEYUP, on_sys_key_up)
        PROCESS_MSG(WM_APPCOMMAND, on_app_command)
        PROCESS_MSG(WM_COMPACTING, on_compacting)
        PROCESS_MSG(WM_DEVMODECHANGE, on_dev_mode_change)
        PROCESS_MSG(WM_FONTCHANGE, on_font_change)
        PROCESS_MSG(WM_PALETTECHANGED, on_palette_changed)
        PROCESS_MSG(WM_SPOOLERSTATUS, on_spooler_status)
        PROCESS_MSG(WM_SYSCOLORCHANGE, on_sys_color_change)
        PROCESS_MSG(WM_TIMECHANGE, on_time_change)
        PROCESS_MSG(WM_WININICHANGE, on_win_ini_change)
        PROCESS_MSG(WM_POWERBROADCAST, on_power_broadcast)
        PROCESS_MSG(WM_USERCHANGED, on_user_changed)
        PROCESS_MSG(WM_INPUTLANGCHANGE, on_input_lang_change)
        PROCESS_MSG(WM_INPUTLANGCHANGEREQUEST, on_input_lang_change_request)
        // Input message
        PROCESS_MSG(WM_CHAR, on_char)
        PROCESS_MSG(WM_DEADCHAR, on_deadchar)
        PROCESS_MSG(WM_UNICHAR, on_unichar)
        PROCESS_MSG(WM_HSCROLL, on_hscroll)
        PROCESS_MSG(WM_VSCROLL, on_vscroll)
        PROCESS_MSG(WM_KEYDOWN, on_key_down)
        PROCESS_MSG(WM_HOTKEY, on_hot_key)
        PROCESS_MSG(WM_LBUTTONDBLCLK, on_lbutton_dblclk)
        PROCESS_MSG(WM_LBUTTONDOWN, on_lbutton_down)
        PROCESS_MSG(WM_LBUTTONUP, on_lbutton_up)
        PROCESS_MSG(WM_RBUTTONDBLCLK, on_rbutton_dblclk)
        PROCESS_MSG(WM_RBUTTONDOWN, on_rbutton_down)
        PROCESS_MSG(WM_RBUTTONUP, on_rbutton_up)
        PROCESS_MSG(WM_MBUTTONDBLCLK, on_mbutton_dblclk)
        PROCESS_MSG(WM_MBUTTONDOWN, on_mbutton_down)
        PROCESS_MSG(WM_MBUTTONUP, on_mbutton_up)
        PROCESS_MSG(WM_XBUTTONDBLCLK, on_xbutton_dblclk)
        PROCESS_MSG(WM_XBUTTONDOWN, on_xbutton_down)
        PROCESS_MSG(WM_XBUTTONUP, on_xbutton_up)
        PROCESS_MSG(WM_MOUSEACTIVATE, on_mouse_activate)
        PROCESS_MSG(WM_MOUSEHOVER, on_mouse_hover)
        PROCESS_MSG(WM_MOUSELEAVE, on_mouse_leave)
        PROCESS_MSG(WM_MOUSEMOVE, on_mouse_move)
        PROCESS_MSG(WM_MOUSEHWHEEL, on_mouse_hwheel)
        PROCESS_MSG(WM_MOUSEWHEEL, on_mouse_wheel)
        PROCESS_MSG(WM_SETCURSOR, on_set_cursor)
        PROCESS_MSG(WM_TIMER, on_timer)
        // Initialization messages
        PROCESS_MSG(WM_INITMENU, on_init_menu)
        PROCESS_MSG(WM_INITMENUPOPUP, on_init_menu_popup)
        PROCESS_MSG(WM_UNINITMENUPOPUP, on_uninit_menu_popup)
        // Clipboard message
        PROCESS_MSG(WM_ASKCBFORMATNAME, on_ask_cb_format_name)
        PROCESS_MSG(WM_CHANGECBCHAIN, on_change_cb_chain)
        PROCESS_MSG(WM_DESTROYCLIPBOARD, on_destroy_clipboard)
        PROCESS_MSG(WM_DRAWCLIPBOARD, on_draw_clipboard)
        PROCESS_MSG(WM_HSCROLLCLIPBOARD, on_hscroll_clipboard)
        PROCESS_MSG(WM_VSCROLLCLIPBOARD, on_vscroll_clipboard)
        PROCESS_MSG(WM_PAINTCLIPBOARD, on_paint_clipboard)
        PROCESS_MSG(WM_RENDERALLFORMATS, on_render_all_formats)
        PROCESS_MSG(WM_SIZECLIPBOARD, on_size_clipboard)
        PROCESS_MSG(WM_CLIPBOARDUPDATE, on_clipboard_update)
        // Control message
        PROCESS_MSG(WM_COMPAREITEM, on_compare_item)
        PROCESS_MSG(WM_DELETEITEM, on_delete_item)
        PROCESS_MSG(WM_DRAWITEM, on_draw_item)
        PROCESS_MSG(WM_GETDLGCODE, on_get_dlg_code)
        PROCESS_MSG(WM_MEASUREITEM, on_measure_item)
        PROCESS_MSG(WM_CHARTOITEM, on_char_to_item)
        PROCESS_MSG(WM_VKEYTOITEM, on_vkey_to_item)
        // MDI message
        PROCESS_MSG(WM_MDIACTIVATE, on_mdi_activate)
        // Menu loop notification messages
        PROCESS_MSG(WM_ENTERMENULOOP, on_enter_menu_loop)
        PROCESS_MSG(WM_EXITMENULOOP, on_exit_menu_loop)
        PROCESS_MSG(WM_MENURBUTTONUP, on_menu_rbutton_up)
        PROCESS_MSG(WM_MENUDRAG, on_menu_drag)
#if(WINVER >= 0x0500)
        PROCESS_MSG(WM_MENUGETOBJECT, on_menu_get_object)
#endif
        PROCESS_MSG(WM_MENUCOMMAND, on_menu_command)
        PROCESS_MSG(WM_NEXTMENU, on_next_menu)
        // Win4 messages
        PROCESS_MSG(WM_STYLECHANGED, on_style_changed)
        PROCESS_MSG(WM_STYLECHANGING, on_style_changing)
        PROCESS_MSG(WM_SIZING, on_sizing)
        PROCESS_MSG(WM_MOVING, on_moving)
        PROCESS_MSG(WM_ENTERSIZEMOVE, on_enter_size_move)
        PROCESS_MSG(WM_EXITSIZEMOVE, on_exit_size_move)
        PROCESS_MSG(WM_CAPTURECHANGED, on_capture_changed)
        PROCESS_MSG(WM_DEVICECHANGE, on_device_change)
    END_MSG_PROCESS()

    return false;
}


/////////////////////////////////////////////////////////////////////////////
// Window creation hooks

// Workflow (MUST BE IN SAME THREAD)
//   window.create -> install cbt hook -> enter cbt_filter_hool -> uninstall cbt hook

LRESULT CALLBACK NXGUI::cbt_filter_hook(int code, WPARAM wparam, LPARAM lparam)
{
    hook_thread_state*  hts = NULL;
    window* wnd = NULL;

    // Get hook_thread_state
    hts = (hook_thread_state*)GLOBAL.get_tls_data();
    if (NULL == hts) {
        return 0;   // Allow to create
    }
    assert(NULL != hts->hhook());
    assert(NULL != hts->wnd());

    // we only handle HCBT_CREATEWND
    if (code != HCBT_CREATEWND) {
        // wait for HCBT_CREATEWND just pass others on...
        return CallNextHookEx(hts->hhook(), code, wparam, lparam);
    }


    //
    //  HCBT_CREATEWND
    //
    try {

        HWND h = (HWND)wparam;
        LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lparam)->lpcs;

        // Note: special check to avoid subclassing the IME window
        if (!wnd->is_dbcs() && !NXGUI::is_ime(h, lpcs)) {
            assert(NULL == wnd->hwnd());
            wnd->attach(h);
//            wnd->pre_subclass_window();
        }
    }
    catch (...) {
    }


    // uninstall hook
    hts->uninstall_hook();
    delete hts;
    hts = NULL;
    return 0;
}



//
// do
//
LRESULT WINAPI NXGUI::WindowProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam)
{
    window* wnd = (window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    assert(NULL != wnd);
    if (NULL == wnd) {
        return FALSE;
    }
    return wnd->wnd_proc(hwnd, msg, wparam, lparam);
}

bool NXGUI::is_ime(HWND h, LPCREATESTRUCT lpcs)
{

    // check for cheap CS_IME style first...
    if (GetClassLongW(h, GCL_STYLE) & CS_IME) {
        return true;
    }

    // Get class name of the window that is being created
    LPCWSTR pwzClassName;
    WCHAR   wzClassName[_countof(L"ime") + 1];
    if (DWORD_PTR(lpcs->lpszClass) > 0xFFFF) {
        pwzClassName = lpcs->lpszClass;
    }
    else {
        wzClassName[0] = '\0';
#pragma warning(push)
#pragma warning(disable: 4302) // 'type cast' : truncation from 'LPCSTR' to 'ATOM'
        GlobalGetAtomNameW((ATOM)lpcs->lpszClass, wzClassName, _countof(wzClassName));
#pragma warning(pop)
        pwzClassName = wzClassName;
    }

    return (CSTR_EQUAL == ::CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, pwzClassName, -1, L"ime", -1, NULL, NULL, 0));
}