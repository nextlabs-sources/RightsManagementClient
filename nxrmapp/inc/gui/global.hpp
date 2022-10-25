

#ifndef __NXGUI_GLOBAL_HPP__
#define __NXGUI_GLOBAL_HPP__


#include <Windows.h>

namespace NXGUI {
namespace global {


bool __stdcall initialize(HINSTANCE instance, HWND mainwnd, bool is_dll=false) noexcept;

// throw if not initialized
void __stdcall check_initialization();


}   // NXGUI::global
}   // NXGUI




#endif