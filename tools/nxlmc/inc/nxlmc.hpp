


#ifndef __NXL_MC_HPP__
#define __NXL_MC_HPP__


#include <string>
#include <vector>
#include <map>
#include <set>

namespace NX {

   
class commandline
{
public:
    commandline();
    commandline(const std::wstring& line);
    virtual ~commandline();

    inline const std::wstring& command() const noexcept { return _command; }
    inline const std::map<std::wstring, std::vector<std::wstring>>& parameters() const noexcept { return _parameters; }

    commandline& operator = (const commandline& line);

    std::wstring get_next_part(std::wstring& line, const wchar_t sp);

private:
    std::wstring    _command;
    std::map<std::wstring, std::vector<std::wstring>>   _parameters;
};

class nxlmc
{
public:
    nxlmc();
    virtual ~nxlmc();

    void usage(const std::wstring& command);
    void run();
    int exec(const commandline& cmdl);

protected:
    int create(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int decrypt(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int verify(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int show(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int section_resize(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int section_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int section_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int attribute_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int attribute_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int template_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int template_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int template_set_remote(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int tag_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters);
    int tag_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters);

private:
    int create_empty_file(const std::wstring& file, const std::vector<std::pair<std::wstring, unsigned long>>& sections);
    int convert_file(const std::wstring& in_file, const std::wstring& out_file, const std::vector<std::pair<std::wstring, unsigned long>>& sections);
    int decrypt_file(const std::wstring& in_file, const std::wstring& out_file);
};


}


#endif