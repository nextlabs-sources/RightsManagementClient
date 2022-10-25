


#ifndef __NXGUI_RESOURCE_HPP__
#define __NXGUI_RESOURCE_HPP__

#include <string>

namespace NXGUI {
namespace resource {


class resource_base
{
public:
    resource_base();
    resource_base(unsigned long lang_id);
    virtual ~resource_base();

    inline bool empty() const noexcept { return (NULL == _h); }
    inline unsigned long language_id() const noexcept { return _lang_id; }

    virtual void load(unsigned int id, HINSTANCE instance=NULL) noexcept = 0;
    virtual void load(const wchar_t* resource_name, HINSTANCE instance=NULL) noexcept = 0;
    virtual void free() noexcept;

    //virtual operator HCURSOR() const { throw std::exception("not a cursor resource"); }
    virtual operator HBITMAP() const { throw std::exception("not a bitmap resource"); }
    virtual operator HICON() const { throw std::exception("not a icon resource"); }
    virtual operator HMENU() const { throw std::exception("not a menu resource"); }
    virtual operator HACCEL() const { throw std::exception("not a accelerator resource"); }

protected:
    void load_ex(const wchar_t* resource_name, const wchar_t* resource_type, HINSTANCE instance) noexcept;

protected:
    HRSRC   _h;
    unsigned long _lang_id;
};

class cursor : public resource_base
{
public:
    cursor();
    cursor(unsigned long lang_id);
    virtual ~cursor();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class bitmap : public resource_base
{
public:
    bitmap();
    bitmap(unsigned long lang_id);
    virtual ~bitmap();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class icon : public resource_base
{
public:
    icon();
    icon(unsigned long lang_id);
    virtual ~icon();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class menu : public resource_base
{
public:
    menu();
    menu(unsigned long id, HINSTANCE instance = NULL, unsigned long lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    virtual ~menu();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
    virtual void free() noexcept;

    void create() noexcept;
    void create_popup() noexcept;
    void attach(HMENU h) noexcept;
    HMENU detach() noexcept;

    int item_count() const noexcept;
    bool append_item(unsigned int id, const std::wstring& name, bool disabled=false) noexcept;
    bool append_bmp_item(unsigned int id, HBITMAP hbmp, bool disabled=false) noexcept;
    bool append_check_item(unsigned int id, HBITMAP hbmp_check, HBITMAP hbmp_uncheck, const std::wstring& name, bool checked=false, bool disabled=false) noexcept;
    bool append_separator() noexcept;
    bool append_submenu(HMENU h, const std::wstring& name) noexcept;

    bool insert_item(unsigned int pos, unsigned int id, const std::wstring& name, bool disabled=false) noexcept;
    bool insert_bmp_item(unsigned int pos, unsigned int id, HBITMAP hbmp, bool disabled=false) noexcept;
    bool insert_check_item(unsigned int pos, unsigned int id, HBITMAP hbmp_check, HBITMAP hbmp_uncheck, const std::wstring& name, bool checked=false, bool disabled=false) noexcept;
    bool insert_separator(unsigned int pos) noexcept;
    bool insert_submenu(unsigned int pos, HMENU h, const std::wstring& name) noexcept;

    HMENU get_sub_menu(unsigned int pos) noexcept;
    unsigned int get_menu_item_id(unsigned int pos) noexcept;

    bool remove_item(unsigned int id, bool by_pos = false) noexcept;
    bool delete_item(unsigned int id, bool by_pos = false) noexcept;
    bool enable_item(unsigned int id, bool by_pos = false) noexcept;
    bool disable_item(unsigned int id, bool by_pos = false) noexcept;
    std::wstring get_item_name(unsigned int id, bool by_pos = false) noexcept;
    bool change_item_name(unsigned int id, const std::wstring& name, bool by_pos = false) noexcept;
    bool change_item_bitmap(unsigned int id, HBITMAP h, bool by_pos = false) noexcept;

    // check state
    unsigned int state(unsigned int id, bool by_pos = false) const noexcept;
    inline bool exists(unsigned int id, bool by_pos = false) const noexcept { return state_valid(state(id, by_pos)); }
    inline bool is_checked(unsigned int id, bool by_pos = false) const noexcept { return state_checked(state(id, by_pos)); }
    inline bool is_disabled(unsigned int id, bool by_pos = false) const noexcept { return state_disabled(state(id, by_pos)); }
    inline bool is_grayed(unsigned int id, bool by_pos = false) const noexcept { return state_grayed(state(id, by_pos)); }
    inline bool is_highlighted(unsigned int id, bool by_pos = false) const noexcept { return state_highlighted(state(id, by_pos)); }
    inline bool is_menubreak(unsigned int id, bool by_pos = false) const noexcept { return state_menubreak(state(id, by_pos)); }
    inline bool is_ownerdraw(unsigned int id, bool by_pos = false) const noexcept { return state_ownerdraw(state(id, by_pos)); }
    inline bool is_popup(unsigned int id, bool by_pos = false) const noexcept { return state_popup(state(id, by_pos)); }
    inline bool is_separator(unsigned int id, bool by_pos = false) const noexcept { return state_separator(state(id, by_pos)); }
    inline bool state_valid(unsigned int st) const noexcept { return ((unsigned int)-1 != st); }
    inline bool state_checked(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_CHECKED)); }
    inline bool state_disabled(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_DISABLED)); }
    inline bool state_grayed(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_GRAYED)); }
    inline bool state_highlighted(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_HILITE)); }
    inline bool state_menubreak(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & (MF_MENUBARBREAK | MF_MENUBREAK))); }
    inline bool state_ownerdraw(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_OWNERDRAW)); }
    inline bool state_popup(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_POPUP)); }
    inline bool state_separator(unsigned int st) const noexcept { return (state_valid(st) && 0 != (st & MF_SEPARATOR)); }


private:
    bool _destroy_required;
};

class dialog : public resource_base
{
public:
    dialog();
    dialog(unsigned long lang_id);
    virtual ~dialog();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class string : public resource_base
{
public:
    string();
    string(unsigned long lang_id);
    virtual ~string();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class fontdir : public resource_base
{
public:
    fontdir();
    fontdir(unsigned long lang_id);
    virtual ~fontdir();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class font : public resource_base
{
public:
    font();
    font(unsigned long lang_id);
    virtual ~font();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class accelerator : public resource_base
{
public:
    accelerator();
    accelerator(unsigned long lang_id);
    virtual ~accelerator();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class rcdata : public resource_base
{
public:
    rcdata();
    rcdata(unsigned long lang_id);
    virtual ~rcdata();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class messagetable : public resource_base
{
public:
    messagetable();
    messagetable(unsigned long lang_id);
    virtual ~messagetable();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class html : public resource_base
{
public:
    html();
    html(unsigned long lang_id);
    virtual ~html();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class version : public resource_base
{
public:
    version();
    version(unsigned long lang_id);
    virtual ~version();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class group_cursor : public resource_base
{
public:
    group_cursor();
    group_cursor(unsigned long lang_id);
    virtual ~group_cursor();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};

class group_icon : public resource_base
{
public:
    group_icon();
    group_icon(unsigned long lang_id);
    virtual ~group_icon();

    virtual void load(unsigned int id, HINSTANCE instance = NULL) noexcept;
    virtual void load(const  wchar_t* template_name, HINSTANCE instance = NULL) noexcept;
};


}   // NXGUI::resource
}   // NXGUI


#endif