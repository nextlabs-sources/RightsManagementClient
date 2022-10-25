
#ifndef __NX_OVERLAY_H__
#define __NX_OVERLAY_H__


#include <string>
#include <vector>
#include <memory>
#include <thread>


namespace NX {

namespace overlay {

class overlay_object
{
public:
    overlay_object();
    overlay_object(const std::wstring& shost,
        const std::wstring& suser,
        const std::wstring& text,
        const std::wstring& font_name,
        int font_size,
        const std::wstring& text_color,
        int transratio,
        int rotation);
    virtual ~overlay_object();

    inline const std::wstring& get_text() const { return _text; }
    inline COLORREF get_text_color() const { return _color; }
    inline int get_transparency() const { return _transratio; }
    inline int get_rotation() const { return _rotation; }
    inline const std::wstring& get_font_name() const { return _font_name; }
    inline int get_font_size() const { return _font_size; }
    
    overlay_object& operator = (const overlay_object& other);

    void to_bitmap(const std::wstring& file) const;
    void to_png(const std::wstring& file) const;

protected:
    COLORREF name_to_color(const std::wstring& color_name);
    int ratio_to_transparency(int ratio);
    std::wstring remove_escaping_character(const std::wstring& s);
    std::wstring normalize_text(const std::wstring& s, const std::wstring& suser, const std::wstring& shost);

private:
    std::wstring    _text;
    COLORREF        _color;
    int             _transratio;
    int             _rotation;
    std::wstring    _font_name;
    int             _font_size;
};

class overlay_image
{
public:
    overlay_image();
    virtual ~overlay_image();

    void create(const std::wstring& file, const overlay_object& obj);
    void clear();

    const std::wstring& image_path() const { return _image_path; }
    int transparency_ratio() const { return _transratio; }

private:
    std::wstring    _image_path;
    int             _transratio;
};

}
}


#endif