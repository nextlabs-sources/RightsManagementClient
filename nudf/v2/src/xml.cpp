

#include <string>
#include <fstream>
#include <boost\algorithm\string.hpp>

#include <map>

#include <nudf\xml.hpp>
#include <nudf\string.hpp>
#include <nudf\conversion.hpp>


using namespace NX;

//
//  class xml_parser
//

namespace xml_detail
{
std::wstring to_utf16(const std::wstring& s)
{
    return s;
}
std::wstring to_utf16(const std::string& s)
{
    return NX::conversion::utf8_to_utf16(s);
}
}

class xml_tag
{
public:
    typedef enum tag_type {
        tag_null = 0,
        tag_comment,
        tag_processing_instruction,
        tag_single_element,
        tag_open_element,
        tag_close_element
    } tag_type;

    xml_tag() : _type(tag_null)
    {
    }
    xml_tag(tag_type t, const std::wstring& n) : _type(t), _name(n)
    {
    }
    xml_tag(tag_type t, const std::wstring& n, const std::map<std::wstring, xml_attribute>& a) : _type(t), _name(n), _attributes(a)
    {
    }
    ~xml_tag()
    {
    }

    const std::wstring& name() const { return _name; }
    const std::map<std::wstring, xml_attribute>& attributes() const { return _attributes; }
    tag_type type() const { return _type; }

    inline bool is_comment() const { return (_type == tag_comment); }
    inline bool is_processing_instruction() const { return (_type == tag_processing_instruction); }
    inline bool is_single_element() const { return (_type == tag_single_element); }
    inline bool is_open_element() const { return (_type == tag_open_element); }
    inline bool is_close_element() const { return (_type == tag_close_element); }

    xml_tag& operator = (const xml_tag& other)
    {
        if (this != &other) {
            _name = other.name();
            _type = other.type();
            _attributes = other.attributes();
        }
        return *this;
    }

    bool empty() const { return (_type == tag_null); }
    void clear()
    {
        _type = tag_null;
        _name.clear();
        _attributes.clear();
    }

private:
    std::wstring _name;
    std::map<std::wstring, xml_attribute> _attributes;
    tag_type _type;
};

template <typename CharType>
class xml_parser
{
public:
    xml_parser() : _current_line(0), _current_column(0)
    {
    }
    virtual ~xml_parser() {}
    
    std::vector<std::shared_ptr<xml_node>> parse()
    {
        std::vector<std::shared_ptr<xml_node>> node_list;

        do {

            std::shared_ptr<xml_node> node = get_next_node();
            if (node == nullptr) {
                break;
            }

            if (node->is_processing_instruction_node() && 0 == _wcsicmp(node->get_name().c_str(), L"xml")) {
                // this is XML declaration
                continue;
            }

            node_list.push_back(node);

        } while (true);

        return std::move(node_list);
    }

protected:
    virtual CharType read_next_char() = 0;
    virtual CharType peek_next_char() = 0;
    CharType read_next_nwspace_char()
    {
        CharType ch = read_next_char();
        while (ch != _eof && iswspace((int)ch)) {
            ch = read_next_char();
        }
        return ch;
    }
    CharType peek_next_nwspace_char()
    {
        CharType ch = peek_next_char();
        while (ch != _eof && iswspace((int)ch)) {
            read_next_char(); // move to next
            ch = peek_next_char();
        }
        return ch;
    }

protected:
    void report_error(const std::string& s)
    {
        std::string info = NX::string_formater("xml parsing error at (%d, %d): %s", _current_line, _current_column, s.c_str());
        throw std::exception(info.c_str());
    }

    virtual void reset_parser()
    {
        _current_line = 0;
        _current_column = 0;
    }

protected:
    std::shared_ptr<xml_node> get_next_node()
    {
        xml_tag tag = get_next_tag();
        return tag.empty() ? std::shared_ptr<xml_node>(nullptr) : parse_xml_tag(tag);
    }
    std::shared_ptr<xml_node> parse_xml_tag(const xml_tag& tag)
    {
        if (tag.is_comment()) {
            return xml_node::create_comment_node(tag.name());
        }
        else if (tag.is_processing_instruction()) {
            return xml_node::create_processing_instruction_node(tag.name(), tag.attributes());
        }
        else if (tag.is_single_element()) {
            return xml_node::create_attribute_node(tag.name(), tag.attributes());
        }
        else if (tag.is_open_element()) {

            std::wstring text = get_node_text();
            xml_tag next_tag = get_next_tag();
            if (text.empty()) {
                // no text
                if (next_tag.is_close_element()) {
                    // next tag is a close tag
                    if (0 != _wcsicmp(tag.name().c_str(), next_tag.name().c_str())) {
                        report_error("mismatch close tag");
                    }
                    // <TAG attr="x"></TAG>
                    return xml_node::create_attribute_node(tag.name(), tag.attributes());
                }
                else {
                    // sub-node
                    std::shared_ptr<xml_node> node = xml_node::create_element_node(tag.name(), tag.attributes());

                    do {
                        if (next_tag.empty()) {
                            report_error("missing close tag");
                        }
                        if (next_tag.is_close_element()) {
                            if (0 != _wcsicmp(tag.name().c_str(), next_tag.name().c_str())) {
                                report_error("mismatch close tag");
                            }
                            break;
                        }
                        else {
                            std::shared_ptr<xml_node> sub_node = parse_xml_tag(next_tag);
                            node->append_child(sub_node);
                            next_tag = get_next_tag();
                        }
                    } while (true);

                    return node;
                }
            }
            else {
                if (!next_tag.is_close_element()) {
                    report_error("missing close tag");
                }
                if (0 != _wcsicmp(tag.name().c_str(), next_tag.name().c_str())) {
                    report_error("mismatch close tag");
                }
                return xml_node::create_text_node(tag.name(), tag.attributes(), text);
            }
        }
        else {
            report_error("missing close tag");
        }

        return std::shared_ptr<xml_node>(nullptr);
    }

    xml_tag get_next_tag()
    {
        CharType ch = read_next_nwspace_char();
        if (ch == _eof) {
            // end
            return xml_tag();
        }
        if (ch != CharType('<')) {
            report_error("missing begin of tag: unexpected character");
        }

        ch = peek_next_char();
        if (ch == _eof) {
            report_error("unexpected xml ending");
            return xml_tag();
        }
        else if (ch == CharType(' ') || ch == CharType('>')) {
            report_error("unexpected character after '<'");
            return xml_tag();
        }
        else if (ch == CharType('!')) {
            // Comment
            read_next_char();  // ignore '!'
            ch = read_next_char();
            if (ch != CharType('-')) {
                report_error("unexpected character in comment tag");
            }
            ch = read_next_char();
            if (ch != CharType('-')) {
                report_error("unexpected character in comment tag");
            }
            return complete_comment_literal();
        }
        else if (ch == CharType('?')) {
            // Processing Instructions
            read_next_char();  // ignore '?'
            return complete_processing_instruction_literal();
        }
        else if (ch == CharType('/')) {
            // Close Tag
            read_next_char();  // ignore '/'
            return complete_close_tag_literal();
        }
        else {
            // normal node
            return complete_open_tag_literal();
        }
        return xml_tag();
    }

    xml_attribute get_next_attribute()
    {
        CharType ch = peek_next_nwspace_char();
        if (ch == _eof) {
            report_error("unexpected xml ending");
        }
        if (ch == CharType('<')) {
            report_error("unexpected character ('<')");
        }
        if (ch == CharType('/') || ch == CharType('?') || ch == CharType('>')) {
            return xml_attribute();
        }

        std::wstring attr_name = get_attribute_name();

        ch = read_next_nwspace_char();
        if (ch != CharType('=')) {
            report_error("unexpected character: '=' not found");
        }
        ch = peek_next_nwspace_char();
        if (ch != CharType('\"')) {
            report_error("unexpected character: '\"' not found");
        }

        std::wstring attr_value = complete_string_literal();

        return xml_attribute(attr_name, attr_value);
    }

protected:
    std::wstring get_node_name()
    {
        std::basic_string<CharType> s;
        do {
            CharType ch = peek_next_char();
            if (ch == _eof) {
                report_error("unexpected xml ending");
            }
            else if (ch == CharType(' ') || ch == CharType('/') || ch == CharType('>')) {
                // done
                break;
            }
            else {
                s.push_back(ch);
                read_next_char();   // move to next
            }
        } while (true);

        std::wstring ws = xml_detail::to_utf16(s);
        boost::algorithm::trim(ws);
        return std::move(ws);
    }
    std::wstring get_attribute_name()
    {
        std::basic_string<CharType> n;
        do {

            CharType ch = peek_next_char();

            if (ch == _eof || ch == CharType('<') || ch == CharType('>') || ch == CharType('/')) {
                // bad xml
                throw std::exception();
            }
            else if (ch == CharType(' ') || ch == CharType('=')) {
                // end of attribute name
                break;
            }
            else {
                // a valid character
                n.push_back(ch);
                read_next_char(); // move to next
            }

        } while (true);
        std::wstring ws = xml_detail::to_utf16(n);
        boost::algorithm::trim(ws);
        return std::move(ws);
    }
    std::wstring get_node_text()
    {
        std::basic_string<CharType> text;
        do {

            CharType ch = peek_next_char();

            if (ch == _eof) {
                // bad xml
                throw std::exception();
            }
            else if (ch == CharType('<')) {
                // end of text
                break;
            }
            else {
                // a valid character
                text.push_back(ch);
                read_next_char(); // move to next
            }

        } while (true);
        std::wstring ws = xml_detail::to_utf16(text);
        boost::algorithm::trim(ws);
        return std::move(ws);
    }
    xml_tag complete_processing_instruction_literal()
    {
        std::wstring tag_name = get_node_name();
        std::map<std::wstring, xml_attribute> tag_attrs;

        // get all the attributes
        while (true) {
            xml_attribute attr = get_next_attribute();
            if (attr.empty()) {
                break;
            }
            // add attributes
            std::wstring k(attr.name());
            std::transform(k.begin(), k.end(), k.begin(), tolower);
            tag_attrs[k] = attr;
        }

        CharType ch = read_next_nwspace_char();
        if (ch != CharType('?')) {
            report_error("unexpected character: '?' not found");
        }
        ch = read_next_char();
        if (ch != CharType('>')) {
            report_error("unexpected character: '>' not found");
        }

        return xml_tag(xml_tag::tag_processing_instruction, tag_name, tag_attrs);
    }
    xml_tag complete_open_tag_literal()
    {
        // normal open tag
        std::map<std::wstring, xml_attribute> tag_attrs;
        std::wstring tag_name = get_node_name();
        if (tag_name.empty()) {
            report_error("empty tag name");
        }

        // get all the attributes
        while (true) {
            xml_attribute attr = get_next_attribute();
            if (attr.empty()) {
                break;
            }
            // add attributes
            std::wstring k(attr.name());
            std::transform(k.begin(), k.end(), k.begin(), tolower);
            tag_attrs[k] = attr;
        }

        // attributes finished
        CharType ch = read_next_nwspace_char();
        if (ch == CharType('/')) {
            // attributes node - no text, no child
            ch = read_next_char();
            if (ch != CharType('>')) {
                report_error("unexpected character: '>' not found");
            }
            return xml_tag(xml_tag::tag_single_element, tag_name, tag_attrs);
        }
        else {
            if (ch != CharType('>')) {
                report_error("unexpected character: '>' not found");
            }
            return xml_tag(xml_tag::tag_open_element, tag_name, tag_attrs);
        }
    }
    xml_tag complete_close_tag_literal()
    {
        std::wstring tag_name = get_node_name();
        CharType ch = read_next_nwspace_char();
        if (ch != CharType('>')) {
            report_error("unexpected character: '>' not found");
        }
        return xml_tag(xml_tag::tag_close_element, tag_name);
    }
    xml_tag complete_comment_literal()
    {
        std::basic_string<CharType> s;
        CharType ch = read_next_nwspace_char();
        do {

            if (ch == _eof) {
                throw std::exception();
            }
            else if (ch == CharType('-')) {
                CharType ch2 = peek_next_char();
                if (ch2 == CharType('>')) {
                    read_next_char();
                    break;
                }
                else {
                }
            }
            else {
                s.push_back(ch);
            }

        } while (true);

        std::wstring ws = xml_detail::to_utf16(s);
        boost::algorithm::trim_if(ws, [](const wchar_t c) -> bool {return (c == L'-'); });
        boost::algorithm::trim(ws);

        return xml_tag(xml_tag::tag_comment, ws);
    }
    std::wstring complete_string_literal()
    {
        // Format: "xxxxxxx"
        std::basic_string<CharType> s;
        CharType ch = read_next_char();

        if (ch != CharType('\"')) {
            report_error("unexpected character: '\"' not found");
        }

        ch = read_next_nwspace_char();
        while (ch != CharType('\"')) {

            if (ch == _eof) {
                report_error("unexpected xml ending");
            }
            else if (ch == CharType('\\')) {
                ch = read_next_char();
                switch (ch)
                {
                case CharType('r'):
                    s.push_back(CharType('\r'));
                    break;
                case CharType('n'):
                    s.push_back(CharType('\n'));
                    break;
                case CharType('t'):
                    s.push_back(CharType('\t'));
                    break;
                case CharType('\"'):
                    s.push_back(CharType('\"'));
                    break;
                default:
                    report_error("unexpected character");
                    break;
                }
            }
            else {
                s.push_back(ch);
            }

            // move to next
            ch = read_next_char();
        }

        std::wstring ws = xml_detail::to_utf16(s);
        return std::move(ws);
    }

    static const CharType _eof = 0;

protected:
    size_t      _current_line;
    size_t      _current_column;
};

template <typename CharType>
class xml_string_parser : public xml_parser<CharType>
{
public:
    xml_string_parser(const std::basic_string<CharType>& s) : xml_parser<CharType>(), _position(s.c_str()), _startpos(s.c_str()), _endpos(s.c_str() + s.length())
    {
    }
    virtual ~xml_string_parser() {}


    virtual void reset_parser()
    {
        xml_parser::reset_parser();
        _position = _startpos;
    }

    virtual CharType read_next_char()
    {
        if (nullptr == _position)
            return xml_parser::_eof;
        if (*_position == 0 || _position == _endpos)
            return xml_parser::_eof;

        CharType ch = *_position;
        if (ch == CharType('\n')) {
            _current_line++;
            _current_column = 0;
        }
        else {
            _current_column++;
        }
        ++_position; // point to next
        return ch;
    }

    virtual CharType peek_next_char()
    {
        if (nullptr == _position)
            return xml_parser::_eof;
        if (*_position == 0 || _position == _endpos)
            return xml_parser::_eof;
        return (*_position);
    }

private:
    const CharType* _position;
    const CharType* _startpos;
    const CharType* _endpos;
};


//
//  class xml_attribute
//

xml_attribute::xml_attribute()
{
}
xml_attribute::xml_attribute(const std::wstring& n, const std::wstring& v) : _name(n), _value(v)
{
}
xml_attribute::xml_attribute(const std::string& n, const std::string& v) : _name(NX::conversion::utf8_to_utf16(n)), _value(NX::conversion::utf8_to_utf16(v))
{
}
xml_attribute::~xml_attribute()
{
}

xml_attribute& xml_attribute::operator = (const xml_attribute& other)
{
    if (this != &other) {
        _name = other.name();
        _value = other.value();
    }
    return *this;
}
bool xml_attribute::operator == (const xml_attribute& other) const
{
    return (this == &other) ? true : (other.name() == name());
}

//
//  class xml_node
//

xml_node::xml_node() : _type(node_null)
{
}

xml_node::xml_node(node_type t, const std::wstring& n, const std::wstring& s, const std::map<std::wstring, xml_attribute>& a) : _type(t), _name(n), _text(s), _attributes(a)
{
}

xml_node:: ~xml_node()
{
    clear();
}

std::shared_ptr<xml_node> xml_node::create_comment_node(const std::wstring& comment)
{
    return std::shared_ptr<xml_node>(new xml_comment(comment));
}

std::shared_ptr<xml_node> xml_node::create_processing_instruction_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a)
{
    return std::shared_ptr<xml_node>(new xml_processing_instruction(n, a));
}

std::shared_ptr<xml_node> xml_node::create_element_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a)
{
    return std::shared_ptr<xml_node>(new xml_element(n, a));
}

std::shared_ptr<xml_node> xml_node::create_text_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a, const std::wstring& t)
{
    return std::shared_ptr<xml_node>(new xml_element(n, a, t));
}

std::shared_ptr<xml_node> xml_node::create_attribute_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a)
{
    return std::shared_ptr<xml_node>(new xml_element(n, a));
}

void xml_node::clear()
{
    _type = node_null;
    _name.clear();
    _text.clear();
    _children.clear();
    _attributes.clear();
}

std::shared_ptr<xml_node> xml_node::insert_child(size_t index, const std::shared_ptr<xml_node>& node)
{
    if (children_count() < index) {
        throw std::exception("out of range");
    }

    if (index == 0) {
        _children.push_front(node);
    }
    else {
        auto pos = _children.begin();
        while (index > 0) {
            ++pos;
            --index;
        }
        pos = _children.insert(pos, node);
    }

    return node;
}

std::shared_ptr<xml_node> xml_node::remove_child(size_t index)
{
    if (children_count() <= index) {
        return std::shared_ptr<xml_node>(nullptr);
    }

    auto pos = _children.begin();
    while (index > 0) {
        ++pos;
        --index;
    }
    std::shared_ptr<xml_node> node = (*pos);
    _children.erase(pos);
    return node;
}

std::shared_ptr<xml_node> xml_node::find_child_element(const std::wstring& tag_name)
{
    auto pos = std::find_if(_children.begin(), _children.end(), [&](const std::shared_ptr<xml_node>& p)->bool {
        return (p->is_element_node() && 0 == _wcsicmp(tag_name.c_str(), p->get_name().c_str()));
    });
    return (pos == _children.end()) ? std::shared_ptr<xml_node>(nullptr) : (*pos);
}

std::shared_ptr<xml_node> xml_node::append_child(const std::shared_ptr<xml_node>& node)
{
    _children.push_back(node);
    return node;
}

std::wstring xml_node::serialize() const
{
    return std::wstring();
}

void xml_node::set_attribute(const std::wstring& n, const std::wstring& v)
{
    xml_attribute attr(n, v);
    std::wstring k(n);
    std::transform(k.begin(), k.end(), k.begin(), tolower);
    _attributes[attr.name()] = attr;
}

std::wstring xml_node::get_attribute(const std::wstring& n) const
{
    auto pos = std::find_if(_attributes.begin(), _attributes.end(), [&](const std::pair<std::wstring, xml_attribute>& item) -> bool {
        return (0 == _wcsicmp(n.c_str(), item.first.c_str()));
    });
    return (pos == _attributes.end()) ? std::wstring() : ((*pos).second.value());
}


//
//  class xml_element
//

xml_element::xml_element() : xml_node(node_element, std::wstring(), std::wstring(), std::map<std::wstring, xml_attribute>())
{
}

xml_element::xml_element(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a) : xml_node(node_element, n, std::wstring(), a)
{
}

xml_element::xml_element(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a, const std::wstring& t) : xml_node(node_element, n, t, a)
{
}

xml_element::~xml_element()
{
}

std::wstring xml_element::get_text() const
{
    if (!is_text_node()) {
        return std::wstring();
    }
    return xml_node::get_text();
}

void xml_element::set_text(const std::wstring& s)
{
    if (is_text_node() || is_attribute_node()) {
        xml_node::set_text(s);
    }
    else {
        throw std::exception("not a text node");
    }
}

std::shared_ptr<xml_node> xml_element::insert_child(size_t index, const std::shared_ptr<xml_node>& node)
{
    if (is_text_node()) {
        throw std::exception("cannot insert child to a text node");
    }
    return xml_node::insert_child(index, node);
}

std::shared_ptr<xml_node> xml_element::append_child(const std::shared_ptr<xml_node>& node)
{
    if (is_text_node()) {
        throw std::exception("cannot append child to a text node");
    }
    return xml_node::append_child(node);
}

std::shared_ptr<xml_node> xml_element::remove_child(size_t index)
{
    if (is_text_node()) {
        throw std::exception("cannot remove child from a text node");
    }
    return xml_node::remove_child(index);
}

std::wstring xml_element::serialize() const
{
    std::wstring body(L"<");
    body += get_name();
    std::for_each(attributes().begin(), attributes().end(), [&](const std::pair<std::wstring, xml_attribute>& item) {
        body += L" " + item.second.name() + L"=\"" + item.second.value() + L"\"";
    });

    if (is_attribute_node()) {
        body += L"/>";
    }
    else if (is_text_node()) {
        body += L">";
        body += get_text();
        body += L"</";
        body += get_name();
        body += L">";
    }
    else {
        assert(children_count() != 0);
        body += L">";
        std::for_each(children().begin(), children().end(), [&](const std::shared_ptr<xml_node>& sp) {
            body += sp->serialize();
        });
        body += L"</";
        body += get_name();
        body += L">";
    }

    return std::move(body);
}

//
//  class xml_comment
//
xml_comment::xml_comment() : xml_node(node_comment, std::wstring(), std::wstring(), std::map<std::wstring, xml_attribute>())
{
}

xml_comment::xml_comment(const std::wstring& comment) : xml_node(node_comment, std::wstring(), comment, std::map<std::wstring, xml_attribute>())
{
}

xml_comment::~xml_comment()
{
}

const std::list<std::shared_ptr<xml_node>>& xml_comment::children() const
{
    throw std::exception("comment node doesn't have child");
}

std::list<std::shared_ptr<xml_node>>& xml_comment::children()
{
    throw std::exception("comment node doesn't have child");
}

std::shared_ptr<xml_node> xml_comment::insert_child(size_t index, const std::shared_ptr<xml_node>& node)
{
    throw std::exception("comment node doesn't have child");
}

std::shared_ptr<xml_node> xml_comment::append_child(const std::shared_ptr<xml_node>& node)
{
    throw std::exception("comment node doesn't have child");
}

std::shared_ptr<xml_node> xml_comment::remove_child(size_t index)
{
    throw std::exception("comment node doesn't have child");
}

std::wstring xml_comment::serialize() const
{
    std::wstring body(L"<!-- ");
    body += get_text();
    body += L" -->";
    return std::move(body);
}

//
//  class xml_processing_instruction
//
xml_processing_instruction::xml_processing_instruction() : xml_node(node_processing_instruction, std::wstring(), std::wstring(), std::map<std::wstring, xml_attribute>())
{
}

xml_processing_instruction::xml_processing_instruction(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a) : xml_node(node_processing_instruction, n, std::wstring(), a)
{
}

xml_processing_instruction::~xml_processing_instruction()
{
}

std::wstring xml_processing_instruction::get_text() const
{
    throw std::exception("processing_instruction node doesn't have text");
}

void xml_processing_instruction::set_text(const std::wstring& s)
{
    throw std::exception("processing_instruction node doesn't have text");
}

std::shared_ptr<xml_node> xml_processing_instruction::insert_child(size_t index, const std::shared_ptr<xml_node>& node)
{
    throw std::exception("processing_instruction node doesn't have child");
}

std::shared_ptr<xml_node> xml_processing_instruction::append_child(const std::shared_ptr<xml_node>& node)
{
    throw std::exception("processing_instruction node doesn't have child");
}

std::shared_ptr<xml_node> xml_processing_instruction::remove_child(size_t index)
{
    throw std::exception("processing_instruction node doesn't have child");
}

std::wstring xml_processing_instruction::serialize() const
{
    std::wstring body(L"<");
    body += get_name();
    std::for_each(attributes().begin(), attributes().end(), [&](const std::pair<std::wstring, xml_attribute>& item) {
        body += L" " + item.second.name() + L"=\"" + item.second.value() + L"\"";
    });
    body += L"?>";
    return std::move(body);
}

//
//  class xml_document
//

xml_document::xml_document()
{
}

xml_document::~xml_document()
{
}

void xml_document::load_from_string(const std::wstring& s)
{
    xml_string_parser<wchar_t> xp(s);
    std::vector<std::shared_ptr<xml_node>> node_list = xp.parse();
    clear();
    std::for_each(node_list.begin(), node_list.end(), [&](const std::shared_ptr<xml_node>& node) {
        _root.append_child(node);
    });
}

void xml_document::load_from_string(const std::string& s)
{
    xml_string_parser<char> xp(s);
    std::vector<std::shared_ptr<xml_node>> node_list = xp.parse();
    clear();
    std::for_each(node_list.begin(), node_list.end(), [&](const std::shared_ptr<xml_node>& node) {
        _root.append_child(node);
    });
}

void xml_document::load_from_file(const std::wstring& file)
{
    std::vector<char> data;
    std::ifstream fp;
    fp.open(file, std::ifstream::binary);
    if (!fp.is_open() || !fp.good()) {
        throw std::exception("fail to open file");
    }

    // get length of file:
    fp.seekg(0, fp.end);
    size_t length = (size_t)fp.tellg();
    fp.seekg(0, fp.beg);


    if (0 == length) {
        fp.close();
        clear();
        return;
    }

    std::string s;
    fp.read(NX::string_buffer<char>(s, length + 1), length);
    fp.close();
    load_from_string(s);
}

void xml_document::to_file(const std::wstring& file)
{
    static const std::string xml_declare("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    std::string s = NX::conversion::utf16_to_utf8(serialize());
    std::ofstream fp;

    fp.open(file, std::ofstream::binary|std::ofstream::trunc);
    if (!fp.is_open() || !fp.good()) {
        throw std::exception("fail to open file");
    }
    fp.write(xml_declare.c_str(), xml_declare.length());
    if (s.length() > 0) {
        fp.write(s.c_str(), s.length());
    }
    fp.close();
}

void xml_document::clear()
{
    _root.clear();
}

std::wstring xml_document::serialize() const
{
    std::wstring body;
    std::for_each(_root.children().begin(), _root.children().end(), [&](const std::shared_ptr<xml_node>& node) {
        body += node->serialize();
    });
    return std::move(body);
}