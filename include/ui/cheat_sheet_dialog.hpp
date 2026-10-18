#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

class CheatSheetDialog {
public:
    static void Show(HINSTANCE hInstance, HWND hTargetApp = nullptr);
    static bool IsOpen();
    static void Close();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    HWND m_hTargetApp = nullptr;
    HINSTANCE m_hInstance = nullptr;

    static CheatSheetDialog* s_instance;
};

} // namespace minglish
