

#include <Windows.h>


#include <boost/algorithm/string.hpp>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\dbglog.hpp>
#include <nudf\filesys.hpp>


using namespace NX::dbg;
using namespace NX;


log_item::log_item() : _log_level(LL_INFO), _thread_id(0)
{
    memset(&_timestamp, 0, sizeof(_timestamp));
}

log_item::log_item(const log_item& other)
{
    _timestamp = other.get_timestamp();
    _message = other.get_message();
    _log_level = other.get_log_level();
    _thread_id = other.get_thread_id();
}

//log_item::log_item(log_item&& other)
//{
//    std::move(other.get_message());
//    _timestamp = other.get_timestamp();
//    _log_level = other.get_log_level();
//    _thread_id = other.get_thread_id();
//}

log_item::log_item(LOGLEVEL level, const std::wstring& msg) : _log_level(level > LOGLEVEL::LL_ALL ? LOGLEVEL::LL_ALL : level), _message(msg), _thread_id(GetThreadId(NULL))
{
    memset(&_timestamp, 0, sizeof(_timestamp));
    GetLocalTime(&_timestamp);
    while (!_message.empty() && (_message[0] == L'\r' || _message[0] == L'\n')) {
        _message = _message.substr(0, _message.length() - 1);
    }
    while (!_message.empty() && (boost::algorithm::ends_with(_message, L"\r") || boost::algorithm::ends_with(_message, L"\n"))) {
        _message = _message.substr(0, _message.length() - 1);
    }
}

log_item::log_item(LOGLEVEL level, const std::string& msg) : _log_level(level > LOGLEVEL::LL_ALL ? LOGLEVEL::LL_ALL : level), _message(NX::conversion::utf8_to_utf16(msg)), _thread_id(GetThreadId(NULL))
{
    memset(&_timestamp, 0, sizeof(_timestamp));
    GetLocalTime(&_timestamp);
    while (!_message.empty() && (_message[0] == L'\r' || _message[0] == L'\n')) {
        _message = _message.substr(0, _message.length() - 1);
    }
    while (!_message.empty() && (boost::algorithm::ends_with(_message, L"\r") || boost::algorithm::ends_with(_message, L"\n"))) {
        _message = _message.substr(0, _message.length() - 1);
    }
}

log_item::~log_item()
{
}

std::wstring log_item::serialize() const
{
    static const wchar_t* LevelName[] = {
        L"CRIT",
        L"ERRO",
        L"WARN",
        L"INFO",
        L"INF1",
        L"INF2",
        L"INF3",
        L"INF4",
        L"INF5",
        L"INF6",
        L"INF7",
        L"INF8",
        L"INF9",
        L"DBUG",
        L"DETL",
        L"UKWN"
    };
    static const std::wstring blank_header(L"                                    ");
    std::wstring s;
    const SYSTEMTIME& st = get_timestamp();
    const int level_id = get_log_level() > LOGLEVEL::LL_ALL ? LOGLEVEL::LL_ALL : get_log_level();

    // Build Header
    // "[2016-03-11T10:38:45.341 B701 ERRO] "  ==> 36 characters
    // "                                    "  ==> 36 characters
    s = NX::string_formater(L"[%04d-%02d-%02dT%02d:%02d:%02d.%03d %04X %s] ",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        get_thread_id(),
        LevelName[level_id]);

    // Break down message (handle '\n')
    std::for_each(_message.begin(), _message.end(), [&](const wchar_t& ch) {        
        switch (ch)
        {
        case 0:
        case L'\r':
            break;
        case L'\n':
            s += blank_header;
            break;
        default:
            s.push_back(ch);
            break;
        }
    });
    return std::move(s);
}

log_item& log_item::operator = (const log_item& other)
{
    if (this != &other) {
        _timestamp = other.get_timestamp();
        _message = other.get_message();
        _log_level = other.get_log_level();
        _thread_id = other.get_thread_id();
    }
    return *this;
}

//log_item& log_item::operator = (log_item&& other)
//{
//    if (this != &other) {
//        std::move(other.get_message());
//        _timestamp = other.get_timestamp();
//        _log_level = other.get_log_level();
//        _thread_id = other.get_thread_id();
//    }
//    return *this;
//}


//
//  class file_log
//

file_log::file_log() : _h(INVALID_HANDLE_VALUE), _accept_level(LL_DEBUG), _queue_limit(512), _size_limit(0), _rotate_count(3)
{
    ::InitializeCriticalSection(&_lock);
    _events[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _events[1] = ::CreateEventW(NULL, TRUE, FALSE, NULL);
}

file_log::~file_log()
{
    close();

    if (NULL != _events[0]) {
        CloseHandle(_events[0]);
        _events[0] = NULL;
    }
    if (NULL != _events[1]) {
        CloseHandle(_events[1]);
        _events[1] = NULL;
    }
    ::DeleteCriticalSection(&_lock);
}

void file_log::create(const std::wstring& file, LOGLEVEL accept_level, unsigned long size_limit, unsigned long rotate_count, unsigned long queue_limit)
{
    if (opened()) {
        throw NX::exception(WIN32_ERROR_MSG(ERROR_SHARING_VIOLATION, "log file has been opened"));
    }

    _h = ::CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
    if (INVALID_HANDLE_VALUE == _h) {
        throw NX::exception(WIN32_ERROR_MSG(GetLastError(), "fail to open log file"));
    }

    if (0 == GetFileSize(_h, NULL)) {
        // new file, write BOM
        write_bom();
    }

    SetFilePointer(_h, 0, NULL, FILE_END);

    _file = file;
    _accept_level = accept_level;
    _size_limit = size_limit;
    _rotate_count = rotate_count;
    _queue_limit = queue_limit;

    try {
        _log_thread = std::thread(file_log::writer, this);
    }
    catch (const std::exception& e) {
        CloseHandle(_h);
        _h = INVALID_HANDLE_VALUE;
        throw NX::exception(NX::string_formater("fail to create log thread: %s", e.what()));
    }
}

void file_log::close()
{
    if (_log_thread.joinable()) {
        SetEvent(_events[1]);
        _log_thread.join();
    }
    if (opened()) {
        CloseHandle(_h);
        _h = INVALID_HANDLE_VALUE;
    }
}

void file_log::push(const log_item& item)
{
    bool pushed = false;
    ::EnterCriticalSection(&_lock);
    if (_queue.size() < get_queue_limit()) {
        _queue.push(item);
        pushed = true;
    }
    ::LeaveCriticalSection(&_lock);
    if (pushed) {
        SetEvent(_events[0]);
    }
}

log_item file_log::pop()
{
    log_item item;
    ::EnterCriticalSection(&_lock);
    if (!_queue.empty()) {
        item = _queue.front();
        _queue.pop();
    }
    ::LeaveCriticalSection(&_lock);
    return item;
}

void file_log::rotate()
{
    NX::fs::dos_filepath path(_file);
    const std::wstring& dir = path.file_dir();
    const std::wstring& name_part = path.file_name().name_part();
    const std::wstring& name_extension = path.file_name().extension();

    if (get_rotate_count() == 0) {
        SetFilePointer(_h, 0, NULL, FILE_BEGIN);
        SetEndOfFile(_h);
        return;
    }
    
    // move other back up file
    for (int i = 0; i < (int)get_rotate_count(); i++) {
        
        std::wstring source_file = dir + L"\\" + name_part + L"_" + std::to_wstring((int)get_rotate_count() - 1 - i) + name_extension;
        std::wstring target_file = (0 == i) ? L"" : (dir + L"\\" + name_part + L"_" + std::to_wstring((int)get_rotate_count() - i) + name_extension);

        if (target_file.empty()) {
            ::DeleteFileW(source_file.c_str());
        }
        else {
            ::MoveFileEx(source_file.c_str(), target_file.c_str(), MOVEFILE_REPLACE_EXISTING);
        }
    }

    // Move current file to #0
    CloseHandle(_h);
    _h = INVALID_HANDLE_VALUE;
    std::wstring file_0 = dir + L"\\" + name_part + L"_0" + name_extension;
    MoveFileEx(_file.c_str(), file_0.c_str(), MOVEFILE_REPLACE_EXISTING);

    // Reopen file
    _h = ::CreateFileW(_file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
    assert(INVALID_HANDLE_VALUE != _h);
    if (INVALID_HANDLE_VALUE != _h) {
        write_bom();
    }
}

void file_log::write(const log_item& item)
{
    unsigned long bytes_written = 0;
    const std::wstring& s = item.serialize();

    write_line_end();
    if (!s.empty()) {
        ::WriteFile(_h, s.c_str(), (unsigned long)(sizeof(wchar_t) * s.length()), &bytes_written, NULL);
    }

    const unsigned long file_size = GetFileSize(_h, NULL);
    if (file_size > _size_limit) {
        rotate();
    }
}

void file_log::write_bom()
{
    static const unsigned short BOM_UTF16 = 0xFEFF;
    unsigned long bytes_written = 0;
    ::WriteFile(_h, &BOM_UTF16, sizeof(BOM_UTF16), &bytes_written, NULL);
}

void file_log::write_line_end()
{
    unsigned long bytes_written = 0;
    ::WriteFile(_h, L"\r\n", 4, &bytes_written, NULL);
}

void file_log::writer(file_log* instance)
{
    bool active = true;

    do {

        unsigned long wait_result = ::WaitForMultipleObjects(2, instance->_events, FALSE, INFINITE);
        switch (wait_result)
        {
        case WAIT_OBJECT_0:
            // new item comes in
            while (true) {
                const log_item& item = instance->pop();
                if (item.empty()) {
                    break;
                }
                instance->write(item);
            }
            break;

        case (WAIT_OBJECT_0 + 1):
            // exit
            active = false;
            break;

        default:
            // error
            active = false;
            break;
        }

    } while (active);
}
