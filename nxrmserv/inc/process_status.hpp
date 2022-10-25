
#ifndef __NX_PROCESS_STATUS_H__
#define __NX_PROCESS_STATUS_H__


#include <string>
#include <vector>
#include <memory>
#include <map>


namespace NX {


typedef enum RIGHT_ID {
    RIGHT_VIEW = 0,
    RIGHT_EDIT,
    RIGHT_PRINT,
    RIGHT_CLIPBOARD,
    RIGHT_SAVEAS,
    RIGHT_DECRYPT,
    RIGHT_SCREENCAP,
    RIGHT_SEND,
    RIGHT_CLASSIFY,
    RIGHT_MAX
} RIGHT_ID;

class process_forbidden_rights
{
public:
    process_forbidden_rights();
    ~process_forbidden_rights();

    void disable_right(RIGHT_ID id);
    void enable_right(RIGHT_ID id, bool force = false);

    void increase_forbidden_count(RIGHT_ID id);
    void decrease_forbidden_count(RIGHT_ID id);

    bool is_right_forbidden(RIGHT_ID id) const;

    void clear();
    process_forbidden_rights& operator = (const process_forbidden_rights& other);

    inline const unsigned short* get_rights_counter() const { return _rights_counter; }

private:
    unsigned short  _rights_counter[RIGHT_MAX];
};

class dwm_window
{
public:
    typedef enum WNDSTATUS {
        WndStatusUnknown = -1,
        WndStatusFalse = 0,
        WndStatusTrue = 1
    } WNDSTATUS;

    dwm_window();
    dwm_window(unsigned long h, WNDSTATUS init_status = WndStatusUnknown);
    ~dwm_window();


    dwm_window& operator = (const dwm_window& other);

    inline WNDSTATUS get_status() const { return _status; }
    inline unsigned long get_hwnd() const { return _h; }
    inline bool status_unknown() const { return (WndStatusUnknown == _status); }
    inline bool status_true() const { return (WndStatusTrue == _status); }
    inline bool status_false() const { return (WndStatusFalse == _status); }
    inline void set_status(WNDSTATUS s) { _status = s; }

private:
    unsigned long _h;
    WNDSTATUS     _status;
};

class process_status
{
public:
    process_status();
    process_status(unsigned long id, unsigned long session_id, unsigned __int64 flags, const std::wstring& image_path, const std::wstring& image_publisher);
    ~process_status();

    void add_wnd(unsigned long h, dwm_window::WNDSTATUS init_status = dwm_window::WndStatusUnknown);
    void remove_wnd(unsigned long h);
    void set_wnd_status(unsigned long h, dwm_window::WNDSTATUS s);
    process_status& operator =(const process_status& other);
    void clear();
    void set_process_rights(unsigned __int64 granted_rights);
    unsigned __int64 get_process_rights();

    inline unsigned long get_id() const { return _id; }
    inline unsigned long get_session_id() const { return _session_id; }
    inline unsigned __int64 get_flags() const { return _flags; }
    inline const std::wstring& get_image() const { return _image; }
    inline const std::wstring& get_publisher() const { return _publisher; }
    inline const std::vector<dwm_window>& get_wnds() const { return _wnds; }

    inline void set_flags(unsigned __int64 f) { _flags = f; }
    inline void add_flags(unsigned __int64 f) { _flags |= f; }
    inline void remove_flags(unsigned __int64 f) { _flags &= (~f); }

    inline bool is_flag_on(unsigned __int64 f) const { return (f == (_flags & f)); }


private:
    unsigned long   _id;
    unsigned long   _session_id;
    std::wstring    _image;
    std::wstring    _publisher;
    unsigned __int64 _flags;
    process_forbidden_rights _forbidden_rights;
    std::vector<dwm_window>  _wnds;
    CRITICAL_SECTION _lock;
};

class process_map
{
public:
    process_map();
    ~process_map();

    void add_process(unsigned long process_id, unsigned long session_id, unsigned __int64 flags, const std::wstring& image_path, const std::wstring& image_publisher);
    void remove_process(unsigned long process_id);
    std::shared_ptr<process_status> get_process(unsigned long process_id) const;

    std::vector<unsigned long> get_valid_overlay_windows(unsigned long session_id) const;

    bool process_has_overlay_window(unsigned long process_id) const;

private:
    std::map<unsigned long, std::shared_ptr<process_status>> _map;
    mutable CRITICAL_SECTION _lock;
};

}


#endif