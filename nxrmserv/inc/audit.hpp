


#ifndef __NXRM_SERV_AUDIT_HPP__
#define __NXRM_SERV_AUDIT_HPP__


#include <string>
#include <iostream>
#include <fstream>

#include "policy.hpp"


#define AUDIT_TYPE_LOG_AUTHORIZATION        0x00000001
#define AUDIT_TYPE_LOG_OPERATION            0x00000002


#define AUTHORIZATION_NAME                  L"Authorization"

namespace NX {


class audit_writer
{
public:
    audit_writer();
    virtual ~audit_writer();

    inline bool is_open() noexcept { return (INVALID_HANDLE_VALUE != _fh); }

    bool open(const std::wstring& file, bool truncate);
    void close() noexcept;
    bool write(const std::string& s) noexcept;
    void flush();
    void reset();
    void get_lines(std::vector<std::string>& lines);

private:
    HANDLE  _fh;
};

class audit_archiver
{
public:
    audit_archiver();
    virtual ~audit_archiver();

    inline bool is_open() noexcept { return _fp.is_open(); }

    bool open(const std::wstring& dir);
    void close() noexcept;
    void save(const std::vector<std::string>& records);
    
protected:
    void check_datetime();

protected:
    unsigned long get_current_dtflag() const noexcept;
    std::wstring dtflag_to_filename(unsigned long stflag) const noexcept;

private:
    std::ofstream   _fp;
    unsigned long   _dtflag;
    std::wstring    _dir;
};

class audit_uploader
{
public:
    audit_uploader();
    virtual ~audit_uploader();
    bool upload(const std::vector<std::string>& records) noexcept;
};

class serv_audit
{
public:
    serv_audit();
    virtual ~serv_audit();

    void clear() noexcept;
    void start(const std::wstring& work_dir);
    void stop();

    void audit(const NX::EVAL::eval_object& eo);
    void upload();

    inline bool active() const noexcept { return _active; }

protected:
    std::wstring generate_id() const noexcept;
    std::wstring generate_timestamp() const noexcept;
    void get_upload_records(std::vector<std::string>& records);

private:
    audit_writer    _writer;
    audit_archiver  _archiver;
    audit_uploader  _uploader;
    std::wstring    _workdir;
    std::wstring    _record_file;
    std::wstring    _cache_file;
    CRITICAL_SECTION    _cache_lock;
    bool            _active;
};



}   // namespace NX



#endif