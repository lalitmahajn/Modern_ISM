#include "hook/keyboard_hook.hpp"

namespace minglish {

KeyboardHookManager::KeyboardHookManager()
    : m_injector(std::make_unique<Win32SendInputInjector>()) {
}

KeyboardHookManager::~KeyboardHookManager() {
    Uninstall();
}

KeyboardHookManager& KeyboardHookManager::Instance() {
    static KeyboardHookManager instance;
    return instance;
}

bool KeyboardHookManager::Install(HINSTANCE hInstance) {
    if (m_hook != nullptr) {
        return true;
    }
    if (!hInstance) {
        hInstance = GetModuleHandleW(nullptr);
    }
    m_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    return m_hook != nullptr;
}

void KeyboardHookManager::Uninstall() {
    if (m_hook != nullptr) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
}

bool KeyboardHookManager::IsInstalled() const {
    return m_hook != nullptr;
}

void KeyboardHookManager::SetEngineEnabled(bool enabled) {
    m_enabled = enabled;
    if (!m_enabled) {
        m_engine.Reset();
    }
}

bool KeyboardHookManager::IsEngineEnabled() const {
    return m_enabled;
}

void KeyboardHookManager::ToggleEngine() {
    SetEngineEnabled(!m_enabled);
}

void KeyboardHookManager::SetFontProfile(const FontProfile& profile) {
    m_engine.SetFontProfile(profile);
}

const FontProfile& KeyboardHookManager::GetFontProfile() const {
    return m_engine.GetFontProfile();
}

TranspilerEngine& KeyboardHookManager::GetEngine() {
    return m_engine;
}

ITextInjector* KeyboardHookManager::GetInjector() {
    return m_injector.get();
}

void KeyboardHookManager::SetInjector(std::unique_ptr<ITextInjector> injector) {
    m_injector = std::move(injector);
}

bool KeyboardHookManager::ProcessKeyEvent(DWORD vkCode, bool isKeyUp, DWORD flags,
                                         bool ctrlDown, bool altDown, bool winDown,
                                         char asciiChar) {
    if (!m_enabled) {
        return false;
    }

    // Ignore synthetic / injected keystrokes to prevent recursive infinite loops
    if (flags & LLKHF_INJECTED) {
        return false;
    }

    // Pass through key-up release events
    if (isKeyUp) {
        return false;
    }

    // Modifier chords (Ctrl+C, Ctrl+V, Alt+Tab, Win+D, etc.) bypass hook and flush buffer
    if (ctrlDown || altDown || winDown) {
        m_engine.Reset();
        return false;
    }

    // Word boundary and navigation keys reset active buffer and pass through to target app
    if (vkCode == VK_SPACE || vkCode == VK_RETURN || vkCode == VK_TAB ||
        vkCode == VK_ESCAPE || vkCode == VK_LEFT || vkCode == VK_RIGHT ||
        vkCode == VK_UP || vkCode == VK_DOWN || vkCode == VK_HOME ||
        vkCode == VK_END || vkCode == VK_PRIOR || vkCode == VK_NEXT ||
        vkCode == VK_DELETE) {
        m_engine.Reset();
        return false;
    }

    // Physical Backspace handling:
    // If buffer is active, intercept, swallow physical backspace, and re-inject modified syllable.
    // If buffer is empty, pass through so user can delete regular text in their app.
    if (vkCode == VK_BACK) {
        if (m_engine.GetCurrentUnicode().empty()) {
            return false;
        }

        InjectionDiffU16 diffU16 = m_engine.OnBackspaceU16();
        if (m_injector) {
            m_injector->Inject(diffU16);
        }
        return true;
    }

    // Printable character input interception
    if (asciiChar != 0) {
        InjectionDiffU16 diffU16 = m_engine.OnCharU16(asciiChar);
        if (m_injector) {
            m_injector->Inject(diffU16);
        }
        return true;
    }

    return false;
}

LRESULT CALLBACK KeyboardHookManager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    auto* pKbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    if (!pKbd) {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // Loop prevention: bypass injected keys immediately
    if (pKbd->flags & LLKHF_INJECTED) {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // Bypass keystrokes if the foreground window belongs to Modern ISM (e.g. Font Switcher search box or Cheat Sheet)
    HWND fgWnd = GetForegroundWindow();
    if (fgWnd) {
        DWORD fgPid = 0;
        GetWindowThreadProcessId(fgWnd, &fgPid);
        if (fgPid == GetCurrentProcessId()) {
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }
    }

    bool isKeyUp = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

    bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    bool altDown = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    bool winDown = ((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0) || ((GetAsyncKeyState(VK_RWIN) & 0x8000) != 0);

    char asciiChar = 0;
    if (!ctrlDown && !altDown && !winDown && !isKeyUp) {
        bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool capsLock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

        DWORD vk = pKbd->vkCode;
        if (vk >= 'A' && vk <= 'Z') {
            bool upper = (shiftDown ^ capsLock);
            asciiChar = upper ? static_cast<char>(vk) : static_cast<char>(vk + 32);
        } else if (vk >= '0' && vk <= '9' && !shiftDown) {
            asciiChar = static_cast<char>(vk);
        } else {
            // Check punctuation and symbols via ToAscii
            BYTE keyState[256];
            GetKeyboardState(keyState);
            keyState[VK_SHIFT] = shiftDown ? 0x80 : 0;
            keyState[VK_CONTROL] = 0;
            keyState[VK_MENU] = 0;

            WORD charBuf = 0;
            if (ToAscii(vk, pKbd->scanCode, keyState, &charBuf, 0) == 1) {
                char ch = static_cast<char>(charBuf & 0xFF);
                if (ch >= 32 && ch <= 126) {
                    asciiChar = ch;
                }
            }
        }
    }

    bool handled = Instance().ProcessKeyEvent(pKbd->vkCode, isKeyUp, pKbd->flags,
                                            ctrlDown, altDown, winDown, asciiChar);
    if (handled) {
        return 1; // Intercept and swallow keystroke
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace minglish
