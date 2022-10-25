


#ifndef __NXGUI_DIALOG_HPP__
#define __NXGUI_DIALOG_HPP__

#include <string>
#include <vector>

#include "winbase.hpp"

namespace NXGUI {


class dialog_template_std;
class dialog_template_ext;

class sz_or_ord
{
public:
    sz_or_ord() : _ordinal(0xFFFF)
    {
    }
    sz_or_ord(const void* p)
    {
        unsigned short type = *((const unsigned short*)p);
        if (0xFFFF == type) {
            _ordinal = *(((const unsigned short*)p) + 1);
        }
        else {
            _name = (const wchar_t*)p;
            _ordinal = 0xFFFF;
        }
    }
    ~sz_or_ord()
    {
    }

    sz_or_ord& operator = (const sz_or_ord& other)
    {
        if (this != &other) {
            _ordinal = other.ordinal();
            _name = other.name();
        }
        return *this;
    }

    inline bool is_ordinal() const noexcept { return (0xFFFF != _ordinal); }
    inline bool is_empty() const noexcept { return (0xFFFF == _ordinal && _name.empty()); }
    inline unsigned short ordinal() const noexcept { return _ordinal; }
    inline const std::wstring& name() const noexcept { return _name; }

    inline size_t size() const
    {
        if (0xFFFF == _ordinal) {
            return ((_name.length() + 1) * sizeof(wchar_t));
        }
        else {
            return (sizeof(unsigned short) * 2);
        }
    }

    inline std::vector<unsigned char> data() const noexcept
    {
        std::vector<unsigned char> buf;
        buf.resize(size(), 0);
        if (is_ordinal()) {
            assert(buf.size() == 4);
            unsigned short* p = (unsigned short*)buf.data();
            p[0] = 0xFFFF;
            p[1] = ordinal();
        }
        else {
            assert(buf.size() == ((_name.length()+1) * sizeof(wchar_t)));
            if (!_name.empty()) {
                memcpy(&buf[0], _name.c_str(), _name.length()*sizeof(wchar_t));
            }
        }
        return std::move(buf);
    }

private:
    std::wstring    _name;
    unsigned short  _ordinal;
};

class dialog_item_template
{
public:
    dialog_item_template()
    {
    }
    dialog_item_template(const DLGITEMTEMPLATE* p)
    {
        _style = p->style;
        _extended_style = p->dwExtendedStyle;
        _x = p->x;
        _y = p->y;
        _cx = p->cx;
        _cy = p->cy;
        _id = p->id;
    }
    virtual ~dialog_item_template()
    {
    }

    dialog_item_template& operator = (const dialog_item_template& other)
    {
        if (this != &other) {
            _style = other.style();
            _extended_style = other.extended_style();
            _x = other.x();
            _y = other.y();
            _cx = other.cx();
            _cy = other.cy();
            _id = other.id();
        }
        return *this;
    }

    inline short id() const noexcept { return _id; }
    inline unsigned long style() const noexcept { return _style; }
    inline unsigned long extended_style() const noexcept { return _extended_style; }
    inline short x() const noexcept { return _x; }
    inline short y() const noexcept { return _y; }
    inline short cx() const noexcept { return _cx; }
    inline short cy() const noexcept { return _cy; }

protected:
    unsigned long   _style;
    unsigned long   _extended_style;
    short           _x;
    short           _y;
    short           _cx;
    short           _cy;
    short           _id;
};

class dialog_item_template_ex : public dialog_item_template
{
    typedef struct {
        DWORD     helpID;
        DWORD     exStyle;
        DWORD     style;
        short     x;
        short     y;
        short     cx;
        short     cy;
        DWORD     id;
    } DLGITEMTEMPLATEEX_HEADER;

public:
    dialog_item_template_ex()
    {
    }
    dialog_item_template_ex(const void* p)
    {
        const DLGITEMTEMPLATEEX_HEADER* hdr = (const DLGITEMTEMPLATEEX_HEADER*)p;
        _help_id        = hdr->helpID;
        _extended_style = hdr->exStyle;
        _style          = hdr->style;
        _x              = hdr->x;
        _y              = hdr->y;
        _cx             = hdr->cx;
        _cy             = hdr->cy;
        _id             = (short)hdr->id;
        const unsigned char* wndcls_p = (const unsigned char*)(hdr + 1);
        _wnd_class = sz_or_ord(wndcls_p);
        const unsigned char* title_p = wndcls_p + _wnd_class.size();
        _title = sz_or_ord(title_p);
        const unsigned short* extra_count = (const unsigned short*)(title_p + _title.size());
        if (*extra_count != 0) {
            const unsigned short* extra_p = (extra_count + 1);
            _extra_data = std::vector<unsigned short>(extra_p, extra_p + (*extra_count));
        }
    }
    virtual ~dialog_item_template_ex()
    {
    }

    dialog_item_template_ex& operator = (const dialog_item_template_ex& other)
    {
        if (this != &other) {
            dialog_item_template::operator=(other);
            _help_id = other.help_id();
            _wnd_class = other.wnd_class();
            _title = other.title();
            _extra_data = other.extra_dta();
        }
        return *this;
    }

    inline short help_id() const noexcept { return (short)_help_id; }
    inline sz_or_ord wnd_class() const noexcept { return _wnd_class; }
    inline sz_or_ord title() const noexcept { return _title; }
    inline const std::vector<unsigned short>& extra_dta() const noexcept { return _extra_data; }

    inline size_t size() const
    {
        return (sizeof(DLGITEMTEMPLATEEX_HEADER) + _wnd_class.size() + _title.size() + sizeof(unsigned short) + _extra_data.size());
    }

    inline std::vector<unsigned char> data() const noexcept
    {
        std::vector<unsigned char> buf;
        buf.resize(size(), 0);

        DLGITEMTEMPLATEEX_HEADER* hdr = (DLGITEMTEMPLATEEX_HEADER*)(&buf[0]);
        hdr->helpID = _help_id;
        hdr->exStyle = _extended_style;
        hdr->style = _style;
        hdr->x = _x;
        hdr->y = _y;
        hdr->cx = _cx;
        hdr->cy = _cy;
        hdr->id = _id;

        unsigned char* wndcls_p = (unsigned char*)(hdr + 1);
        const std::vector<unsigned char>& wndcls_r = _wnd_class.data();
        assert(wndcls_r.size() == _wnd_class.size());
        memcpy(wndcls_p, &wndcls_r[0], wndcls_r.size());

        unsigned char* title_p = wndcls_p + wndcls_r.size();
        const std::vector<unsigned char>& title_r = _title.data();
        assert(title_r.size() == _title.size());
        memcpy(title_p, &title_r[0], title_r.size());

        unsigned short* extra_count = (unsigned short*)(title_p + title_r.size());
        *extra_count = (unsigned short)_extra_data.size();
        if (*extra_count != 0) {
            unsigned short* extra_p = (extra_count + 1);
            memcpy(extra_p, &_extra_data[0], _extra_data.size());
        }

        return std::move(buf);
    }

private:
    unsigned long   _help_id;
    sz_or_ord       _wnd_class;
    sz_or_ord       _title;
    std::vector<unsigned short> _extra_data;
};

class dialog_template
{
public:
    virtual ~dialog_template();

    static dialog_template* create(unsigned int template_id, HINSTANCE instance);
    static dialog_template* create(const wchar_t* template_name, HINSTANCE instance);
    static dialog_template* create(HGLOBAL h);

    virtual void clear() noexcept;
    virtual bool empty() const noexcept;

    virtual bool is_standard() const noexcept = 0;
    virtual bool is_extended() const noexcept = 0;
    virtual dialog_template_std* as_standard() = 0;
    virtual dialog_template_ext* as_extended() = 0;
    virtual size_t size() const noexcept = 0;

    virtual unsigned long style() const { return _style; }
    virtual unsigned long extended_style() const { return _extended_style; }
    virtual unsigned short item_count() const { return (unsigned short)_cdit; }
    virtual short x() const noexcept { return _x; }
    virtual short y() const noexcept { return _y; }
    virtual short cx() const noexcept { return _cx; }
    virtual short cy() const noexcept { return _cy; }
    virtual const sz_or_ord& menu() const noexcept { return _menu; }
    virtual const sz_or_ord& wnd_class() const noexcept { return _class; }
    virtual const sz_or_ord& title() const noexcept { return _title; }

protected:
    dialog_template() {}

protected:
    unsigned long   _style;
    unsigned long   _extended_style;
    unsigned long   _cdit;
    short           _x;
    short           _y;
    short           _cx;
    short           _cy;
    sz_or_ord       _menu;
    sz_or_ord       _class;
    sz_or_ord       _title;
    unsigned short  _pointsize;
    std::wstring    _typeface;
};

class dialog_template_std : public dialog_template
{
public:
    dialog_template_std(const void* p);
    virtual ~dialog_template_std();

    virtual bool is_standard() const noexcept { return true; }
    virtual bool is_extended() const noexcept { return false; }
    virtual dialog_template_std* as_standard() { return this; }
    virtual dialog_template_ext* as_extended() { throw std::exception("not an extended dialog template"); }

    virtual size_t size() const noexcept;

    inline const std::vector<dialog_item_template>& items() const noexcept { return _items; }
    inline std::vector<dialog_item_template>& items() noexcept { return _items; }

protected:
    dialog_template_std();
private:
    std::vector<dialog_item_template>   _items;
};

class dialog_template_ext : public dialog_template
{
public:
    dialog_template_ext(const void* p);
    virtual ~dialog_template_ext();

    virtual bool is_standard() const noexcept { return false; }
    virtual bool is_extended() const noexcept { return true; }
    virtual dialog_template_std* as_standard() { throw std::exception("not an extended dialog template"); }
    virtual dialog_template_ext* as_extended() { return this; }

    virtual size_t size() const noexcept;

    inline unsigned long help_id() const noexcept { return _help_id; }
    inline unsigned short dlg_ver() const noexcept { return _dlg_ver; }
    inline unsigned short signature() const noexcept { return _signature; }
    inline const std::vector<dialog_item_template_ex>& items() const noexcept { return _items; }
    inline std::vector<dialog_item_template_ex>& items() noexcept { return _items; }

protected:
    dialog_template_ext();
private:
    std::vector<dialog_item_template_ex>   _items;
    unsigned long _help_id;
    unsigned short _dlg_ver;
    unsigned short _signature;
    unsigned short _font_weight;
    unsigned char  _font_italic;
    unsigned char  _font_charset;
};




class dialog : public window
{
	// Modeless construct
public:
    dialog();
	void initialize();

	virtual bool create(const wchar_t* template_name, window* parent = NULL);
	virtual bool create(unsigned int template_id, window* parent = NULL);
	virtual bool create_indirect(LPCDLGTEMPLATE dlg_template, window* parent = NULL, void* dlg_init = NULL);
	virtual bool create_indirect(HGLOBAL h, window* parent = NULL);

	// Modal construct
public:
	explicit dialog(const wchar_t* template_name, window* parent = NULL);
	explicit dialog(unsigned int template_id, window* parent = NULL);
	bool init_modal_indirect(LPCDLGTEMPLATE dlg_template, window* parent = NULL, void* dlg_init = NULL);
    bool init_modal_indirect(HGLOBAL h, window* parent = NULL);

// Attributes
public:
	void map_dialog_rect(LPRECT lpRect) const;
	void set_help_id(unsigned int id);

// Operations
public:
	// modal processing
	virtual INT_PTR do_modal();

	// support for passing on tab control - use 'PostMessage' if needed
	void next_dlg_ctrl() const;
	void prev_dlg_ctrl() const;
	void goto_dlg_ctrl(window* pWndCtrl);

	// default button access
	void set_def_id(UINT id);
	DWORD get_def_id() const;

	// termination
	void end_dialog(int result);

    // Overridables (special message map entries)
	virtual bool on_init_dialog();
	virtual void on_set_font(HFONT h);

protected:
	virtual void on_ok();
	virtual void on_cancel();

// Implementation
public:
	virtual ~dialog();
	virtual bool pre_translate_message(MSG* msg);
	virtual bool on_command(WPARAM wparam, LPARAM lparam, LRESULT& result);
	virtual bool check_auto_center();

protected:
	unsigned int    _id_help;                 // Help ID (0 for none, see HID_BASE_RESOURCE)

	// parameters for 'DoModal'
	const wchar_t*  _template_name;     // name or MAKEINTRESOURCE
	HGLOBAL         _template_handle;   // indirect (_template == NULL)
	LPCDLGTEMPLATE  _template;          // indirect if (_template == NULL)
	void*           _dlg_init;          // DLGINIT resource data
	window*         _parent;            // parent/owner window
	HWND            _topwnd;            // top level parent window (may be disabled)
	bool            _close_by_enddlg;   // indicates that the dialog was closed by calling EndDialog method

	virtual void pre_init_dialog();

	// implementation helpers
	HWND pre_modal();
	void post_modal();

	bool create_indirect(LPCDLGTEMPLATE dlg_template, window* parent, void* lpDialogInit, HINSTANCE instance);
    bool create_indirect(HGLOBAL h, window* parent, HINSTANCE instacne);

protected:
	LRESULT on_command_help(WPARAM wParam, LPARAM lParam);
	LRESULT on_help_hit_test(WPARAM wParam, LPARAM lParam);
	LRESULT handle_init_dialog(WPARAM, LPARAM);

	virtual bool on_set_font(WPARAM wparam, LPARAM lparam, LRESULT& result);
	virtual bool on_paint(WPARAM wparam, LPARAM lparam, LRESULT& result);
	virtual bool on_query_end_session(WPARAM wparam, LPARAM lparam, LRESULT& result);
	virtual bool on_end_session(WPARAM wparam, LPARAM lparam, LRESULT& result);
};

}

#endif