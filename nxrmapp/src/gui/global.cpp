

#include <Windows.h>


#include <string>
#include <memory>

#include <gui\global.hpp>


using namespace NXGUI;
using namespace NXGUI::global;


namespace NXGUI {
namespace global {

namespace {
class global_state
{
public:
    enum module_type {application=0, dynamic_library};
    global_state();
    global_state(module_type type, HINSTANCE instance, HWND main_wnd);
    ~global_state();

    void clear();

    inline HINSTANCE instance() const noexcept { return _instance; }
    inline HWND main_window() const noexcept { return _mainwnd; }

private:
    module_type _module_type;
    HINSTANCE   _instance;
    HWND        _mainwnd;
};

global_state::global_state() : _module_type(application), _instance((HINSTANCE)GetModuleHandleW(NULL)), _mainwnd(NULL)
{
}
global_state::global_state(module_type type, HINSTANCE instance, HWND main_wnd) : _module_type(type), _instance(instance), _mainwnd(main_wnd)
{
}
global_state::~global_state()
{
    clear();
}
void global_state::clear()
{
}
}

static std::shared_ptr<global_state>   GLOBAL;


//
//  export functions
//
bool __stdcall initialize(HINSTANCE instance, HWND main_wnd, bool is_dll) noexcept
{
    bool result = false;

    if (is_dll) {
        if (NULL == instance) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }
    }
    else {
        if (NULL == instance) {
            instance = (HINSTANCE)GetModuleHandleW(NULL);
        }
    }
    GLOBAL = std::shared_ptr<global_state>(new global_state(is_dll ? global_state::dynamic_library : global_state::application, instance, main_wnd));
    if (GLOBAL == NULL) {
        return false;
    }

    return true;
}

void __stdcall check_initialization()
{
    if (GLOBAL == NULL) {
        throw std::exception("NXGUI not initialized");
    }
}



}   // NXGUI::global
}   // NXGUI
