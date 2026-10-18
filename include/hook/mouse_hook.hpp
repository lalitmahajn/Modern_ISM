#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

class MouseHookManager {
public:
    static MouseHookManager& Instance();

    bool Install(HINSTANCE hInstance = nullptr);
    void Uninstall();
    bool IsInstalled() const;

    // Filter function testable without installing global OS hook
    // Returns true if event was a click that triggered a buffer reset
    bool ProcessMouseEvent(UINT msg);

    // Checks foreground window and resets buffer if window focus changed
    bool CheckForegroundWindow(HWND currentForeground);

private:
    MouseHookManager();
    ~MouseHookManager();

    MouseHookManager(const MouseHookManager&) = delete;
    MouseHookManager& operator=(const MouseHookManager&) = delete;

    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK m_hook = nullptr;
    HWND m_lastForeground = nullptr;
};

} // namespace minglish
