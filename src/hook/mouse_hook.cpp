#include "hook/mouse_hook.hpp"
#include "hook/keyboard_hook.hpp"

namespace minglish {

MouseHookManager::MouseHookManager() = default;

MouseHookManager::~MouseHookManager() {
    Uninstall();
}

MouseHookManager& MouseHookManager::Instance() {
    static MouseHookManager instance;
    return instance;
}

bool MouseHookManager::Install(HINSTANCE hInstance) {
    if (m_hook != nullptr) {
        return true;
    }
    if (!hInstance) {
        hInstance = GetModuleHandleW(nullptr);
    }
    m_hook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
    return m_hook != nullptr;
}

void MouseHookManager::Uninstall() {
    if (m_hook != nullptr) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
}

bool MouseHookManager::IsInstalled() const {
    return m_hook != nullptr;
}

bool MouseHookManager::ProcessMouseEvent(UINT msg) {
    if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN ||
        msg == WM_NCLBUTTONDOWN || msg == WM_NCRBUTTONDOWN || msg == WM_NCMBUTTONDOWN) {
        // User clicked elsewhere: immediately clear syllable buffer without backspacing
        KeyboardHookManager::Instance().GetEngine().Reset();
        return true;
    }
    return false;
}

bool MouseHookManager::CheckForegroundWindow(HWND currentForeground) {
    if (m_lastForeground == nullptr) {
        m_lastForeground = currentForeground;
        return false;
    }

    if (currentForeground != m_lastForeground) {
        m_lastForeground = currentForeground;
        KeyboardHookManager::Instance().GetEngine().Reset();
        return true;
    }

    return false;
}

LRESULT CALLBACK MouseHookManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        UINT msg = static_cast<UINT>(wParam);
        Instance().ProcessMouseEvent(msg);
    }
    // Always pass mouse events through so normal clicking in applications is unaffected
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace minglish
