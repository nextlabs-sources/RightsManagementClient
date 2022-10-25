

#include <Windows.h>

#include <iostream>

#include "gui\winbase.hpp"



int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    NXGUI::window wnd;
    RECT rect{ 0, 0, 100, 200 };
    MSG msg;

    if (!NXGUI::window::register_wnd_class(L"test_app", 0)) {
        std::cout << "fail to register window class (" << (int)GetLastError() << L")" << std::endl;
    }

    if (!wnd.create2(0, L"test_app", L"Test", 0, 200, 200, 100, 200, NULL, NULL, NULL)) {
        std::cout << "fail to create window (" << (int)GetLastError() << L")" << std::endl;
    }

    ShowWindow(wnd, nCmdShow);
    UpdateWindow(wnd);

    // Step 3: The Message Loop
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

