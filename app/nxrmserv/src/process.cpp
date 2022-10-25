

#include <windows.h>

#include <map>
#include <memory>

#include <boost/algorithm/string.hpp>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\filesys.hpp>

#include "process.hpp"


namespace {


class pe_cache
{
public:
    pe_cache()
    {
        ::InitializeCriticalSection(&_lock);
    }
    
    ~pe_cache()
    {
        ::DeleteCriticalSection(&_lock);
    }

    std::shared_ptr<NX::win::pe_file> find(const std::wstring& image_path)
    {
        std::shared_ptr<NX::win::pe_file> info_ptr;
        std::wstring normalized_image_path(image_path);
        std::transform(normalized_image_path.begin(), normalized_image_path.end(), normalized_image_path.begin(), tolower);

        ::EnterCriticalSection(&_lock);
        auto pos = _map.find(normalized_image_path);
        if (pos != _map.end()) {
            info_ptr = (*pos).second;
        }
        ::LeaveCriticalSection(&_lock);

        return info_ptr;
    }

    void insert(const std::wstring& image_path, const NX::win::pe_file& info)
    {
        std::wstring normalized_image_path(image_path);
        std::transform(normalized_image_path.begin(), normalized_image_path.end(), normalized_image_path.begin(), tolower);
        std::shared_ptr<NX::win::pe_file> info_ptr(new NX::win::pe_file(info));
        if (info_ptr != nullptr) {
            ::EnterCriticalSection(&_lock);
            _map[normalized_image_path] = info_ptr;
            ::LeaveCriticalSection(&_lock);
        }
    }

    void insert(const std::wstring& image_path, const std::shared_ptr<NX::win::pe_file>& info_ptr)
    {
        std::wstring normalized_image_path(image_path);
        std::transform(normalized_image_path.begin(), normalized_image_path.end(), normalized_image_path.begin(), tolower);
        if (info_ptr != nullptr) {
            ::EnterCriticalSection(&_lock);
            _map[normalized_image_path] = info_ptr;
            ::LeaveCriticalSection(&_lock);
        }
    }

    void remove(const std::wstring& image_path)
    {
        std::wstring normalized_image_path(image_path);
        std::transform(normalized_image_path.begin(), normalized_image_path.end(), normalized_image_path.begin(), tolower);
        ::EnterCriticalSection(&_lock);
        _map.erase(normalized_image_path);
        ::LeaveCriticalSection(&_lock);
    }

private:
    std::map<std::wstring, std::shared_ptr<NX::win::pe_file>>  _map;
    CRITICAL_SECTION    _lock;
};

}

static pe_cache PE_CACHE;


process_forbidden_rights::process_forbidden_rights()
{
    memset(&_rights_counter, 0, sizeof(_rights_counter));
}

process_forbidden_rights::~process_forbidden_rights()
{
}

void process_forbidden_rights::disable_right(RIGHT_ID id)
{
    increase_forbidden_count(id);
}

void process_forbidden_rights::enable_right(RIGHT_ID id, bool force)
{
    if (id < RIGHT_MAX) {
        if (force) {
            _rights_counter[id] = 0;
        }
        else {
            decrease_forbidden_count(id);
        }
    }
}

void process_forbidden_rights::increase_forbidden_count(RIGHT_ID id)
{
    if (id < RIGHT_MAX) {
        if (0xFFFF == _rights_counter[id]) {
            _rights_counter[id] = 1;
        }
        else {
            _rights_counter[id] = _rights_counter[id] + 1;
        }
    }
}

void process_forbidden_rights::decrease_forbidden_count(RIGHT_ID id)
{
    if (id < RIGHT_MAX && 0 != _rights_counter[id]) {
        _rights_counter[id] = _rights_counter[id] - 1;
    }
}

bool process_forbidden_rights::is_right_forbidden(RIGHT_ID id) const
{
    bool result = true;
    if (id < RIGHT_MAX) {
        if (0 == _rights_counter[id]) {
            result = false;
        }
    }
    return result;
}

void process_forbidden_rights::clear()
{
    memset(&_rights_counter, 0, sizeof(_rights_counter));
}

process_forbidden_rights& process_forbidden_rights::operator = (const process_forbidden_rights& other)
{
    if (this != &other) {
        memcpy(_rights_counter, other.get_rights_counter(), sizeof(_rights_counter));
    }
    return *this;
}


process_record::process_record() : _process_id(0), _session_id(0xFFFFFFFF), _flags(0)
{
}

process_record::process_record(unsigned long process_id, unsigned __int64 flags) : _session_id(process_record::get_session_id_from_pid(process_id)),
    _process_id((0xFFFFFFFF == _session_id) ? 0 : process_id),
    _flags((0 == _process_id) ? 0 : _flags),
    _image_path((0 == _process_id) ? std::wstring() : process_record::get_image_path_from_pid(_process_id))
{
    std::shared_ptr<NX::win::pe_file> existing_info_ptr = PE_CACHE.find(_image_path);
    if (existing_info_ptr == nullptr) {
        _pe_file_info = std::shared_ptr<NX::win::pe_file>(new NX::win::pe_file(_image_path));
        if (!_pe_file_info->empty()) {
            PE_CACHE.insert(_image_path, _pe_file_info);
        }
    }
    else {
        _pe_file_info = existing_info_ptr;
    }
}

process_record::process_record(unsigned long process_id, unsigned long session_id, const std::wstring& image_path, unsigned __int64 flags) : _session_id(session_id),
    _process_id(process_id),
    _flags((0 == _process_id) ? 0 : _flags),
    _image_path(process_record::normalize_image_path(image_path))
{
    std::shared_ptr<NX::win::pe_file> existing_info_ptr = PE_CACHE.find(_image_path);
    if (existing_info_ptr == nullptr) {
        _pe_file_info = std::shared_ptr<NX::win::pe_file>(new NX::win::pe_file(_image_path));
        if (!_pe_file_info->empty()) {
            PE_CACHE.insert(_image_path, _pe_file_info);
        }
    }
    else {
        _pe_file_info = existing_info_ptr;
    }
}

process_record::~process_record()
{
    clear();
}

unsigned long process_record::get_session_id_from_pid(unsigned long process_id)
{
    unsigned long session_id = 0xFFFFFFFF;
    if (!ProcessIdToSessionId(process_id, &session_id)) {
        session_id = 0xFFFFFFFF;
    }
    return session_id;
}

std::wstring process_record::normalize_image_path(const std::wstring& image_path)
{
    NX::fs::dos_filepath result(image_path);
    return result.path();
}

std::wstring process_record::get_image_path_from_pid(unsigned long process_id)
{
    std::wstring image_path;

    if (0 == process_id) {
        GetModuleFileNameW(NULL, NX::string_buffer<wchar_t>(image_path, MAX_PATH), MAX_PATH);
    }
    else {
        HANDLE h = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
        if (NULL != h) {
            GetModuleFileNameW(NULL, NX::string_buffer<wchar_t>(image_path, MAX_PATH), MAX_PATH);
            CloseHandle(h);
        }
    }

    std::transform(image_path.begin(), image_path.end(), image_path.begin(), tolower);
    return std::move(image_path);
}

process_record& process_record::operator = (const process_record& other)
{
    if (this != &other) {
        _process_id = other.get_process_id();
        _session_id = other.get_session_id();
        _flags = other.get_flags();
        _image_path = other.get_image_path();
        _pe_file_info = other.get_pe_file_info();

        _forbidden_rights = other.get_forbidden_rights();
        _protected_windows = other.get_protected_windows();
    }
    return *this;
}

void process_record::clear()
{
    _process_id = 0;
    _session_id = 0xFFFFFFFF;
    _flags = 0;
    _image_path.clear();
    _pe_file_info.reset();
    _forbidden_rights.clear();
    _protected_windows.clear();
}


process_cache::process_cache()
{
    ::InitializeCriticalSection(&_lock);
}

process_cache::~process_cache()
{
    clear();
    ::DeleteCriticalSection(&_lock);
}

bool process_cache::empty() const
{
    bool result = false;
    ::EnterCriticalSection(&_lock);
    result = _map.empty();
    ::LeaveCriticalSection(&_lock);
    return result;
}

void process_cache::clear()
{
    ::EnterCriticalSection(&_lock);
    _map.clear();
    ::LeaveCriticalSection(&_lock);
}

process_record process_cache::find(unsigned long process_id)
{
    process_record record;
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        record = (*pos).second;
    }
    ::LeaveCriticalSection(&_lock);
    return record;
}

void process_cache::insert(const process_record& record)
{
    ::EnterCriticalSection(&_lock);
    _map[record.get_process_id()] = record;
    ::LeaveCriticalSection(&_lock);
}

void process_cache::remove(unsigned long process_id)
{
    ::EnterCriticalSection(&_lock);
    _map.erase(process_id);
    ::LeaveCriticalSection(&_lock);
}

void process_cache::reset_process_flags(unsigned long process_id, unsigned __int64 f)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.reset_flags(f);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::set_process_flags(unsigned long process_id, unsigned __int64 f)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.set_flags(f);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::clear_process_flags(unsigned long process_id, unsigned __int64 f)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.clear_flags(f);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::forbid_process_right(unsigned long process_id, RIGHT_ID id)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.get_forbidden_rights().disable_right(id);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::enable_process_right(unsigned long process_id, RIGHT_ID id, bool force)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.get_forbidden_rights().enable_right(id, force);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::insert_protected_window(unsigned long process_id, unsigned long hwnd, PROTECT_MODE mode)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.get_protected_windows()[hwnd] = process_protected_window(hwnd, mode);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::change_protected_window_mode(unsigned long process_id, unsigned long hwnd, PROTECT_MODE mode)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.get_protected_windows()[hwnd].change_mode(mode);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::remove_protected_window(unsigned long process_id, unsigned long hwnd)
{
    ::EnterCriticalSection(&_lock);
    auto pos = _map.find(process_id);
    if (pos != _map.end()) {
        (*pos).second.get_protected_windows().erase(hwnd);
    }
    ::LeaveCriticalSection(&_lock);
}

void process_cache::clear_protected_windows(unsigned long process_id)
{
    ::EnterCriticalSection(&_lock);
    if (0 == process_id) {
        std::for_each(_map.begin(), _map.end(), [](std::pair<unsigned long, process_record> item) {
            item.second.get_protected_windows().clear();
        });
    }
    else {
        auto pos = _map.find(process_id);
        if (pos != _map.end()) {
            (*pos).second.get_protected_windows().clear();
        }
    }
    ::LeaveCriticalSection(&_lock);
}

std::vector<process_protected_window> process_cache::get_protected_windows(unsigned long process_id)
{
    std::vector<process_protected_window> protected_windows;

    ::EnterCriticalSection(&_lock);
    if (0 == process_id) {
        std::for_each(_map.begin(), _map.end(), [&](const std::pair<unsigned long, process_record>& item) {
            std::for_each(item.second.get_protected_windows().begin(), item.second.get_protected_windows().end(), [&](const std::pair<unsigned long, process_protected_window>& windows_item) {
                protected_windows.push_back(windows_item.second);
            });
        });
    }
    else {
        auto pos = _map.find(process_id);
        if (pos != _map.end()) {
            std::for_each((*pos).second.get_protected_windows().begin(), (*pos).second.get_protected_windows().end(), [&](const std::pair<unsigned long, process_protected_window>& windows_item) {
                protected_windows.push_back(windows_item.second);
            });
        }
    }
    ::LeaveCriticalSection(&_lock);

    return protected_windows;
}
