

#include <Windows.h>

#include <string>
#include <vector>

#include <gui\winres.hpp>


using namespace NXGUI;



//
//   class resource_base
//

resource::resource_base::resource_base() : _h(NULL), _lang_id(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
{
}

resource::resource_base::resource_base(unsigned long lang_id) : _h(NULL), _lang_id(lang_id)
{
}

resource::resource_base::~resource_base()
{
    free();
}

void resource::resource_base::load_ex(const wchar_t* name, const wchar_t* type, HINSTANCE instance) noexcept
{
    _h = ::FindResourceExW((HMODULE)instance, type, name, (WORD)language_id());
}

void resource::resource_base::free() noexcept
{
    _h = NULL; _lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
}

//
//   class cursor
//

resource::cursor::cursor() : resource_base()
{
}

resource::cursor::cursor(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::cursor::~cursor()
{
}

void resource::cursor::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::cursor::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_CURSOR, instance);
}

//
//   class bitmap
//

resource::bitmap::bitmap() : resource_base()
{
}

resource::bitmap::bitmap(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::bitmap::~bitmap()
{
}

void resource::bitmap::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::bitmap::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_BITMAP, instance);
}

//
//   class icon
//

resource::icon::icon() : resource_base()
{
}

resource::icon::icon(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::icon::~icon()
{
}

void resource::icon::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::icon::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_ICON, instance);
}

//
//   class menu
//

resource::menu::menu() : resource_base(), _destroy_required(false)
{
}

resource::menu::menu(unsigned long id, HINSTANCE instance, unsigned long lang_id) : resource_base(lang_id), _destroy_required(false)
{
    load(id, instance);
}

resource::menu::~menu()
{
}

void resource::menu::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::menu::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    free();
    load_ex(name, RT_MENU, instance);
}

void resource::menu::free() noexcept
{
    if (!empty() && _destroy_required) {
        DestroyMenu((HMENU)_h);
    }
    resource_base::free();
}

void resource::menu::create() noexcept
{
    _h = (HRSRC)::CreateMenu();
    _destroy_required = (NULL != _h) ? true : false;
}

void resource::menu::create_popup() noexcept
{
    _h = (HRSRC)::CreatePopupMenu();
    _destroy_required = (NULL != _h) ? true : false;
}

void resource::menu::attach(HMENU h) noexcept
{
    _h = (HRSRC)h;
    _destroy_required = (NULL != _h) ? true : false;
}

HMENU resource::menu::detach() noexcept
{
    HMENU h = (HMENU)_h;
    resource_base::free();
    _destroy_required = false;
    return h;
}

int resource::menu::item_count() const noexcept
{
    return empty() ? 0 : ::GetMenuItemCount((HMENU)_h);
}

bool resource::menu::append_item(unsigned int id, const std::wstring& name, bool disabled) noexcept
{
    unsigned int flags = MF_STRING | MF_BYCOMMAND;
    if (disabled) {
        flags |= (MF_DISABLED | MF_GRAYED);
    }
    return AppendMenuW((HMENU)_h, flags, id, name.c_str()) ? true : false;
}

bool resource::menu::append_bmp_item(unsigned int id, HBITMAP hbmp, bool disabled) noexcept
{
    return insert_bmp_item((unsigned long)-1, id, hbmp, disabled);
}

bool resource::menu::append_check_item(unsigned int id, HBITMAP hbmp_check, HBITMAP hbmp_uncheck, const std::wstring& name, bool checked, bool disabled) noexcept
{
    return insert_check_item((unsigned int)-1, id, hbmp_check, hbmp_uncheck, name, checked, disabled);
}

bool resource::menu::append_separator() noexcept
{
    return AppendMenuW((HMENU)_h, MF_SEPARATOR, -1, NULL) ? true : false;
}

bool resource::menu::append_submenu(HMENU h, const std::wstring& name) noexcept
{
    return AppendMenuW((HMENU)_h, MF_POPUP, (UINT_PTR)h, name.c_str()) ? true : false;
}

bool resource::menu::insert_item(unsigned int pos, unsigned int id, const std::wstring& name, bool disabled) noexcept
{
    unsigned int flags = MF_BYPOSITION | MF_STRING;
    if (disabled) {
        flags |= (MF_DISABLED | MF_GRAYED);
    }
    return InsertMenuW((HMENU)_h, pos, flags, id, name.c_str()) ? true : false;
}

bool resource::menu::insert_bmp_item(unsigned int pos, unsigned int id, HBITMAP hbmp, bool disabled) noexcept
{
    MENUITEMINFOW mii = { 0 };
    memset(&mii, 0, sizeof(MENUITEMINFOW));
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_BITMAP |MIIM_STATE | MIIM_ID;
    mii.fType = MFT_BITMAP;
    mii.fState = disabled ? (MFS_DISABLED | MFS_GRAYED) : MFS_ENABLED;
    mii.wID = (unsigned short)id;
    mii.hSubMenu = NULL;
    mii.hbmpChecked = NULL;
    mii.hbmpUnchecked = NULL;
    mii.dwItemData = 0;
    mii.dwTypeData = 0;
    mii.cch = 0;
    mii.hbmpItem = hbmp;
    return InsertMenuItemW((HMENU)_h, pos, TRUE, &mii) ? true : false;
}

bool resource::menu::insert_check_item(unsigned int pos, unsigned int id, HBITMAP hbmp_check, HBITMAP hbmp_uncheck, const std::wstring& name, bool checked, bool disabled) noexcept
{
    unsigned int state = 0;
    if (disabled) {
        state |= (MFS_DISABLED | MFS_GRAYED);
    }
    if (checked) {
        state |= MFS_CHECKED;
    }
    MENUITEMINFOW mii = { 0 };
    memset(&mii, 0, sizeof(MENUITEMINFOW));
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_CHECKMARKS | MIIM_STRING | MIIM_STATE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.fState = state;
    mii.wID = (unsigned short)id;
    mii.hSubMenu = NULL;
    mii.hbmpChecked = hbmp_check;
    mii.hbmpUnchecked = hbmp_uncheck;
    mii.dwItemData = 0;
    mii.dwTypeData = 0;
    mii.cch = (UINT)name.length();
    mii.hbmpItem = NULL;
    return InsertMenuItemW((HMENU)_h, pos, TRUE, &mii) ? true : false;
}

bool resource::menu::insert_separator(unsigned int pos) noexcept
{
    return InsertMenuW((HMENU)_h, pos, MF_BYPOSITION | MF_SEPARATOR, -1, NULL) ? true : false;
}

bool resource::menu::insert_submenu(unsigned int pos, HMENU h, const std::wstring& name) noexcept
{
    return InsertMenuW((HMENU)_h, pos, MF_BYPOSITION | MF_POPUP, -1, name.c_str()) ? true : false;
}

HMENU resource::menu::get_sub_menu(unsigned int pos) noexcept
{
    return GetSubMenu((HMENU)_h, pos);
}

unsigned int resource::menu::get_menu_item_id(unsigned int pos) noexcept
{
    return GetMenuItemID((HMENU)_h, pos);
}

bool resource::menu::remove_item(unsigned int id, bool by_pos) noexcept
{
    return RemoveMenu((HMENU)_h, id, by_pos ? MF_BYPOSITION : MF_BYCOMMAND) ? true : false;
}

bool resource::menu::delete_item(unsigned int id, bool by_pos) noexcept
{
    return DeleteMenu((HMENU)_h, id, by_pos ? MF_BYPOSITION : MF_BYCOMMAND) ? true : false;
}

bool resource::menu::enable_item(unsigned int id, bool by_pos) noexcept
{
    unsigned int st = state(id, by_pos);
    if (state_separator(st)) {
        return false;
    }
    if (!state_disabled(st)) {
        return true;
    }
    return EnableMenuItem((HMENU)_h, id, by_pos ? (MF_BYPOSITION | MF_ENABLED) : (MF_BYCOMMAND | MF_ENABLED)) ? true : false;
}

bool resource::menu::disable_item(unsigned int id, bool by_pos) noexcept
{
    unsigned int st = state(id, by_pos);
    if (state_separator(st)) {
        return false;
    }
    if (state_disabled(st)) {
        return true;
    }
    return EnableMenuItem((HMENU)_h, id, by_pos ? (MF_BYPOSITION | MF_DISABLED) : (MF_BYCOMMAND | MF_DISABLED)) ? true : false;
}

std::wstring resource::menu::get_item_name(unsigned int id, bool by_pos) noexcept
{
    std::wstring ws;
    int len = GetMenuStringW((HMENU)_h, id, NULL, 0, by_pos ? MF_BYPOSITION : MF_BYCOMMAND);
    if (0 == len) {
        return ws;
    }
    std::vector<wchar_t> buf;
    buf.resize(len + 1, 0);
    len = GetMenuStringW((HMENU)_h, id, &buf[0], (int)buf.size(), by_pos ? MF_BYPOSITION : MF_BYCOMMAND);
    if (len == 0) {
        return ws;
    }

    ws = &buf[0];
    return std::move(ws);
}

bool resource::menu::change_item_name(unsigned int id, const std::wstring& name, bool by_pos) noexcept
{
    return ModifyMenuW((HMENU)_h, id, by_pos ? MF_BYPOSITION : MF_BYCOMMAND, id, name.c_str()) ? true : false;
}

bool resource::menu::change_item_bitmap(unsigned int id, HBITMAP h, bool by_pos) noexcept
{
    return ModifyMenuW((HMENU)_h, id, by_pos ? MF_BYPOSITION : MF_BYCOMMAND, id, (LPCTSTR)h) ? true : false;
}

// check state
unsigned int resource::menu::state(unsigned int id, bool by_pos) const noexcept
{
    return GetMenuState((HMENU)_h, id, by_pos ? MF_BYPOSITION : MF_BYCOMMAND);
}

//
//   class dialog
//

resource::dialog::dialog() : resource_base()
{
}

resource::dialog::dialog(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::dialog::~dialog()
{
}

void resource::dialog::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::dialog::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_DIALOG, instance);
}

//
//   class string
//

resource::string::string() : resource_base()
{
}

resource::string::string(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::string::~string()
{
}

void resource::string::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::string::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_STRING, instance);
}

//
//   class fontdir
//

resource::fontdir::fontdir() : resource_base()
{
}

resource::fontdir::fontdir(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::fontdir::~fontdir()
{
}

void resource::fontdir::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::fontdir::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_FONTDIR, instance);
}

//
//   class font
//

resource::font::font() : resource_base()
{
}

resource::font::font(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::font::~font()
{
}

void resource::font::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::font::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_FONT, instance);
}

//
//   class accelerator
//

resource::accelerator::accelerator() : resource_base()
{
}

resource::accelerator::accelerator(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::accelerator::~accelerator()
{
}

void resource::accelerator::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::accelerator::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_ACCELERATOR, instance);
}

//
//   class rcdata
//

resource::rcdata::rcdata() : resource_base()
{
}

resource::rcdata::rcdata(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::rcdata::~rcdata()
{
}

void resource::rcdata::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::rcdata::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_RCDATA, instance);
}

//
//   class messagetable
//

resource::messagetable::messagetable() : resource_base()
{
}

resource::messagetable::messagetable(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::messagetable::~messagetable()
{
}

void resource::messagetable::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::messagetable::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_MESSAGETABLE, instance);
}

//
//   class html
//

resource::html::html() : resource_base()
{
}

resource::html::html(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::html::~html()
{
}

void resource::html::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::html::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_HTML, instance);
}

//
//   class version
//

resource::version::version() : resource_base()
{
}

resource::version::version(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::version::~version()
{
}

void resource::version::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::version::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_VERSION, instance);
}

//
//   class group_cursor
//

resource::group_cursor::group_cursor() : resource_base()
{
}

resource::group_cursor::group_cursor(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::group_cursor::~group_cursor()
{
}

void resource::group_cursor::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::group_cursor::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_GROUP_CURSOR, instance);
}

//
//   class group_icon
//

resource::group_icon::group_icon() : resource_base()
{
}

resource::group_icon::group_icon(unsigned long lang_id) : resource_base(lang_id)
{
}

resource::group_icon::~group_icon()
{
}

void resource::group_icon::load(unsigned int id, HINSTANCE instance) noexcept
{
    load(MAKEINTRESOURCEW(id), instance);
}

void resource::group_icon::load(const  wchar_t* name, HINSTANCE instance) noexcept
{
    load_ex(name, RT_GROUP_ICON, instance);
}