

#ifndef __NXRM_PKG_PROFILE_CLASSIFY_HPP__
#define __NXRM_PKG_PROFILE_CLASSIFY_HPP__



#include <string>
#include <map>
#include "profile_basic.hpp"

namespace nxrm {
namespace pkg {


#define PROFILE_CLASSIFY_NODENAME           L"CLASSIFY_PROFILE"
#define PROFILE_CLASSIFY_ELEMENT_NODENAME   L"ELEMENT"
#define PROFILE_CLASSIFY_VALUE_NODENAME     L"VALUE"
#define PROFILE_CLASSIFY_NAME_ATTRIBUTE     L"name"
#define PROFILE_CLASSIFY_DEFAULT_ATTRIBUTE  L"default"
#define PROFILE_CLASSIFY_MANDATORY_ATTRIBUTE    L"mandatory"
#define PROFILE_CLASSIFY_MULTISELECT_ATTRIBUTE  L"multi-select"

/*
    <CLASSIFY_PROFILE>
        <!-- Level #0, Item #0 -->
        <ELEMENT name="" default="yes">
            <!-- Level #1, Item #0-0 -->
            <ELEMENT name="" multi-select="yes">
                <VALUE name="" default="yes" />
                <VALUE name="" />
                <VALUE name="" />
            </ELEMENT>
            <!-- Level #1, Item #0-1 -->
            <ELEMENT name="" multi-select="no">
                <VALUE name="" default="yes" />
                <VALUE name="" desc="" />
                <VALUE name="" desc="" />
            </ELEMENT>
            <!-- Level #1, Item #0-2 -->
            <ELEMENT name="" desc="">                
                <!-- Level #1, Item #0-2-0 -->
                <ELEMENT name="" multi-select="no">
                    <VALUE name="" default="yes" />
                    <VALUE name="" />
                    <VALUE name="" />
                </ELEMENT>
            </ELEMENT>
        </ELEMENT>
        <!-- Level #0, Item #1 -->
        <ELEMENT name="">
        </ELEMENT>
        <!-- Level #0, Item #2 -->
        <ELEMENT name="">
        </ELEMENT>
    </CLASSIFY_PROFILE>
*/

class CValue
{
public:
    CValue() : _default(false)
    {
    }
    CValue(const std::wstring& name, bool is_default) : _name(name), _default(is_default)
    {
    }
    virtual ~CValue()
    {
    }

    inline const std::wstring& GetName() const throw() {return _name;}
    inline bool IsDefault() const throw() {return _default;}

    CValue& operator = (const CValue& value)
    {
        if(this != &value) {
            _name = value.GetName();
            _default = value.IsDefault();
        }
        return *this;
    }

private:
    std::wstring    _name;
    bool            _default;
};

class CElement : public CValue
{
public:
    CElement();
    virtual ~CElement();

    void InsertElement(const CElement& e) throw();
    void InsertValue(const CValue& e) throw();

    inline bool IsLeaf() const throw() {return (_subelems.empty());}
    inline bool IsMultiSelectAllowed() const throw() {return _multiselect;}
    inline const std::vector<CElement>& GetSubElements() const throw() {return _subelems;}
    inline const std::vector<CValue>& GetSubValues() const throw() {return _subvalues;}

private:
    bool                  _multiselect;
    std::vector<CElement> _subelems;
    std::vector<CValue>   _subvalues;
};

class CClassifyProfile : public CProfile
{
public:
    CClassifyProfile();
    virtual ~CClassifyProfile();
    
    inline const std::vector<CElement>& GetElements() const {return _elems;}
    inline std::vector<CElement>& GetElements() {return _elems;}

    inline CClassifyProfile& operator =(const CClassifyProfile& profile) throw()
    {
        if(this != &profile) {
            ((CProfile*)this)->operator=(profile);
            _elems = profile.GetElements();
        }
        return *this;
    }
        
    void FromXml(IXMLDOMNode* node);
    void ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const;

private:
    std::vector<CElement>   _elems;
};

    
}   // namespace nxrm::pkg
}   // namespace nxrm


#endif  // __NXRM_PKG_PROFILE_BASIC_HPP__