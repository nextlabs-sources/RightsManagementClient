
#ifndef __NXSERV_PROFILE_HPP__
#define __NXSERV_PROFILE_HPP__


#include <string>
#include <map>

namespace NX {


class authn_token
{
public:
    authn_token() : _expire_time(0) {}
    authn_token(const std::wstring& s);
    authn_token(const std::wstring& user_name, const std::wstring& principle_name, const std::wstring& user_id, const std::wstring& user_domain, unsigned long time_to_live = 2880 /*in minutes, default is 2 days*/);
    authn_token(const std::wstring& user_name, const std::wstring& principle_name, const std::wstring& user_id, const std::wstring& user_domain, const FILETIME& ft);
    ~authn_token() {}

    inline const std::wstring& user_name() const noexcept { return _user_name; }
    inline const std::wstring& principle_name() const noexcept { return _user_principle_name; }
    inline const std::wstring& user_id() const noexcept { return _user_id; }
    inline const std::wstring& user_domain() const noexcept { return _user_domain; }
    inline unsigned __int64 expire_time() const noexcept { return _expire_time; }
    inline bool never_expire() const noexcept { return (0 == _expire_time); }
    inline std::wstring full_user_name() const noexcept
    {
        return _user_principle_name.empty() ? build_full_name() : _user_principle_name;
    }
    inline std::wstring netbios_user_name() const noexcept
    {
        std::wstring netbiosname;
        if (!user_domain().empty() && NULL == wcschr(user_domain().c_str(), L'.')) {
            netbiosname = user_domain() + L"\\" + user_name();
        }
        return netbiosname;
    }

    authn_token& operator = (const authn_token& other)
    {
        if (this != &other) {
            _user_name = other.user_name();
            _user_principle_name = other.principle_name();
            _user_id = other.user_id();
            _user_domain = other.user_domain();
            _expire_time = other.expire_time();
        }
        return *this;
    }

    bool expired() const noexcept;
    void clear() noexcept;
    bool empty() const noexcept;
    std::wstring serialize() const noexcept;
    void load(const std::wstring& ws);
    void set_expire() noexcept;

protected:
    std::wstring build_full_name() const noexcept
    {
        if (_user_name.empty() || _user_domain.empty()) {
            return _user_name;
        }
        std::wstring full_name;
        if (NULL == wcschr(_user_domain.c_str(), L'.')) {
            full_name = _user_domain + L"\\" + _user_name;
        }
        else {
            full_name = _user_name + L"@" + _user_domain;
        }
        return full_name;
    }

private:
    std::wstring     _user_name;
    std::wstring     _user_principle_name;
    std::wstring     _user_id;
    std::wstring     _user_domain;
    unsigned __int64 _expire_time;
};

class profile
{
public:
    profile();
    profile(const std::wstring& id);
    virtual ~profile();
    
    enum AUTHNTYPE {
        LocalAuthn = 0,
        ExternalAuthn
    };

    bool operator == (const profile& other) const noexcept
    {
        if(this != &other) {
            return (_id == other.id());
        }
        else {
            return true;
        }
    }

    profile& operator = (const profile& other)
    {
        if(this != &other) {
            _id = other.id();
            _name = other.name();
            _domain = other.domain();
            _attributes = other.attributes();
            _authn_type = other.authn_type();
            _authn_token = other.token();
            _authn_attributes = other.authn_attributes();
        }
        return *this;
    }

    inline void clear() noexcept
    {
        _id.clear();
        _name.clear();
        _domain.clear();
        _attributes.clear();
        _authn_attributes.clear();
        _authn_token.clear();
        _authn_type = LocalAuthn;
        _dirty = false;
    }

    std::wstring authn_type_string() const noexcept
    {
        switch (authn_type())
        {
        case ExternalAuthn: return L"External";
        case LocalAuthn: default: break;
        }
        return L"Local";
    }

    inline bool dirty() const noexcept {return _dirty;}
    inline const std::wstring& id() const noexcept {return _id;}
    inline const std::wstring& name() const noexcept {return _name;}
    inline const std::wstring& domain() const noexcept {return _domain;}
    inline AUTHNTYPE authn_type() const noexcept {return _authn_type;}
    inline const authn_token& token() const noexcept { return _authn_token; }
    inline authn_token& token() noexcept { return _authn_token; }
    inline const std::map<std::wstring, std::wstring> attributes() const noexcept { return _attributes; }
    inline std::map<std::wstring, std::wstring> attributes() noexcept { return _attributes; }
    inline const std::map<std::wstring, std::wstring> authn_attributes() const noexcept { return _authn_attributes; }
    inline std::map<std::wstring, std::wstring> authn_attributes() noexcept { return _authn_attributes; }

    inline void mark_dirty() noexcept { _dirty = true; }
    inline void set_id(const std::wstring& id) noexcept { _id = id; std::transform(_id.begin(), _id.end(), _id.begin(), toupper); }
    inline void set_name(const std::wstring& name) noexcept { _name = name; std::transform(_name.begin(), _name.end(), _name.begin(), tolower); }
    inline void set_domain(const std::wstring& domain) noexcept { _domain = domain; std::transform(_domain.begin(), _domain.end(), _domain.begin(), tolower); }
    inline void set_authn_token(const authn_token& token) noexcept { _authn_token = token; }
    inline void set_authn_type(AUTHNTYPE authn_type) noexcept { _authn_type = authn_type; }

    inline bool empty() const noexcept {return (_name.empty() || _id.empty());}
    inline bool logged_on() const noexcept
    {
        if (empty()) {
            return false;
        }
        if (_authn_type == LocalAuthn) {
            return true;
        }
        return (!_authn_token.empty());
    }
    inline bool expired() const noexcept { return token().expired(); }

    inline bool is_local_authn() const noexcept { return (_authn_type == LocalAuthn); }
    inline bool is_external_authn() const noexcept { return (_authn_type == ExternalAuthn); }


    static bool exists(const std::wstring& id) noexcept;
    void load(const std::wstring& id);
    std::wstring serialize() noexcept;
    bool save() noexcept;

    // attributes
    bool has_attribute(const std::wstring& name) const noexcept;
    std::wstring get_attribute(const std::wstring& name) const noexcept;


private:
    bool            _dirty;
    std::wstring    _id;
    std::wstring    _name;
    std::wstring    _domain;
    AUTHNTYPE       _authn_type;
    NX::authn_token _authn_token;
    std::map<std::wstring, std::wstring> _attributes;
    std::map<std::wstring, std::wstring> _authn_attributes;
};



}   // namespace NX


#endif