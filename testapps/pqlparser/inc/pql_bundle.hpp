

#ifndef __NX_PQL_BUNDLE_HPP__
#define __NX_PQL_BUNDLE_HPP__

#include <string>
#include <set>
#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>


namespace pql {

namespace bundle {

class _Policy;

class _Bundle
{
public:
    _Bundle() : _timestamp(0),_unknown_user_id(-1), _unknown_app_id(-1), _unknown_host_id(-1)
    {
    }

    virtual ~_Bundle()
    {
    }

    static const std::string TAG_DEPLOYMENT_TIME;
    static const std::string TAG_DEPLOYMENT_ENTITIES;
    static const std::string TAG_POLICY_TO_ALL_USERS;
    static const std::string TAG_POLICY_TO_ALL_HOSTS;
    static const std::string TAG_POLICY_TO_ALL_APPLICATIONS;
    static const std::string TAG_SUBJECT_TO_POLICY_MAP;
    static const std::string TAG_SUBJECT_KEYS;

    void clear() noexcept { _policy_map.clear(); }

    void parse(std::basic_istream<char>& ss);

    inline unsigned __int64 timestamp() const noexcept { return _timestamp;}
    inline const std::map<__int64, std::shared_ptr<_Policy>>& policy_map() const noexcept { return _policy_map; }
    inline std::map<__int64, std::shared_ptr<_Policy>>& policy_map() noexcept { return _policy_map; }


    inline const std::set<__int64>&   policy_to_all_user() const noexcept { return _policy_to_all_user; }
    inline const std::set<__int64>&   policy_to_all_host() const noexcept { return _policy_to_all_host; }
    inline const std::set<__int64>&   policy_to_all_apps() const noexcept { return _policy_to_all_apps; }
    inline const std::set<__int64>&   policy_to_unknown_users() const noexcept { return _policy_to_unknown_users; }
    inline const std::set<__int64>&   policy_to_unknown_apps() const noexcept { return _policy_to_unknown_apps; }
    inline const std::set<__int64>&   policy_to_unknown_hosts() const noexcept { return _policy_to_unknown_hosts; }
    inline unsigned __int64 unknown_user_id() const noexcept{ return _unknown_user_id; }
    inline unsigned __int64 unknown_app_id() const noexcept{ return _unknown_app_id; }
    inline unsigned __int64 unknown_host_id() const noexcept{ return _unknown_host_id; }

    inline const std::map<std::string, __int64>& subject_id_map() const noexcept { return _subject_id_map; }
    inline std::map<std::string, __int64>& subject_id_map() noexcept { return _subject_id_map; }
    inline const std::map<std::string, std::set<__int64>> policy_to_subject() const noexcept { return _policy_to_subject; }
    inline std::map<std::string, std::set<__int64>> policy_to_subject() noexcept { return _policy_to_subject; }

private:
    void get_nextline(std::basic_istream<char>& ss, std::string& line);

    template<typename T, class _Pr>
    void line_to_set(std::string& line, const char seperator, std::set<T>& set, _Pr _pred)
    {
        while (!line.empty()) {
            std::string e;
            std::string::size_type pos  = line.find(seperator);
            if (std::string::npos != pos) {
                e = line.substr(0, pos);
                line = line.substr(pos + 1);
            }
            else {
                e = line;
                line = "";
            }
            
            try {
                boost::algorithm::trim(e);
                T t = _pred(e);
                set.insert(t);
            }
            catch (...) {
                ;// exception, not add
            }
        }
    }

    template <typename CharType>
    bool is_digit(const std::basic_string<CharType>& s)
    {
        return (s.end() != std::find_if_not(s.begin(), s.end(), [](const CharType it) -> bool {
            return (it >= static_cast<CharType>('0') && it <= static_cast<CharType>('9')); 
        })) ? false : true;
    }

    template <typename CharType>
    bool is_hex(const std::basic_string<CharType>& s)
    {
        return (s.end() != std::find_if_not(s.begin(), s.end(), [](const CharType it) -> bool {
            return ((it >= CharType('0') && it <= CharType('9')) | (it >= CharType('a') && it <= CharType('f')) | (it >= CharType('A') && it <= CharType('F')));
        })) ? false : true;
    }

private:
    unsigned __int64 _timestamp;
    std::map<__int64,std::shared_ptr<_Policy>> _policy_map;
    std::set<__int64>   _policy_to_all_user;
    std::set<__int64>   _policy_to_all_host;
    std::set<__int64>   _policy_to_all_apps;

    std::set<__int64>   _policy_to_unknown_users;
    std::set<__int64>   _policy_to_unknown_apps;
    std::set<__int64>   _policy_to_unknown_hosts;
    __int64             _unknown_user_id;
    __int64             _unknown_app_id;
    __int64             _unknown_host_id;

    std::map<std::string, __int64> _subject_id_map;
    std::map<std::string,std::set<__int64>> _policy_to_subject;
};

class _Policy
{
public:
    _Policy() : _id(-1), _approved(false), _hidden(false) {}
    _Policy(const std::string& s);
    virtual ~_Policy() {}

    inline int id() const noexcept { return _id; }
    inline bool approved() const noexcept { return _approved; }
    inline bool hidden() const noexcept { return _hidden; }
    inline const std::string& name() const noexcept { return _name; }
    inline const std::string& attribute() const noexcept { return _attribute; }

    inline bool is_valid() const noexcept { return (_id > 0 && !_name.empty()); }

    void clear();
    void add_line(const std::string& s);

    _Policy& operator = (const _Policy& other)
    {
        if (this != &other) {
            _id = other.id();
            _approved = other.approved();
            _hidden = other.hidden();
            _name = other.name();
            _attribute = other.attribute();
        }
        return *this;
    }

    static const std::string TAG_ID;
    static const std::string TAG_DESCRIPTION;
    static const std::string TAG_ATTRIBUTE;
    static const std::string TAG_FOR;
    static const std::string TAG_ON;
    static const std::string TAG_TO;
    static const std::string TAG_BY;
    static const std::string TAG_WHERE;
    static const std::string TAG_DO;
    static const std::string TAG_ON_ALLOW_DO;
    static const std::string TAG_ON_DENY_DO;

private:
    void set_title(const std::string& s);

private:
    int         _id;
    bool        _approved;
    bool        _hidden;
    std::string _name;
    std::string _attribute;
    std::map<std::string, std::string> _map;
};


namespace details {




}


}   // namespace pql::bundle
}   // namespace pql


#endif