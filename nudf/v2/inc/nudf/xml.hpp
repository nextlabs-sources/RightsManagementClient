

#pragma once
#ifndef __NUDF_XML_HPP__
#define __NUDF_XML_HPP__




#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <algorithm>


namespace NX {


class xml_attribute
{
public:
    xml_attribute();
    xml_attribute(const std::wstring& n, const std::wstring& v);
    xml_attribute(const std::string& n, const std::string& v);
    ~xml_attribute();

    xml_attribute& operator = (const xml_attribute& other);
    bool operator == (const xml_attribute& other) const;

    inline const std::wstring& name() const { return _name; }
    inline const std::wstring& value() const { return _value; }
    inline bool empty() const { return _name.empty(); }
    inline void clear() { _name.clear(); _value.clear(); }

private:
    std::wstring _name;
    std::wstring _value;
};


class xml_node
{
public:
    virtual ~xml_node();

    typedef enum node_type {
        node_null = 0,
        node_element = 1,
        node_processing_instruction,
        node_comment
    } node_type;

    static std::shared_ptr<xml_node> create_comment_node(const std::wstring& comment);
    static std::shared_ptr<xml_node> create_processing_instruction_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a);
    static std::shared_ptr<xml_node> create_element_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a);
    static std::shared_ptr<xml_node> create_text_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a, const std::wstring& t);
    static std::shared_ptr<xml_node> create_attribute_node(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a);

    inline node_type type() const { return _type; }
    inline bool is_element_node() const { return (node_element == type()); }
    inline bool is_comment_node() const { return (node_comment == type()); }
    inline bool is_processing_instruction_node() const { return (node_processing_instruction == type()); }
    inline bool is_text_node() const { return (is_element_node() && _children.empty() && !_text.empty()); }
    inline bool is_attribute_node() const { return (is_element_node() && _children.empty() && _text.empty()); }

    inline size_t children_count() const { return _children.size(); }
    inline size_t attributes_count() const { return _attributes.size(); }
    
    virtual void clear();
    
    virtual std::wstring get_name() const { return _name; }
    virtual std::wstring get_text() const { return _text; }
    virtual void set_text(const std::wstring& s) { _text = s; }

    virtual std::wstring get_attribute(const std::wstring& n) const;
    virtual void set_attribute(const std::wstring& n, const std::wstring& v);

    virtual std::shared_ptr<xml_node> insert_child(size_t index, const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> append_child(const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> remove_child(size_t index);
    virtual std::shared_ptr<xml_node> find_child_element(const std::wstring& tag_name);


    virtual std::wstring serialize() const;

protected:
    xml_node();
    xml_node(node_type t, const std::wstring& n, const std::wstring& s, const std::map<std::wstring, xml_attribute>& a);
    xml_node& operator = (const xml_node& other) { throw std::exception("no copy is allowed"); }
    xml_node& operator = (xml_node&& other) { throw std::exception("no copy is allowed"); }
    const std::list<std::shared_ptr<xml_node>>& children() const { return _children; }
    std::list<std::shared_ptr<xml_node>>& children() { return _children; }
    const std::map<std::wstring, xml_attribute>& attributes() const { return _attributes; }
    std::map<std::wstring, xml_attribute>& attributes() { return _attributes; }

private:
    node_type                               _type;
    std::wstring                            _name;
    std::wstring                            _text;
    std::list<std::shared_ptr<xml_node>>    _children;
    std::map<std::wstring, xml_attribute>   _attributes;

    friend class xml_document;
};

class xml_element : public xml_node
{
public:
    xml_element();
    xml_element(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a);
    xml_element(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a, const std::wstring& t);
    virtual ~xml_element();

    virtual std::wstring get_text() const;
    virtual void set_text(const std::wstring& s);
        
    virtual std::shared_ptr<xml_node> insert_child(size_t index, const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> append_child(const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> remove_child(size_t index);

    virtual std::wstring serialize() const;

private:
};

class xml_comment : public xml_node
{
public:
    xml_comment();
    xml_comment(const std::wstring& comment);
    virtual ~xml_comment();
    
    virtual const std::list<std::shared_ptr<xml_node>>& children() const;
    virtual std::list<std::shared_ptr<xml_node>>& children();
    
    virtual std::shared_ptr<xml_node> insert_child(size_t index, const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> append_child(const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> remove_child(size_t index);
    virtual std::wstring serialize() const;

private:
};

class xml_processing_instruction : public xml_node
{
public:
    xml_processing_instruction();
    xml_processing_instruction(const std::wstring& n, const std::map<std::wstring, xml_attribute>& a);
    virtual ~xml_processing_instruction();

    virtual std::wstring get_text() const;
    virtual void set_text(const std::wstring& s);
        
    virtual std::shared_ptr<xml_node> insert_child(size_t index, const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> append_child(const std::shared_ptr<xml_node>& node);
    virtual std::shared_ptr<xml_node> remove_child(size_t index);

    virtual std::wstring serialize() const;

private:
};

class xml_document
{
public:
    xml_document();
    ~xml_document();

    void load_from_string(const std::wstring& s);
    void load_from_string(const std::string& s);
    void load_from_file(const std::wstring& file);
    void to_file(const std::wstring& file);
    void clear();

    inline bool empty() const { return (_root.children().empty()); } 
    std::shared_ptr<xml_node> document_root()
    {
        auto pos = std::find_if(_root.children().begin(), _root.children().end(), [](const std::shared_ptr<xml_node>& node)->bool {
            return node->is_element_node();
        });
        return (pos != _root.children().end()) ? (*pos) : std::shared_ptr<xml_node>(nullptr);
    }
    
    std::wstring serialize() const;

private:
    // no copy allowed
    xml_document& operator = (const xml_document& other) { return *this; }

private:
    xml_node _root;
};


}


#endif