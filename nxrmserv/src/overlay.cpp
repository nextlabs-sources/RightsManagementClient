


#include <Windows.h>


#include <boost/algorithm/string.hpp>
#include <nudf\string.hpp>
#include <nudf\bitmap.hpp>

#include "overlay.hpp"


using namespace NX;
using namespace NX::overlay;


overlay_object::overlay_object()
    : _text(L""), _transratio(90), _font_name(L"Sitka Text"), _font_size(16), _color(name_to_color(L"Gray")), _rotation(-45)
{
}

overlay_object::overlay_object(const std::wstring& shost,
    const std::wstring& suser,
    const std::wstring& text,
    const std::wstring& font_name,
    int font_size,
    const std::wstring& text_color,
    int transratio,
    int rotation)
    : _text(normalize_text(text, suser, shost)), _font_name(font_name), _font_size(font_size), _color(name_to_color(text_color)), _transratio(ratio_to_transparency(transratio)), _rotation(rotation%180)
{
}

overlay_object::~overlay_object()
{
}

overlay_object& overlay_object::operator = (const overlay_object& other)
{

    if (this != &other) {
        _text = other.get_text();
        _color = other.get_text_color();
        _transratio = other.get_transparency();
        _rotation = other.get_rotation();
        _font_name = other.get_font_name();
        _font_size = other.get_font_size();
    }
    return *this;
}

int overlay_object::ratio_to_transparency(int ratio)
{
    return (100 - ((abs(ratio) <= 100) ? abs(ratio) : 100));
}

COLORREF overlay_object::name_to_color(const std::wstring& color_name)
{
    //
    // See RGB color table:
    //    http://www.rapidtables.com/web/color/RGB_Color.htm
    //
    if (0 == _wcsicmp(color_name.c_str(), L"Red")) {
        return RGB(255, 0, 0);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Lime")) {
        return RGB(0, 255, 0);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Blue")) {
        return RGB(0, 0, 255);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Yellow")) {
        return RGB(255, 255, 0);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Cyan / Aqua")) {
        return RGB(0, 255, 255);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Magenta / Fuchsia")) {
        return RGB(255, 0, 255);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Gray")) {
        return RGB(128, 128, 128);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Dim Gray")) {
        return RGB(105, 105, 105);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Maroon")) {
        return RGB(128, 0, 0);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Olive")) {
        return RGB(128, 128, 0);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Green")) {
        return RGB(0, 128, 0);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Purple")) {
        return RGB(128, 0, 128);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Teal")) {
        return RGB(0, 128, 128);
    }
    else if (0 == _wcsicmp(color_name.c_str(), L"Navy")) {
        return RGB(0, 0, 128);
    }
    else {
        // Black
        return RGB(0, 0, 0);
    }
}

std::wstring overlay_object::remove_escaping_character(const std::wstring& s)
{
    std::wstring ns;

    if (s.length() != 0) {

        bool changed = false;
        const wchar_t* const end_pos = s.c_str() + s.length();
        const wchar_t * p = s.c_str();
        while (p != end_pos) {

            wchar_t c = *(p++);
            if (c == L'\\') {
                switch (*p)
                {
                case L'n':
                    c = L'\n';
                    ++p;
                    changed = true;
                    break;
                case L'r':
                    c = L'\r';
                    ++p;
                    changed = true;
                    break;
                case L't':
                    c = L'\t';
                    ++p;
                    changed = true;
                    break;
                case L'\\':
                    c = L'\\';
                    ++p;
                    changed = true;
                    break;
                default:
                    break;
                }
            }
            ns.push_back(c);
        }
    }

    return std::move(ns);
}

std::wstring overlay_object::normalize_text(const std::wstring& s, const std::wstring& suser, const std::wstring& shost)
{
    SYSTEMTIME st = { 0 };
    std::wstring sdate;
    std::wstring stime;
    std::wstring ns = remove_escaping_character(s);

    GetLocalTime(&st);
    swprintf_s(nudf::string::tempstr<wchar_t>(sdate, 64), 64, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    swprintf_s(nudf::string::tempstr<wchar_t>(stime, 64), 64, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    boost::algorithm::replace_all(ns, L"$(User)", suser);
    boost::algorithm::replace_all(ns, L"$(Host)", shost);
    boost::algorithm::replace_all(ns, L"$(Date)", sdate);
    boost::algorithm::replace_all(ns, L"$(Time)", stime);

    return std::move(ns);
}

void overlay_object::to_bitmap(const std::wstring& file) const
{
    try {
        nudf::image::CTextBitmap bitmap;
        if (!bitmap.Create(get_text().c_str(), get_font_name().c_str(), get_font_size(), RGB(255, 255, 255), get_text_color())) {
            throw std::exception("fail to create bitmap");
        }
        if (get_rotation() != 0) {
            bitmap.Rotate(NULL, get_rotation());
        }
        if (!bitmap.ToFile(file.c_str())) {
            throw std::exception("fail to save to bitmap");
        }
    }
    catch (std::exception& e) {
        ::DeleteFileW(file.c_str());
        throw e;
    }
}

void overlay_object::to_png(const std::wstring& file) const
{
    try {
        nudf::image::CTextBitmap bitmap;
        if (!bitmap.Create(get_text().c_str(), get_font_name().c_str(), get_font_size(), RGB(255, 255, 255), get_text_color())) {
            throw std::exception("fail to create bitmap");
        }
        if (get_rotation() != 0) {
            bitmap.Rotate(NULL, get_rotation());
        }
        if (!bitmap.ToPNGFile(file.c_str())) {
            throw std::exception("fail to save to bitmap");
        }
    }
    catch (std::exception& e) {
        ::DeleteFileW(file.c_str());
        throw e;
    }
}


overlay_image::overlay_image() : _transratio(0)
{
}

overlay_image::~overlay_image()
{
    clear();
}

void overlay_image::create(const std::wstring& file, const overlay_object& obj)
{
    try {
        _image_path = file;
        ::DeleteFileW(_image_path.c_str());
        obj.to_png(_image_path.c_str());
        _transratio = obj.get_transparency();
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}

void overlay_image::clear()
{
    if (!_image_path.empty()) {
        ::DeleteFileW(_image_path.c_str());
        _image_path.clear();
        _transratio = 0;
    }
}
