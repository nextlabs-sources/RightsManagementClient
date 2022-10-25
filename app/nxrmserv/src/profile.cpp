

#include <Windows.h>


#include "profile.hpp"


rmprofile::rmprofile()
{
}

rmprofile::~rmprofile()
{
}

rmprofile& rmprofile::operator = (const rmprofile& other)
{
    if (this != &other) {
        _tenant_id = other.get_tenant_id();
        _user_id = other.get_user_id();
    }
    return *this;
}

void rmprofile::clear()
{
}

void rmprofile::load(const std::wstring& user_id)
{
}

bool rmprofile::expired() const
{
    return true;
}


//
//
//

rmprofile_list::rmprofile_list()
{
}

rmprofile_list::~rmprofile_list()
{
}

rmprofile_list& rmprofile_list::operator = (const rmprofile_list& other)
{
    if (this != &other) {
        ; //
    }
    return *this;
}

void rmprofile_list::load_file(const std::wstring& file)
{
}

void rmprofile_list::load(const std::wstring& json_string)
{
}

void rmprofile_list::save()
{
}

std::wstring rmprofile_list::serialize() const
{
    std::wstring s;
    return std::move(s);
}

void rmprofile_list::set_active_profile(const std::wstring& profile_name, const std::string& profile_token, const std::wstring& exprie_time)
{
}

void rmprofile_list::set_active_profile(const std::wstring& profile_name, const std::string& profile_token, const FILETIME& exprie_time)
{
}
