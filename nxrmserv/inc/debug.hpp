


#ifndef __NXSERV_DEBUG_HPP__
#define __NXSERV_DEBUG_HPP__

#include <string>

namespace NX {


class debug_object
{
public:
    debug_object();
    debug_object(unsigned long session_id);
    virtual ~debug_object();

    bool create() noexcept;

    inline const std::wstring& file() const { return _file; }
    inline const std::wstring& file_name() const { return _file_name; }

protected:
    void generate_file_name(unsigned long session_id) noexcept;
    std::wstring generate_temp_dir();
    bool generate_file(const std::wstring& file, const std::string& content);
    void decrypt_profiles(const std::wstring& wsdir);
    void copy_dump_profiles(const std::wstring& wsdir);
    void find_sub_files(const std::wstring& root_dir, std::vector<std::wstring>& dirs, std::vector<std::wstring>& files);
    bool remove_directory(const std::wstring& folder);
    bool remove_file(const std::wstring& file);
    bool create_zip(const std::wstring& wsdir, const std::wstring& zipfile);
    bool create_empty_zip(const std::wstring& zipfile) noexcept;

private:
    std::wstring _file;
    std::wstring _file_name;
    unsigned long _session_id;
};


}

#endif