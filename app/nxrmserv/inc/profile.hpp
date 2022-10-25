

#pragma once
#ifndef __NXRM_PROFILE_HPP__
#define __NXRM_PROFILE_HPP__


#include <string>
#include <vector>

#include <nudf\time.hpp>

class rmprofile
{
public:
    rmprofile();
    ~rmprofile();

    inline bool empty() const { return _user_id.empty(); }
    inline const std::wstring& get_tenant_id() const { return _tenant_id; }
    inline const std::wstring& get_user_id() const { return _user_id; }

    rmprofile& operator = (const rmprofile& other);

    void load(const std::wstring& user_id);
    void clear();
    bool expired() const;

private:
    std::wstring    _tenant_id;
    std::wstring    _user_id;
    NX::time::datetime  _expire_time;
};

class rmprofile_list
{
public:
    rmprofile_list();
    ~rmprofile_list();

    rmprofile_list& operator = (const rmprofile_list& other);

    void load_file(const std::wstring& file);
    void load(const std::wstring& json_string);
    void save();
    std::wstring serialize() const;

    void set_active_profile(const std::wstring& profile_name, const std::string& profile_token, const std::wstring& exprie_time);
    void set_active_profile(const std::wstring& profile_name, const std::string& profile_token, const FILETIME& exprie_time);

private:
    std::wstring        _active_profile_name;
    NX::time::datetime  _active_profile_expire_time;
    std::string         _active_profile_token;
    std::vector<std::wstring>   _profile_list;
};



#endif