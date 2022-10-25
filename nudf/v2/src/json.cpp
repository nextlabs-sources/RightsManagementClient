

#include <Windows.h>
#include <assert.h>

#include <algorithm>

#include <nudf\json.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>


using namespace NX;

namespace NX {
namespace json_impl {

    

template <typename CharType>
class json_parser
{
public:
    json_parser() : _current_parsing_depth(0), _current_line(0), _current_column(0)
    {
    }

    ~json_parser()
    {
    }

    std::shared_ptr<json_value> parse()
    {
        peek_next_nwspace_char();   // find first non-white-space character
        return parse_object();
    }

protected:
    virtual bool is_eof(CharType ch) = 0;
    virtual CharType read_next_char() = 0;
    virtual CharType peek_next_char() = 0;

    inline void increase_line() { ++_current_line; _current_column = 0; }
    inline void increase_column() { ++_current_column; }
    inline size_t current_line() const { return _current_line; }
    inline size_t current_column() const { return _current_column; }

protected:
    CharType read_next_nwspace_char()
    {
        CharType ch = read_next_char();
        while (!is_eof(ch) && iswspace((int)ch)) {
            ch = read_next_char();
        }
        return ch;
    }
    CharType peek_next_nwspace_char()
    {
        CharType ch = peek_next_char();
        while (!is_eof(ch) && iswspace((int)ch)) {
            read_next_char(); // move to next
            ch = peek_next_char();
        }
        return ch;
    }

    inline std::wstring to_utf16(const std::wstring& s) { return s; }
    inline std::wstring to_utf16(const std::string& s) { return NX::conversion::utf8_to_utf16(s); }

protected:
    std::shared_ptr<json_value> parse_object()
    {
        std::shared_ptr<json_value> sp = json_value::create_object();

        CharType ch = read_next_char(); // caller need to assure that next character is '{'
        assert(ch == CharType('{'));
        if (ch != CharType('{')) {
            throw std::exception("missing '{' at object beginning");
        }

        while (true) {

            std::shared_ptr<json_value> object_value;

            ch = peek_next_nwspace_char();

            // object finished
            if (ch == CharType('}')) {
                read_next_char();   // ignore '}'
                break;
            }

            if (ch == CharType(',')) {
                read_next_char();   // ignore ','
                continue;
            }

            assert(ch == CharType('\"'));
            if (ch != CharType('\"')) {
                throw std::exception("missing '\"' at object name");
            }

            const std::wstring& object_name = this->to_utf16(inter_parse_string());

            ch = read_next_nwspace_char();
            assert(ch == CharType(':'));
            if (ch != CharType(':')) {
                throw std::exception("missing ':' in object item");
            }

            // get value
            ch = peek_next_nwspace_char();
            switch (ch)
            {
            case CharType(','): // value is empty (Null)
            case CharType('N'):
            case CharType('n'):
                sp->as_object()[object_name] = parse_null();
                break;
            case CharType('{'): // value is an object
                sp->as_object()[object_name] = parse_object();
                break;
            case CharType('['): // value is an array
                sp->as_object()[object_name] = parse_array();
                break;
            case CharType('T'): // value is a boolean
            case CharType('t'):
            case CharType('F'):
            case CharType('f'):
                sp->as_object()[object_name] = parse_boolean();
                break;
            case CharType('\"'):// value is a string
                sp->as_object()[object_name] = parse_string();
                break;
            case CharType('-'): // value is a number
            case CharType('0'):
            case CharType('1'):
            case CharType('2'):
            case CharType('3'):
            case CharType('4'):
            case CharType('5'):
            case CharType('6'):
            case CharType('7'):
            case CharType('8'):
            case CharType('9'):
                sp->as_object()[object_name] = parse_number();
                break;
            default:            // value begins with unexpected character
                throw std::exception("unexpected character in object value");
                break;
            }

        }

        return sp;
    }

    std::shared_ptr<json_value> parse_array()
    {
        std::shared_ptr<json_value> sp = json_value::create_array();

        CharType ch = read_next_char(); // caller need to assure that next character is '['
        assert(ch == CharType('['));
        if (ch != CharType('[')) {
            throw std::exception("missing '[' at array beginning");
        }

        while (true) {
            
            ch = peek_next_nwspace_char();

            // array finished
            if (ch == CharType(']')) {
                read_next_char();   // ignore ']'
                break;
            }

            if (ch == CharType(',')) {
                read_next_char();   // ignore ','
                continue;
            }

            // array item
            ch = peek_next_nwspace_char();
            switch (ch)
            {
            case CharType(','): // item is empty (Null)
            case CharType('N'):
            case CharType('n'):
                sp->as_array().push_back(parse_null());
                break;
            case CharType('{'): // item is an object
                sp->as_array().push_back(parse_object());
                break;
            case CharType('['): // item is an array
                sp->as_array().push_back(parse_array());
                break;
            case CharType('T'): // item is a boolean
            case CharType('t'):
            case CharType('F'):
            case CharType('f'):
                sp->as_array().push_back(parse_boolean());
                break;
            case CharType('\"'):// item is a string
                sp->as_array().push_back(parse_string());
                break;
            case CharType('-'): // item is a number
            case CharType('0'):
            case CharType('1'):
            case CharType('2'):
            case CharType('3'):
            case CharType('4'):
            case CharType('5'):
            case CharType('6'):
            case CharType('7'):
            case CharType('8'):
            case CharType('9'):
                sp->as_array().push_back(parse_number());
                break;
            default:            // item begins with unexpected character
                throw std::exception("unexpected character in object value");
                break;
            }

        }

        return sp;
    }

    std::shared_ptr<json_value> parse_null()
    {
        CharType ch = read_next_char(); // caller need to assure that next character is ('N' or 'n')

        if (ch == CharType(',')) {
            return json_value::create_null();
        }
        else if (ch == CharType('N') || ch == CharType('n')) {
            ch = read_next_char();
            if (CharType('U') != ch && (CharType('u') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('L') != ch && (CharType('l') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('L') != ch && (CharType('l') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            return json_value::create_null();
        }
        else {
            throw std::exception("malformed null literal");
        }
    }

    std::shared_ptr<json_value> parse_boolean()
    {
        CharType ch = read_next_char(); // caller need to assure that next character is (")

        if (ch == CharType('T') || ch == CharType('t')) {
            ch = read_next_char();
            if (CharType('R') != ch && (CharType('r') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('U') != ch && (CharType('u') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('E') != ch && (CharType('e') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            return json_value::create_boolean(true);
        }
        else if (ch == CharType('F') || ch == CharType('f')) {
            ch = read_next_char();
            if (CharType('A') != ch && (CharType('a') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('L') != ch && (CharType('l') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('S') != ch && (CharType('s') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            ch = read_next_char();
            if (CharType('E') != ch && (CharType('e') != ch)) {
                throw std::exception("malformed boolean literal");
            }
            return json_value::create_boolean(false);
        }
        else {
            throw std::exception("malformed boolean literal");
        }
    }

    std::shared_ptr<json_value> parse_string()
    {
        std::basic_string<CharType> s = inter_parse_string();
        return json_value::create_string(s);
    }

    std::basic_string<CharType> inter_parse_string()
    {
        std::basic_string<CharType> s;
        CharType ch = read_next_char(); // caller need to assure that next character is (")

        assert(ch == CharType('\"'));

        while (CharType('\"') != (ch = read_next_char())) {

            if (CharType('\\') == ch) {

                switch (ch)
                {
                case CharType('\"'):
                    s.push_back(CharType('\"'));
                    break;
                case CharType('\\'):
                    s.push_back(CharType('\\'));
                    break;
                case CharType('/'):
                    s.push_back(CharType('/'));
                    break;
                case CharType('b'):
                    s.push_back(CharType('\b'));
                    break;
                case CharType('f'):
                    s.push_back(CharType('\f'));
                    break;
                case CharType('r'):
                    s.push_back(CharType('\r'));
                    break;
                case CharType('n'):
                    s.push_back(CharType('\n'));
                    break;
                case CharType('t'):
                    s.push_back(CharType('\t'));
                    break;
                case CharType('u'):
                    // A four-hexdigit Unicode character.
                    // Transform into a 16 bit code point.
                    {
                        CharType hex_str[4] = {0, 0, 0, 0};
                        for (int i = 0; i < 4; i++) {
                            hex_str[i] = read_next_char();
                            if (!NX::utility::is_hex<CharType>(hex_str[i])) {
                                throw std::exception("bad hex value");
                            }
                        }
                        s += handle_hex_unicode(hex_str);
                    }
                    break;
                default:
                    throw std::exception("unsupported escaped character");
                    break;
                }
            }
            else {
                s.push_back(ch);
            }
        }

        return std::move(s);
    }

    std::shared_ptr<json_value> parse_hex_number()
    {
        std::basic_string<CharType> s;

        while (true) {

            CharType ch = peek_next_char();

            if (CharType(' ') == ch || CharType(',') == ch || CharType('}') == ch || CharType(']') == ch) {
                break;
            }

            // not white space nor token character, do real read
            read_next_char();

            if (!NX::utility::is_hex<CharType>(ch)) {
                throw std::exception("unexpected character in hex value");
            }

            s.push_back(ch);
        }

        return json_value::create_number(s.empty() ? 0ULL : std::stoull(s, 0, 16));
    }

    std::shared_ptr<json_value> parse_decimal(CharType cb)
    {
        std::basic_string<CharType> s;
        bool    is_float = false;
        bool    is_exponent = false;
        bool    is_signed = false;

        if (cb == CharType('-')) {
            is_signed = true;
        }
        if (cb == CharType('.')) {
            s.push_back(CharType('0'));
            is_float = true;
        }
        s.push_back(cb);
        while (true) {

            CharType ch = peek_next_char();

            if (CharType(' ') == ch || CharType(',') == ch || CharType('}') == ch || CharType(']') == ch) {
                break;
            }

            // not white space nor token character, do real read
            read_next_char();

            switch (ch)
            {
            case CharType('.'):
                if (is_float) {
                    throw std::exception("number syntax error: duplicate '.'");
                }
                is_float = true;
                s.push_back(ch);
                break;
            case CharType('E'):
            case CharType('e'):
                if (is_exponent) {
                    throw std::exception("number syntax error: duplicate 'E'");
                }
                is_exponent = true;
                s.push_back(ch);
                break;
            case CharType('0'):
            case CharType('1'):
            case CharType('2'):
            case CharType('3'):
            case CharType('4'):
            case CharType('5'):
            case CharType('6'):
            case CharType('7'):
            case CharType('8'):
            case CharType('9'):
                s.push_back(ch);
                break;
            default:
                throw std::exception("number syntax error: unexpected character");
                break;
            }
        }

        return (is_float || is_exponent) ? json_value::create_number(std::stod(s)) : json_value::create_number(std::stoull(s));
    }

    std::shared_ptr<json_value> parse_number()
    {
        CharType ch = 0;

        // check first character
        ch = read_next_nwspace_char();
        if (ch == CharType('0')) {
            CharType ch2 = peek_next_char();
            if (CharType('X') == ch2 || CharType('x') == ch2) {
                // hex
                return parse_hex_number();
            }
        }

        // not hex
        return parse_decimal(ch);
    }

protected:
    std::wstring handle_hex_unicode(const wchar_t* hex_str)
    {
        std::wstring s;
        wchar_t c = 0;
        for (int i = 0; i < 4; i++) {
            c <<= 4;
            c |= (wchar_t)NX::utility::hex_to_int<wchar_t>(hex_str[i]);
        }
        s.push_back(c);
        return std::move(s);
    }

    std::string handle_hex_unicode(const char* hex_str)
    {
        std::wstring s;
        wchar_t c = 0;
        for (int i = 0; i < 4; i++) {
            c <<= 4;
            c |= (wchar_t)NX::utility::hex_to_int<char>(hex_str[i]);
        }
        s.push_back(c);
        return std::move(NX::conversion::utf16_to_utf8(s));
    }

protected:
    size_t  _current_parsing_depth;
    size_t  _current_line;
    size_t  _current_column;
};


template <typename CharType>
class json_string_parser : public json_parser<CharType>
{
public:
    json_string_parser() : json_parser<CharType>()
    {
    }

    json_string_parser(const std::basic_string<CharType>& s) : json_parser<CharType>(), _start_pos(s.c_str()), _end_pos(s.c_str() + s.length()), _p(s.c_str())
    {
    }

    ~json_string_parser()
    {
    }

protected:
    virtual bool is_eof(CharType ch)
    {
        return (0 == ch);
    }

    virtual CharType read_next_char()
    {
        CharType ch = 0;

        if (_p == _end_pos) {
            return 0;
        }

        ch = *(_p++);
        if (ch == CharType('\n')) {
            ++_current_line;
            _current_column = 0;
        }
        else {
            ++_current_column;
        }

        return ch;
    }

    virtual CharType peek_next_char()
    {
        if (_p == _end_pos) {
            return 0;
        }
        return *_p;
    }

private:
    const CharType* _start_pos;
    const CharType* _end_pos;
    const CharType* _p;
};


}
}


using namespace NX::json_impl;

//
//  class json_value
//

std::shared_ptr<json_value> json_value::parse(const std::wstring& s)
{
    json_string_parser<wchar_t> parser(s);
    return parser.parse();
}

std::shared_ptr<json_value> json_value::parse(const std::string& s)
{
    json_string_parser<char> parser(s);
    return parser.parse();
}

std::shared_ptr<json_value> json_value::create_null()
{
    return std::shared_ptr<json_value>(new json_null());
}

std::shared_ptr<json_value> json_value::create_boolean(bool v)
{
    return std::shared_ptr<json_value>(new json_boolean(v));
}

std::shared_ptr<json_value> json_value::create_number(int v)
{
    return std::shared_ptr<json_value>(new json_number(v));
}

std::shared_ptr<json_value> json_value::create_number(unsigned int v)
{
    return std::shared_ptr<json_value>(new json_number(v));
}

std::shared_ptr<json_value> json_value::create_number(__int64 v)
{
    return std::shared_ptr<json_value>(new json_number(v));
}

std::shared_ptr<json_value> json_value::create_number(unsigned __int64 v)
{
    return std::shared_ptr<json_value>(new json_number(v));
}

std::shared_ptr<json_value> json_value::create_number(float v)
{
    return std::shared_ptr<json_value>(new json_number(v));
}

std::shared_ptr<json_value> json_value::create_number(double v)
{
    return std::shared_ptr<json_value>(new json_number(v));
}

std::shared_ptr<json_value> json_value::create_string(const std::wstring& s)
{
    return std::shared_ptr<json_value>(new json_string(s));
}

std::shared_ptr<json_value> json_value::create_string(const std::string& s)
{
    return std::shared_ptr<json_value>(new json_string(NX::conversion::utf8_to_utf16(s)));
}

std::shared_ptr<json_value> json_value::create_array()
{
    return std::shared_ptr<json_value>(new json_array());
}

std::shared_ptr<json_value> json_value::create_object()
{
    return std::shared_ptr<json_value>(new json_object());
}


//
//  class json_null
//

json_null::json_null() : json_value(ValueNull)
{
}

json_null::~json_null()
{
}

std::wstring json_null::serialize() const
{
    return std::wstring(L"null");
}


//
//  class json_boolean
//

json_boolean::json_boolean() : json_value(ValueBoolean), _boolean(false)
{
}

json_boolean::json_boolean(bool v) : json_value(ValueBoolean), _boolean(v)
{
}

json_boolean::~json_boolean()
{
}

std::wstring json_boolean::serialize() const
{
    return _boolean ? L"true" : L"false";
}


//
//  class json_number
//

json_number::json_number() : json_value(ValueNumber), _float(false)
{
    _ull = 0;
}

json_number::json_number(int v) : json_value(ValueNumber), _float(false)
{
    _n = v;
}

json_number::json_number(long v) : json_value(ValueNumber), _float(false)
{
    _l = v;
}

json_number::json_number(unsigned int v) : json_value(ValueNumber), _float(false)
{
    _u = v;
}

json_number::json_number(unsigned long v) : json_value(ValueNumber), _float(false)
{
    _u = v;
}

json_number::json_number(__int64 v) : json_value(ValueNumber), _float(false)
{
    _ll = v;
}

json_number::json_number(unsigned __int64 v) : json_value(ValueNumber), _float(false)
{
    _ull = v;
}

json_number::json_number(float v) : json_value(ValueNumber), _float(true)
{
    _f = v;
}

json_number::json_number(double v) : json_value(ValueNumber), _float(true)
{
    _f = v;
}

json_number::~json_number()
{
}

int json_number::as_int() const
{
    if (is_float()) {
        return (int)((__int64)_f);
    }
    else {
        return _n;
    }
}

long json_number::as_long() const
{
    if (is_float()) {
        return (long)((__int64)_f);
    }
    else {
        return _l;
    }
}

unsigned int json_number::as_uint() const
{
    if (is_float()) {
        return (unsigned int)((__int64)_f);
    }
    else {
        return _u;
    }
}

__int64 json_number::as_int64() const
{
    if (is_float()) {
        return (__int64)_f;
    }
    else {
        return _ll;
    }
}

unsigned __int64 json_number::as_uint64() const
{
    if (is_float()) {
        return (unsigned __int64)((__int64)_f);
    }
    else {
        return _ull;
    }
}

double json_number::as_float() const
{
    if (is_float()) {
        return _f;
    }
    else {
        return (1.0 * _ll);
    }
}

std::wstring json_number::serialize() const
{
    if (is_float()) {
        return NX::conversion::to_wstring(_f);
    }
    else {
        return NX::conversion::to_wstring(_ll);
    }
}


//
//  class json_string
//

json_string::json_string() : json_value(ValueString)
{
}

json_string::json_string(const std::wstring& s) : json_value(ValueString), _s(s)
{
}

json_string::~json_string()
{
}

std::wstring json_string::serialize() const
{
    // quota
    std::wstring s(L"\"");
    s += _s;
    s += L"\"";
    return std::move(s);
}



//
//  class json_array
//

json_array::json_array() : json_value(ValueArray)
{
}

json_array::~json_array()
{
}

void json_array::remove(size_t index)
{
    if (index >= size()) {
        throw std::out_of_range("out of array range");
    }
    _array.erase(begin() + index);
}

std::shared_ptr<json_value>& json_array::operator [](size_t index)
{
    if (index >= size()) {
        throw std::out_of_range("out of array range");
    }
    return _array[index];
}

const std::shared_ptr<json_value>& json_array::operator [](size_t index) const
{
    if (index >= size()) {
        throw std::out_of_range("out of array range");
    }
    return _array[index];
}

std::wstring json_array::serialize() const
{
    std::wstring s(L"[");
    std::for_each(cbegin(), cend(), [&](const std::shared_ptr<json_value>& sp) {
        if (s.length() > 1) {
            s += L",";
        }
        s += sp->serialize();
    });
    s += L"]";
    return std::move(s);
}



//
//  class json_object
//


json_object::json_object() : json_value(ValueObject)
{
}

json_object::~json_object()
{
}

std::shared_ptr<json_value>& json_object::operator [](const std::wstring& key)
{
    auto pos = std::find_if(_object.begin(), _object.end(), [&](const std::pair<std::wstring, std::shared_ptr<json_value>>& item)->bool {
        return (0 == _wcsicmp(key.c_str(), item.first.c_str()));
    });

    if (pos == _object.end()) {
        _object.push_back(std::pair<std::wstring, std::shared_ptr<json_value>>(key, std::shared_ptr<json_value>(nullptr)));
        return _object[_object.size() - 1].second;
    }
    else {
        return (*pos).second;
    }
}

std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::iterator json_object::find(const std::wstring& key)
{
    return std::find_if(_object.begin(), _object.end(), [&](const std::pair<std::wstring, std::shared_ptr<json_value>>& item)->bool {
        return (0 == _wcsicmp(key.c_str(), item.first.c_str()));
    });
}

std::vector<std::pair<std::wstring, std::shared_ptr<json_value>>>::const_iterator json_object::find(const std::wstring& key) const
{
    return std::find_if(_object.cbegin(), _object.cend(), [&](const std::pair<std::wstring, std::shared_ptr<json_value>>& item)->bool {
        return (0 == _wcsicmp(key.c_str(), item.first.c_str()));
    });
}

void json_object::remove(const std::wstring& key)
{
    auto pos = std::find_if(_object.begin(), _object.end(), [&](const std::pair<std::wstring, std::shared_ptr<json_value>>& item)->bool {
        return (0 == _wcsicmp(key.c_str(), item.first.c_str()));
    });
    if (pos == _object.end()) {
        _object.erase(pos);
    }
}

std::wstring json_object::serialize() const
{
    std::wstring s(L"{");
    std::for_each(cbegin(), cend(), [&](const std::pair<std::wstring, std::shared_ptr<json_value>>& item) {
        if (s.length() > 1) {
            s += L",";
        }
        s += L"\"";
        s += item.first;
        s += L"\": ";
        s += item.second->serialize();
    });
    s += L"}";
    return std::move(s);
}