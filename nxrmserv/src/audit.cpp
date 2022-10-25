
#include <windows.h>

#include <boost\algorithm\string.hpp>

#include <sstream>

#include <nudf\string.hpp>
#include <nudf\time.hpp>

#include "nxrmserv.h"
#include "rest.hpp"
#include "audit.hpp"


using namespace NX;



//
//  class audit_writer
//

audit_writer::audit_writer() : _fh(INVALID_HANDLE_VALUE)
{
}

audit_writer::~audit_writer()
{
    close();
}

bool audit_writer::open(const std::wstring& file, bool truncate)
{
    _fh = ::CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, truncate ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
    if (INVALID_HANDLE_VALUE == _fh) {
        return false;
    }

    SetFilePointer(_fh, 0, NULL, FILE_END);
    return true;
}

void audit_writer::close() noexcept
{
    if (INVALID_HANDLE_VALUE != _fh) {
        CloseHandle(_fh);
        _fh = INVALID_HANDLE_VALUE;
    }
}

bool audit_writer::write(const std::string& s) noexcept
{
    unsigned long bytes_written = 0;
    assert(is_open());
    if (s.length() == 0) {
        return true;
    }
    bool result = ::WriteFile(_fh, s.c_str(), (unsigned long)s.length(), &bytes_written, NULL) ? true : false;
    ::WriteFile(_fh, "\r\n", 2, &bytes_written, NULL);
    return result;
}

void audit_writer::flush()
{
    if (INVALID_HANDLE_VALUE != _fh) {
        ::FlushFileBuffers(_fh);
    }
}

void audit_writer::reset()
{
    if (INVALID_HANDLE_VALUE != _fh) {
        ::SetFilePointer(_fh, 0, NULL, FILE_BEGIN);
        ::SetEndOfFile(_fh);
    }
}

void audit_writer::get_lines(std::vector<std::string>& lines)
{
    if (INVALID_HANDLE_VALUE == _fh) {
        return;
    }

    unsigned long file_size = GetFileSize(_fh, NULL);
    if (file_size == 0) {
        return;
    }

    SetFilePointer(_fh, 0, NULL, FILE_BEGIN);

    std::string buf;
    unsigned long bytes_read = 0;
    buf.resize(file_size, 0);
    if (!::ReadFile(_fh, &buf[0], file_size, &bytes_read, NULL)) {
        return;
    }

    std::stringstream ss(std::move(buf));
    std::string line;
    while (std::getline(ss, line)) {
        boost::algorithm::trim(line);
        if (line.empty()) {
            continue;
        }
        lines.push_back(line);
    }
}

//
//  class audit_saver
//

audit_archiver::audit_archiver() : _dtflag(get_current_dtflag())
{
}

audit_archiver::~audit_archiver()
{
    close();
}

bool audit_archiver::open(const std::wstring& dir)
{
    // Record the directory, so that check_datetime() knows where we should
    // create a new file when the dtflag changes.
    _dir = dir;

    std::wstring filename(dir);

    if (!boost::algorithm::ends_with(filename, L"\\")) {
        filename += L"\\";
    }
    filename += dtflag_to_filename(_dtflag);

    _fp.open(filename, std::ofstream::binary | std::ofstream::app, _SH_DENYWR);
    return _fp.is_open();
}

void audit_archiver::close() noexcept
{
    if (_fp.is_open()) {
        _fp.close();
    }
}

void audit_archiver::save(const std::vector<std::string>& records)
{
    check_datetime();
    if (!is_open()) {
        return;
    }
    std::for_each(records.begin(), records.end(), [&](const std::string& line) {
        _fp << line << "\r\n";
    });
}

void audit_archiver::check_datetime()
{
    const unsigned long cur_dt_flag = get_current_dtflag();
    if (_dtflag != cur_dt_flag) {
        _dtflag = cur_dt_flag;
        close();
        open(_dir);
    }
}

unsigned long audit_archiver::get_current_dtflag() const noexcept
{
    unsigned long dtflag = 0;
    SYSTEMTIME st = { 0 };

    GetLocalTime(&st);
    dtflag = st.wYear;
    dtflag <<= 16;
    dtflag |= ((st.wMonth << 8) & 0xFF00);
    dtflag |= (st.wDay & 0x00FF);

    return dtflag;
}

std::wstring audit_archiver::dtflag_to_filename(unsigned long dtflag) const noexcept
{
    std::wstring    filename;
    int nYear = (int)(dtflag >> 16);
    int nMon = (int)((dtflag >> 8) & 0x000000FF);
    int nDay = (int)(dtflag & 0x000000FF);
    swprintf_s(nudf::string::tempstr<wchar_t>(filename, MAX_PATH), MAX_PATH, L"%04d-%02d-%02d.log", nYear, nMon, nDay);
    return std::move(filename);
}

//
//  class audit_uploader
//
audit_uploader::audit_uploader()
{
}

audit_uploader::~audit_uploader()
{
}

bool audit_uploader::upload(const std::vector<std::string>& records) noexcept
{
    bool result = true;
    std::string xml;

    std::string agent_id = NX::utility::conversions::utf16_to_utf8(GLOBAL.agent_info().id());
    std::string tenant_id = NX::utility::conversions::utf16_to_utf8(GLOBAL.register_info().tenant_id());

    xml = "<logService tenantId=\"" + tenant_id + "\" agentId=\"" + agent_id + "\" version=\"5\">";
    xml += "<logRequest>";
    xml += "<logs>";
    std::for_each(records.begin(), records.end(), [&](const std::string& record) {
        xml += record;
    });
    xml += "</logs></logRequest></logService>";
    result = GLOBAL.rest().send_audit_log(xml);

    return result;
}


namespace {
class lock_guard
{
public:
    lock_guard() : _p(NULL) {}
    lock_guard(CRITICAL_SECTION* p) : _p(p)
    {
        if (NULL != _p) {
            ::EnterCriticalSection(_p);
        }
    }
    ~lock_guard()
    {
        if (NULL != _p) {
            ::LeaveCriticalSection(_p);
        }
    }

private:
    CRITICAL_SECTION* _p;
};
}

//
//  class serv_audit
//
serv_audit::serv_audit() : _active(false)
{
    ::InitializeCriticalSection(&_cache_lock);
}

serv_audit::~serv_audit()
{
    stop();
    ::DeleteCriticalSection(&_cache_lock);
}

void serv_audit::clear() noexcept
{
}

void serv_audit::start(const std::wstring& work_dir)
{
    // check & make work dir
    _workdir = work_dir;
    if (_workdir.empty()) {
        GetModuleFileNameW(NULL, nudf::string::tempstr<wchar_t>(_workdir, MAX_PATH), MAX_PATH);
        auto pos = _workdir.find_last_of(L'\\');
        if (pos == std::wstring::npos) {
            GetCurrentDirectoryW(MAX_PATH, nudf::string::tempstr<wchar_t>(_workdir, MAX_PATH));
        }
        else {
            _workdir = _workdir.substr(0, pos);
        }
    }
    else {
        if (boost::algorithm::ends_with(_workdir, L"\\")) {
            _workdir = _workdir.substr(0, _workdir.length()-1);
        }
    }

    _cache_file = _workdir + L"\\audit.cache";

    // create files
    if (!_archiver.open(work_dir)) {
        throw std::exception("fail to open current history log");
    }

    if (!_writer.open(_cache_file, false)) {
        throw std::exception("writer fail to open cache file");
    }

    // done
    _active = true;
}

void serv_audit::stop()
{
    _active = false;
    lock_guard lock(&_cache_lock);
    _archiver.close();
    _writer.close();
}

void serv_audit::audit(const NX::EVAL::eval_object& eo)
{
    if (!_writer.is_open()) {
        return;
    }

    assert(eo.result() != NULL);
    
    try {

        std::wstring log_element;

        std::wstring audit_id = generate_id();
        std::wstring audit_time = generate_timestamp();
        std::wstring audit_rights = eo.result()->final_rights().serialize();

        std::wstring user_name;
        std::wstring user_id;
        std::wstring user_context = eo.user_context();
        std::vector<std::pair<std::wstring, std::wstring>> user_attrs;
        std::wstring app_name;
        std::wstring app_publisher;
        std::vector<std::pair<std::wstring, std::wstring>> app_attrs;
        std::wstring host_name;
        std::wstring host_ipv4;
        std::vector<std::pair<std::wstring, std::wstring>> host_attrs;
        std::wstring fso_name;
        std::vector<std::pair<std::wstring, std::wstring>> res_attrs;
        std::wstring env_heartbeat;
        std::wstring env_current_time;
        std::vector<std::pair<std::wstring, std::wstring>> env_attrs;

        std::vector<std::pair<std::wstring, std::wstring>> policies;

        std::for_each(eo.attributes().cbegin(), eo.attributes().cend(), [&](const NX::EVAL::attribute_multimap::value_type& it) {
            if (0 == _wcsicmp(it.first.c_str(), L"user.name")) {
                user_name = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"user.id")) {
                user_id = it.second.serialize();
                std::transform(user_id.begin(), user_id.end(), user_id.begin(), toupper);
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"application.path")) {
                app_name = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"application.publisher")) {
                app_publisher = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"host.name")) {
                host_name = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"host.inet_addr")) {
                host_ipv4 = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"resource.fso.path")) {
                fso_name = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"environment.time_since_last_heartbeat")) {
                env_heartbeat = it.second.serialize();
            }
            else if (0 == _wcsicmp(it.first.c_str(), L"environment.current_time.identity")) {
                env_current_time = it.second.serialize();
            }
            else if (boost::algorithm::istarts_with(it.first, L"user.")) {
                user_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(5), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"application.")) {
                app_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(12), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"host.")) {
                host_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(5), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"resource.fso.")) {
                res_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(13), it.second.serialize()));
            }
            else if (boost::algorithm::istarts_with(it.first, L"environment.")) {
                env_attrs.push_back(std::pair<std::wstring, std::wstring>(it.first.substr(12), it.second.serialize()));
            }
            else {
                ; // ignore unknown type
            }
        });


        log_element = L"<log uid=\"" + audit_id + L"\" timestamp=\"" + audit_time + L"\" type=\"Evaluation\">";
        log_element += L"<Rights>" + (audit_rights.empty() ? L"None" : audit_rights) + L"</Rights>";
        log_element += L"<Operation>Open</Operation>";
        log_element += L"<Environment>";
        log_element += L"<SecondsSinceLastHeartbeat>" + env_heartbeat + L"</SecondsSinceLastHeartbeat>";
        log_element += L"<SessionType>Console</SessionType>";
        //log_element += L"<current_time.identity>" + env_current_time + L"</current_time.identity>";
        //std::for_each(env_attrs.begin(), env_attrs.end(), [&](const std::pair<std::wstring, std::wstring>& it) {
        //    log_element += L"<" + it.first + L">" + it.second + L"</" + it.first + L">";
        //});
        log_element += L"</Environment>";
        log_element += L"<User>";
        log_element += L"<Name>" + user_name + L"</Name>";
        log_element += L"<Sid>" + user_id + L"</Sid>";
        log_element += L"<Context>" + user_context + L"</Context>";
        if (!user_attrs.empty()) {
            log_element += L"<Attributes>";
            std::for_each(user_attrs.begin(), user_attrs.end(), [&](const std::pair<std::wstring, std::wstring>& it) {
                log_element += L"<" + it.first + L">" + it.second + L"</" + it.first + L">";
            });
            log_element += L"</Attributes>";
        }
        else {
            log_element += L"<Attributes />";
        }
        log_element += L"</User>";
        log_element += L"<Host>";
        log_element += L"<Name>" + host_name + L"</Name>";
        log_element += L"<Ipv4>" + host_ipv4 + L"</Ipv4>";
        log_element += L"</Host>";
        log_element += L"<Application>";
        log_element += L"<Image>" + app_name + L"</Image>";
        log_element += L"<Publisher>" + app_publisher + L"</Publisher>";
        log_element += L"</Application>";
        log_element += L"<Resource>";
        log_element += L"<Path>" + fso_name + L"</Path>";
        log_element += L"<Tags>";
        std::for_each(res_attrs.begin(), res_attrs.end(), [&](const std::pair<std::wstring, std::wstring>& it) {
            log_element += L"<Tag Name=\"" + it.first + L"\" Value=\"" + it.second + L"\" />";
        });
        log_element += L"</Tags>";
        log_element += L"</Resource>";
        log_element += L"<HitPolicies>";
        std::for_each(eo.result()->hit_policies().begin(), eo.result()->hit_policies().end(), [&](const std::pair<std::wstring, std::shared_ptr<NX::EVAL::policy>>& it) {
            log_element += L"<Policy Id=\"" + it.first + L"\" Name=\"" + it.second->name() + L"\" />";
        });
        log_element += L"</HitPolicies>";
        log_element += L"</log>";


        // write to cache
        std::string s = NX::utility::conversions::utf16_to_utf8(log_element);
        lock_guard lock(&_cache_lock);
        _writer.write(s);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

void serv_audit::upload()
{
    if (!_writer.is_open()) {
        return;
    }

    if (!GLOBAL.connected() || GLOBAL.agent_info().empty()) {
        return;
    }
    
    try {


        std::vector<std::string> records;
        get_upload_records(records);
        if (!records.empty()) {
            if (_uploader.upload(records)) {
                // append to saver
                _archiver.save(records);
            }
            else {
                // insert back to writer
                lock_guard lock(&_cache_lock);
                std::for_each(records.begin(), records.end(), [&](const std::string& line) {
                    _writer.write(line);
                });
            }
        }

    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

namespace {
class seconds_to_20150101
{
public:
    seconds_to_20150101()
    {
        SYSTEMTIME st20150101;
        memset(&st20150101, 0, sizeof(st20150101));
        st20150101.wYear = 2015;
        st20150101.wMonth = 1;
        st20150101.wDay = 1;
        nudf::time::CTime tm20100101(&st20150101);
        nudf::time::CTime tmnow;
        tmnow.Now();
        _seconds = (unsigned long)(tmnow.ToSecondsSince1970Jan1st() - tm20100101.ToSecondsSince1970Jan1st());
    }
    ~seconds_to_20150101()
    {
    }
    inline operator unsigned long() const noexcept { return _seconds; }
    inline unsigned long seconds() const noexcept { return _seconds; }
    inline seconds_to_20150101& operator = (const seconds_to_20150101& other)
    {
        if (this != &other) {
            _seconds = other.seconds();
        }
        return *this;
    }

private:
    unsigned long _seconds;
};
class unique_logid_generator
{
public:
    unique_logid_generator() : _interval({ 0, 0 })
    {
        ::InitializeCriticalSection(&_lock);
        seconds_to_20150101 _base;
        _interval.QuadPart = _base.seconds();
    }
    ~unique_logid_generator()
    {
        ::DeleteCriticalSection(&_lock);
    }

    unsigned long generate()
    {
        unsigned long data = 0;
        ::EnterCriticalSection(&_lock);
        _interval.QuadPart++;
        data = _interval.LowPart;
        ::LeaveCriticalSection(&_lock);
        return data;
    }

private:
    CRITICAL_SECTION _lock;
    LARGE_INTEGER    _interval;
};
}
std::wstring serv_audit::generate_id() const noexcept
{
    static unique_logid_generator s_gen;
    static const unsigned long agent_id = _wtoi(GLOBAL.agent_info().id().c_str());
    __int64 audit_id = agent_id;
    audit_id <<= 32;
    audit_id += s_gen.generate();
    std::wstring sid;
    swprintf_s(nudf::string::tempstr<wchar_t>(sid, 128), 128, L"%I64d", audit_id);
    return std::move(sid);
}

std::wstring serv_audit::generate_timestamp() const noexcept
{
    SYSTEMTIME st = { 0 };
    std::wstring stm;
    GetLocalTime(&st);
    std::wstring szone;
    nudf::time::CTimeZone zone;
    zone.BiasToString(szone);
    swprintf_s(nudf::string::tempstr<wchar_t>(stm, 128), 128, L"%04d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    stm += szone;
    return std::move(stm);
}

void serv_audit::get_upload_records(std::vector<std::string>& records)
{
    records.clear();
    lock_guard lock(&_cache_lock);
    _writer.get_lines(records);
    _writer.reset();
}
