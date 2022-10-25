

#ifndef __NXREST_XSTYPE_AGENT_TYPE_HPP__
#define __NXREST_XSTYPE_AGENT_TYPE_HPP__


#include <string>


namespace nxrmrest {
namespace schema {
namespace type {


typedef enum _AGENT_TYPE {
    AGENT_UNKNOWN = 0,
    AGENT_DESKTOP,
    AGENT_LAPTOP,
    AGENT_PHONE,
    AGENT_TABLET
} AGENT_TYPE;


class CAgentType
{
public:
    CAgentType() : _type(AGENT_UNKNOWN)
    {
    }
    CAgentType(_In_ const std::wstring& type) : _type(AGENT_UNKNOWN)
    {
        if(0 == _wcsicmp(type.c_str(), L"DESKTOP")) {
            _type = AGENT_DESKTOP;
        }
        else if(0 == _wcsicmp(type.c_str(), L"LAPTOP")) {
            _type = AGENT_LAPTOP;
        }
        else if(0 == _wcsicmp(type.c_str(), L"PHONE")) {
            _type = AGENT_PHONE;
        }
        else if(0 == _wcsicmp(type.c_str(), L"TABLET")) {
            _type = AGENT_TABLET;
        }
        else {
            _type = AGENT_UNKNOWN;
        }
    }

    ~CAgentType()
    {
    }

    inline AGENT_TYPE GetType() const throw() {return _type;}
    inline void SetType(AGENT_TYPE type) throw() {_type = type;}
    CAgentType& operator = (const CAgentType& at) throw()
    {
        if(this != &at) {
            _type = at.GetType();
        }
        return *this;
    }
    bool operator == (const CAgentType& at) throw()
    {
        return (_type == at.GetType());
    }
    
    inline std::wstring GetName() const throw()
    {
        switch(_type)
        {
        case AGENT_DESKTOP: return L"DESKTOP";
        case AGENT_LAPTOP: return L"LAPTOP";
        case AGENT_PHONE: return L"PHONE";
        case AGENT_TABLET: return L"TABLET";
        case AGENT_UNKNOWN:
        default: break;
        }
        return L"UNKNOWN";
    }

    void Clear() throw()
    {
        _type = AGENT_UNKNOWN;
    }

private:
    AGENT_TYPE  _type;
};

class CGroupInfo
{
public:
    CGroupInfo()
    {
    }
    ~CGroupInfo()
    {
    }

    inline int GetId() const throw() {return _id;}
    inline void SetId(int id) throw() {_id = id;}
    inline const std::wstring& GetName() const throw() {return _name;}
    inline void SetName(const std::wstring& name) throw() {_name = name;}

    CGroupInfo& operator = (const CGroupInfo& info) throw()
    {
        if(this != &info) {
            _id = info.GetId();
            _name = info.GetName();
        }
        return *this;
    }

    bool operator == (const CGroupInfo& info) throw()
    {
        return (_id == info.GetId());
    }

private:
    int     _id;
    std::wstring _name;
};

}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_AGENT_TYPE_HPP__