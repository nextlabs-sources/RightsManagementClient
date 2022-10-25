


#ifndef __NXGUI_WIN_BASE_HPP__
#define __NXGUI_WIN_BASE_HPP__


#include <Windows.h>

#include <string>

namespace NXGUI {

#define NXGUI_WND           L"NWnd"
#define NXGUI_WNDCONTROLBAR L"NControlBar"
#define NXGUI_WNDOLECONTROL L"NOleControl"


#define DECLARE_DEFUALT_MSG_HANDLER(FUNC)   \
    virtual bool FUNC(WPARAM wparam, LPARAM lparam, LRESULT& result) {   \
                        (wparam);       \
                        (lparam);       \
                        (result);       \
                        return false;   \
                }

class window
{
public:
    window();
    virtual ~window();

    static ATOM register_wnd_class(const wchar_t* class_name, DWORD style, UINT icon_id = 0xFFFFFFFF, UINT menu_id = 0xFFFFFFFF);

public:
    inline bool is_child() throw() { return (_h && (::GetWindowLongW(_h, GWL_STYLE) & WS_CHILD)); }

    inline HWND hwnd() noexcept { return _h; }
    inline operator HWND() const noexcept { return _h; }

public:
    // for child windows, views, panes etc
    virtual bool create(const std::wstring& class_name,
                        const std::wstring& window_name,
                        unsigned long style,
                        const RECT& rect,
                        window* parent_wnd,
                        unsigned int menu_id) noexcept;

    // advanced creation (allows access to extended styles)
    virtual bool create2(unsigned long style_ex,
                         const std::wstring& class_name,
                         const std::wstring& window_name,
                         unsigned long style,
                         int x, int y, int width, int height,
                         HWND parent_hwnd,
                         HMENU hmenu,
                         LPVOID param = NULL) noexcept;

    virtual bool create2(unsigned long style_ex,
                         const std::wstring& class_name,
                         const std::wstring& window_name,
                         unsigned long style,
                         const RECT& rect,
                         window* parent_wnd,
                         unsigned int menu_id,
                         LPVOID param = NULL) noexcept;
    
    // for wrapping OLE controls
    //bool create_control(REFCLSID clsid, const std::wstring& window_name, unsigned long style, const RECT& rect, window* parent_wnd, unsigned int id);
    //bool create_control(const std::wstring& class_name, const std::wstring& window_name, unsigned long style, const RECT& rect, window* parent_wnd, unsigned int id);
    //// Another overload for creating controls that use default extents.
    //bool create_control(REFCLSID clsid, const std::wstring& window_name, unsigned long style, const POINT* ppt, const SIZE* psize, window* parent_wnd, unsigned int id);


    void attach(HWND h);
    void detach(bool force=false);

    //virtual void pre_subclass_window();

    LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    HWND get_parent();
    HWND get_top_level_parent();
    HWND get_top_level_window();

    void destroy_window();

    // is this program double-bytes-character-set
    bool is_dbcs();

protected:
    // for creating dialogs and dialog-like windows
    bool create_dlg(const wchar_t* template_name, window* parent_wnd);
    bool create_dlg_indirect(LPCDLGTEMPLATE dialog_template, window* parent_wnd, HINSTANCE instance);
    bool create_run_dlg_indirect(LPCDLGTEMPLATE dialog_template, window* parent_wnd, HINSTANCE instance);

    //virtual bool destroy_window();

    //// special pre-creation and window rect adjustment hooks
    //virtual bool pre_create_window(CREATESTRUCT& cs);

    //// Advanced: virtual AdjustWindowRect
    //enum AdjustType { adjustBorder = 0, adjustOutside = 1 };
    //virtual void calculate_window_rect(LPRECT client_rect, unsigned int adjust_type = adjustBorder);

    //// Window tree access
    //int get_dlg_ctrl_id() const;
    //int set_dlg_ctrl_id(int id);
    //window* get_dlg_item(int id) const;
    //void get_dlg_item(int id, HWND* ph) const;

protected:
    bool hook_window_create();
    void unhook_window_create();
    LRESULT next_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

protected:
    bool on_msg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT& result);

    // Window-Management message handler member functions
    virtual bool on_command(WPARAM wparam, LPARAM lparam) { (wparam); (lparam); return false; }
    virtual bool on_notify(LPNMHDR lpnmh, LRESULT& result) { (lpnmh); (result); return false; }
    virtual bool on_init_dialog() { return false; }

    //
    //  Message handlers
    //
    DECLARE_DEFUALT_MSG_HANDLER(on_activate)
    DECLARE_DEFUALT_MSG_HANDLER(on_activate_app)
    DECLARE_DEFUALT_MSG_HANDLER(on_activate_top_level)
    DECLARE_DEFUALT_MSG_HANDLER(on_cancel_mode)
    DECLARE_DEFUALT_MSG_HANDLER(on_child_activate)
    DECLARE_DEFUALT_MSG_HANDLER(on_close)
    DECLARE_DEFUALT_MSG_HANDLER(on_context_menu)
    DECLARE_DEFUALT_MSG_HANDLER(on_copy_data)
    DECLARE_DEFUALT_MSG_HANDLER(on_create)
    DECLARE_DEFUALT_MSG_HANDLER(on_ctl_color)
    DECLARE_DEFUALT_MSG_HANDLER(on_destroy)
    DECLARE_DEFUALT_MSG_HANDLER(on_enable)
    DECLARE_DEFUALT_MSG_HANDLER(on_end_session)
    DECLARE_DEFUALT_MSG_HANDLER(on_enter_idle)
    DECLARE_DEFUALT_MSG_HANDLER(on_erase_bkgnd)
    DECLARE_DEFUALT_MSG_HANDLER(on_get_minmax_info)
    DECLARE_DEFUALT_MSG_HANDLER(on_help_info)
    DECLARE_DEFUALT_MSG_HANDLER(on_icon_erase_bkgnd)
    DECLARE_DEFUALT_MSG_HANDLER(on_kill_focus)
    DECLARE_DEFUALT_MSG_HANDLER(on_menu_char)
    DECLARE_DEFUALT_MSG_HANDLER(on_menu_select)
    DECLARE_DEFUALT_MSG_HANDLER(on_move)
    DECLARE_DEFUALT_MSG_HANDLER(on_paint)
    DECLARE_DEFUALT_MSG_HANDLER(on_sync_paint)
    DECLARE_DEFUALT_MSG_HANDLER(on_parent_notify)
    DECLARE_DEFUALT_MSG_HANDLER(on_notify_format)
    DECLARE_DEFUALT_MSG_HANDLER(on_query_drag_icon)
    DECLARE_DEFUALT_MSG_HANDLER(on_query_end_session)
    DECLARE_DEFUALT_MSG_HANDLER(on_query_new_palette)
    DECLARE_DEFUALT_MSG_HANDLER(on_query_open)
    DECLARE_DEFUALT_MSG_HANDLER(on_set_focus)
    DECLARE_DEFUALT_MSG_HANDLER(on_show_window)
    DECLARE_DEFUALT_MSG_HANDLER(on_size)
    DECLARE_DEFUALT_MSG_HANDLER(on_tcard)
    DECLARE_DEFUALT_MSG_HANDLER(on_window_pos_changing)
    DECLARE_DEFUALT_MSG_HANDLER(on_window_pos_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_session_change)
    // ui
    DECLARE_DEFUALT_MSG_HANDLER(on_change_ui_state)
    DECLARE_DEFUALT_MSG_HANDLER(on_update_ui_state)
    DECLARE_DEFUALT_MSG_HANDLER(on_query_ui_state)
    // nc
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_activate)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_calc_size)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_create)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_destroy)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_hit_test)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_lbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_lbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_lbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_mbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_mbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_mbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_mouse_hover)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_mouse_leave)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_mouse_move)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_paint)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_rbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_rbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_rbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_xbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_xbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_xbutton_dblclk)
    // System message handler member functions
    DECLARE_DEFUALT_MSG_HANDLER(on_drop_files)
    DECLARE_DEFUALT_MSG_HANDLER(on_palette_is_changing)
    DECLARE_DEFUALT_MSG_HANDLER(on_sys_char)
    DECLARE_DEFUALT_MSG_HANDLER(on_sys_command)
    DECLARE_DEFUALT_MSG_HANDLER(on_sys_dead_char)
    DECLARE_DEFUALT_MSG_HANDLER(on_sys_key_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_sys_key_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_app_command)
#if(_WIN32_WINNT >= 0x0501)
    DECLARE_DEFUALT_MSG_HANDLER(on_raw_input)
#endif
    DECLARE_DEFUALT_MSG_HANDLER(on_compacting)
    DECLARE_DEFUALT_MSG_HANDLER(on_dev_mode_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_font_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_palette_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_spooler_status)
    DECLARE_DEFUALT_MSG_HANDLER(on_sys_color_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_time_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_setting_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_win_ini_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_power_broadcast)
    DECLARE_DEFUALT_MSG_HANDLER(on_user_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_input_lang_change)
    DECLARE_DEFUALT_MSG_HANDLER(on_input_lang_change_request)
    DECLARE_DEFUALT_MSG_HANDLER(on_input_device_change)
    // system
    DECLARE_DEFUALT_MSG_HANDLER(on_char)
    DECLARE_DEFUALT_MSG_HANDLER(on_deadchar)
    DECLARE_DEFUALT_MSG_HANDLER(on_unichar)
    DECLARE_DEFUALT_MSG_HANDLER(on_hscroll)
    DECLARE_DEFUALT_MSG_HANDLER(on_vscroll)
    DECLARE_DEFUALT_MSG_HANDLER(on_key_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_key_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_hot_key)
    DECLARE_DEFUALT_MSG_HANDLER(on_lbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_lbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_lbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_mbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_mbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_mbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_xbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_xbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_xbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_mouse_activate)
    DECLARE_DEFUALT_MSG_HANDLER(on_mouse_hover)
    DECLARE_DEFUALT_MSG_HANDLER(on_mouse_leave)
    DECLARE_DEFUALT_MSG_HANDLER(on_mouse_move)
    DECLARE_DEFUALT_MSG_HANDLER(on_mouse_hwheel)
    DECLARE_DEFUALT_MSG_HANDLER(on_mouse_wheel)
    DECLARE_DEFUALT_MSG_HANDLER(on_rbutton_dblclk)
    DECLARE_DEFUALT_MSG_HANDLER(on_rbutton_down)
    DECLARE_DEFUALT_MSG_HANDLER(on_rbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_set_cursor)
    DECLARE_DEFUALT_MSG_HANDLER(on_timer)
    // Initialization message handler member functions
    DECLARE_DEFUALT_MSG_HANDLER(on_init_menu)
    DECLARE_DEFUALT_MSG_HANDLER(on_init_menu_popup)
    DECLARE_DEFUALT_MSG_HANDLER(on_uninit_menu_popup)
    // Clipboard message handler member functions
    DECLARE_DEFUALT_MSG_HANDLER(on_ask_cb_format_name)
    DECLARE_DEFUALT_MSG_HANDLER(on_change_cb_chain)
    DECLARE_DEFUALT_MSG_HANDLER(on_destroy_clipboard)
    DECLARE_DEFUALT_MSG_HANDLER(on_draw_clipboard)
    DECLARE_DEFUALT_MSG_HANDLER(on_hscroll_clipboard)
    DECLARE_DEFUALT_MSG_HANDLER(on_paint_clipboard)
    DECLARE_DEFUALT_MSG_HANDLER(on_render_all_formats)
    DECLARE_DEFUALT_MSG_HANDLER(on_render_format)
    DECLARE_DEFUALT_MSG_HANDLER(on_size_clipboard)
    DECLARE_DEFUALT_MSG_HANDLER(on_vscroll_clipboard)
    DECLARE_DEFUALT_MSG_HANDLER(on_clipboard_update)
    // Control message handler member functions
    DECLARE_DEFUALT_MSG_HANDLER(on_compare_item)
    DECLARE_DEFUALT_MSG_HANDLER(on_delete_item)
    DECLARE_DEFUALT_MSG_HANDLER(on_draw_item)
    DECLARE_DEFUALT_MSG_HANDLER(on_get_dlg_code)
    DECLARE_DEFUALT_MSG_HANDLER(on_measure_item)
    DECLARE_DEFUALT_MSG_HANDLER(on_char_to_item)
    DECLARE_DEFUALT_MSG_HANDLER(on_vkey_to_item)
    // MDI message handler member functions
    DECLARE_DEFUALT_MSG_HANDLER(on_mdi_activate)
    // Menu loop notification messages
    DECLARE_DEFUALT_MSG_HANDLER(on_enter_menu_loop)
    DECLARE_DEFUALT_MSG_HANDLER(on_exit_menu_loop)
    DECLARE_DEFUALT_MSG_HANDLER(on_menu_rbutton_up)
    DECLARE_DEFUALT_MSG_HANDLER(on_menu_drag)
#if(WINVER >= 0x0500)
    DECLARE_DEFUALT_MSG_HANDLER(on_menu_get_object)
#endif
    DECLARE_DEFUALT_MSG_HANDLER(on_menu_command)
    DECLARE_DEFUALT_MSG_HANDLER(on_next_menu)
    // Win4 messages
    DECLARE_DEFUALT_MSG_HANDLER(on_style_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_style_changing)
    DECLARE_DEFUALT_MSG_HANDLER(on_sizing)
    DECLARE_DEFUALT_MSG_HANDLER(on_moving)
    DECLARE_DEFUALT_MSG_HANDLER(on_enter_size_move)
    DECLARE_DEFUALT_MSG_HANDLER(on_exit_size_move)
    DECLARE_DEFUALT_MSG_HANDLER(on_capture_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_device_change)
    // Desktop Windows Manager messages
    DECLARE_DEFUALT_MSG_HANDLER(on_composition_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_nc_rendering_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_colorization_color_changed)
    DECLARE_DEFUALT_MSG_HANDLER(on_window_maximized_change)




    // for modality
    //virtual void begin_modal_state();
    //virtual void end_modal_state();

    //// for translating Windows messages in main message pump
    //virtual bool pre_translate_message(MSG* pMsg);


    //// for processing Windows messages
    //virtual LRESULT window_proc(UINT message, WPARAM wParam, LPARAM lParam);
    //virtual bool on_wnd_msg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //bool NHandleSetCursor(UINT nHitTest, UINT nMsg);

    //// for handling default processing
    //LRESULT Default();
    //virtual LRESULT def_window_proc(UINT message, WPARAM wParam, LPARAM lParam);

    //// for custom cleanup after WM_NCDESTROY
    //virtual void post_nc_destroy();

    //// for notifications from parent
    //virtual bool on_child_notify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //// return TRUE if parent should not process this message
    //bool reflect_child_notify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //static bool PASCAL reflect_last_msg(HWND hWndChild, LRESULT* pResult = NULL);



protected:
    // standard message implementation
    //virtual LRESULT on_nt_ctl_color(WPARAM wParam, LPARAM lParam);
    //virtual LRESULT on_display_change(WPARAM wParam, LPARAM lParam);
    //virtual LRESULT on_drag_list(WPARAM wParam, LPARAM lParam);


private:
    HWND        _h;             // window's handle
    HWND        _hp;            // parent window's handle
	UINT        _flags;         // see WF_ flags above
    WNDPROC     _super_proc;    // super wnd proc
};


}



#endif