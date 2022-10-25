

#include <Windows.h>

#include <algorithm>

#include <nudf\shared\rightsdef.h>

#include "nxrmflt.h"
#include "process_status.hpp"


using namespace NX;



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



dwm_window::dwm_window() : _h(0), _status(WndStatusUnknown)
{
}

dwm_window::dwm_window(unsigned long h, WNDSTATUS init_status) : _h(h), _status(init_status)
{
}

dwm_window::~dwm_window()
{
}

dwm_window& dwm_window::operator = (const dwm_window& other)
{
    if (this != &other) {
        _h = other.get_hwnd();
        _status = other.get_status();
    }
    return *this;
}

process_status::process_status() : _id(0), _session_id(-1), _flags(0)
{
    ::InitializeCriticalSection(&_lock);
}

process_status::process_status(unsigned long id, unsigned long session_id, unsigned __int64 flags, const std::wstring& image_path, const std::wstring& image_publisher)
    : _id(id), _session_id(session_id), _flags(flags), _publisher(image_publisher), _image(image_path)
{
    ::InitializeCriticalSection(&_lock);
}

process_status::~process_status()
{
    ::DeleteCriticalSection(&_lock);
}

void process_status::add_wnd(unsigned long h, dwm_window::WNDSTATUS init_status)
{
    ::EnterCriticalSection(&_lock);
    auto pos = std::find_if(_wnds.begin(), _wnds.end(), [h](const dwm_window& it) -> bool {return (it.get_hwnd() == h); });
    if (pos == _wnds.end()) {
        _wnds.push_back(dwm_window(h, init_status));
    }
    else {
		if ((int)init_status > ((int)((*pos).get_status()))) {
			(*pos).set_status(init_status);
		}

    }
    ::LeaveCriticalSection(&_lock);
}

void process_status::remove_wnd(unsigned long h)
{
    ::EnterCriticalSection(&_lock);
    auto pos = std::find_if(_wnds.begin(), _wnds.end(), [h](const dwm_window& it) -> bool {return (it.get_hwnd() == h); });
    if (pos != _wnds.end()) {
        _wnds.erase(pos);
        if (is_flag_on(NXRM_PROCESS_FLAG_HAS_OVERLAY_INTEGRATION)) {
            pos = std::find_if(_wnds.begin(), _wnds.end(), [h](const dwm_window& it) -> bool {return it.status_true(); });
            if (pos == _wnds.end()) {
                // this is tightly integrated app, and there is no overlay == true window any more
                //  ==> remove NXRM_PROCESS_FLAG_WITH_NXL_OPENED
                remove_flags(NXRM_PROCESS_FLAG_WITH_NXL_OPENED);
            }
        }
        else {
            pos = std::find_if(_wnds.begin(), _wnds.end(), [h](const dwm_window& it) -> bool {return (it.status_true() || it.status_unknown()); });
            if (pos == _wnds.end()) {
                // this is NOT tightly integrated app, and there is no (status == true or status == unknown) window any more
                //  ==> remove NXRM_PROCESS_FLAG_WITH_NXL_OPENED
                remove_flags(NXRM_PROCESS_FLAG_WITH_NXL_OPENED);
            }
        }
    }
    ::LeaveCriticalSection(&_lock);
}

void process_status::set_wnd_status(unsigned long h, dwm_window::WNDSTATUS s)
{
    ::EnterCriticalSection(&_lock);
    auto pos = std::find_if(_wnds.begin(), _wnds.end(), [h](const dwm_window& it) -> bool {return (it.get_hwnd() == h); });
    if (pos != _wnds.end()) {
        (*pos).set_status(s);
    }
    ::LeaveCriticalSection(&_lock);
}

process_status& process_status::operator =(const process_status& other)
{
    if (this != &other) {
        _id = other.get_id();
        _session_id = other.get_session_id();
        _flags = other.get_flags();
        _image = other.get_image();
        _publisher = other.get_publisher();
        _wnds = other.get_wnds();
    }
    return *this;
}

void process_status::clear()
{
    _id = 0;
    _session_id = -1;
    _flags = 0;
    _image.clear();
    _wnds.clear();
}

void process_status::set_process_rights(unsigned __int64 granted_rights)
{
    // Never forbit VIEW
    ::EnterCriticalSection(&_lock);
    if (0 == (granted_rights & BUILTIN_RIGHT_EDIT)) {
        _forbidden_rights.disable_right(RIGHT_EDIT);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_PRINT)) {
        _forbidden_rights.disable_right(RIGHT_PRINT);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_CLIPBOARD)) {
        _forbidden_rights.disable_right(RIGHT_CLIPBOARD);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_SAVEAS)) {
        _forbidden_rights.disable_right(RIGHT_SAVEAS);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_DECRYPT)) {
        _forbidden_rights.disable_right(RIGHT_DECRYPT);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_SCREENCAP)) {
        _forbidden_rights.disable_right(RIGHT_SCREENCAP);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_SEND)) {
        _forbidden_rights.disable_right(RIGHT_SEND);
    }
    if (0 == (granted_rights & BUILTIN_RIGHT_CLASSIFY)) {
        _forbidden_rights.disable_right(RIGHT_CLASSIFY);
    }
    ::LeaveCriticalSection(&_lock);
}

unsigned __int64 process_status::get_process_rights()
{
    // Always allow view
    unsigned __int64 result = BUILTIN_RIGHT_VIEW;

    ::EnterCriticalSection(&_lock);
    if (!_forbidden_rights.is_right_forbidden(RIGHT_EDIT)) {
        result |= BUILTIN_RIGHT_EDIT;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_PRINT)) {
        result |= BUILTIN_RIGHT_PRINT;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_CLIPBOARD)) {
        result |= BUILTIN_RIGHT_CLIPBOARD;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_SAVEAS)) {
        result |= BUILTIN_RIGHT_SAVEAS;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_DECRYPT)) {
        result |= BUILTIN_RIGHT_DECRYPT;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_SCREENCAP)) {
        result |= BUILTIN_RIGHT_SCREENCAP;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_SEND)) {
        result |= BUILTIN_RIGHT_SEND;
    }
    if (!_forbidden_rights.is_right_forbidden(RIGHT_CLASSIFY)) {
        result |= BUILTIN_RIGHT_CLASSIFY;
    }
    ::LeaveCriticalSection(&_lock);

    return result;
}


process_map::process_map()
{
    ::InitializeCriticalSection(&_lock);
}

process_map::~process_map()
{
    ::DeleteCriticalSection(&_lock);
}

void process_map::add_process(unsigned long process_id, unsigned long session_id, unsigned __int64 flags, const std::wstring& image_path, const std::wstring& image_publisher)
{
    ::EnterCriticalSection(&_lock);
    _map[process_id] = std::shared_ptr<process_status>(new process_status(process_id, session_id, flags, image_path, image_publisher));
    ::LeaveCriticalSection(&_lock);
}

void process_map::remove_process(unsigned long process_id)
{
    ::EnterCriticalSection(&_lock);
    auto pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<process_status>>& it) -> bool {return (it.first == process_id); });
    if (pos != _map.end()) {
        _map.erase(pos);
    }
    ::LeaveCriticalSection(&_lock);
}

std::shared_ptr<process_status> process_map::get_process(unsigned long process_id) const
{
    std::shared_ptr<process_status> sp;
    ::EnterCriticalSection(&_lock);
    auto pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<process_status>>& it) -> bool {return (it.first == process_id); });
    if (pos != _map.end()) {
        sp = (*pos).second;
    }
    ::LeaveCriticalSection(&_lock);
    return sp;
}

bool process_map::process_has_overlay_window(unsigned long process_id) const
{
    bool result = false;

    ::EnterCriticalSection(&_lock);
    auto pos = std::find_if(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<process_status>>& it) -> bool {return (it.first == process_id); });
    if (pos != _map.end()) {
        if ((*pos).second != nullptr) {
            const std::vector<dwm_window>& wnds = (*pos).second->get_wnds();
            const bool is_process_with_nxl_opened = (*pos).second->is_flag_on(NXRM_PROCESS_FLAG_WITH_NXL_OPENED);
			const bool is_process_without_overlypolicy = (*pos).second->is_flag_on(NXRM_PROCESS_FLAG_HAS_NO_OVERLAYPOLICY);
            result = (!wnds.empty() && is_process_with_nxl_opened && !is_process_without_overlypolicy);
        }
    }
    ::LeaveCriticalSection(&_lock);

    return result;
}

__forceinline unsigned long process_id_to_session_id(unsigned long process_id)
{
    unsigned long session_id = -1;
    if (!ProcessIdToSessionId(process_id, &session_id)) {
        session_id = -1;
    }
    return session_id;
}

std::vector<unsigned long> process_map::get_valid_overlay_windows(unsigned long session_id) const
{
    std::vector<unsigned long> buf;
    ::EnterCriticalSection(&_lock);
    std::for_each(_map.begin(), _map.end(), [&](const std::pair<unsigned long, std::shared_ptr<process_status>>& it) {
        // Check session first
        //if (it.second->get_session_id() == session_id) {
        if (process_id_to_session_id(it.second->get_id()) == session_id) {
            const std::vector<dwm_window>& wnds = it.second->get_wnds();
            const bool is_process_with_nxl_opened = it.second->is_flag_on(NXRM_PROCESS_FLAG_WITH_NXL_OPENED);
            const bool is_process_with_overlay_integrated = it.second->is_flag_on(NXRM_PROCESS_FLAG_HAS_OVERLAY_INTEGRATION);
            std::for_each(wnds.begin(), wnds.end(), [&](const dwm_window& wnd) {

                if (is_process_with_nxl_opened) {

                    if (is_process_with_overlay_integrated) {
                        if (wnd.status_true()) {
                            buf.push_back(wnd.get_hwnd());
                        }
                    }
                    else {
                        if (wnd.status_true() || wnd.status_unknown()) {
                            buf.push_back(wnd.get_hwnd());
                        }
                    }
                }
            });
        }
    });
    ::LeaveCriticalSection(&_lock);
    return std::move(buf);
}