#include "ui/tray_icon.hpp"

namespace minglish {

TrayIcon::TrayIcon() = default;

TrayIcon::~TrayIcon() {
    Destroy();
}

bool TrayIcon::Create(HWND hwndOwner, HINSTANCE hInstance) {
    if (m_installed) {
        return true;
    }

    m_hwndOwner = hwndOwner;
    m_hInstance = hInstance ? hInstance : GetModuleHandleW(nullptr);

    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = m_hwndOwner;
    m_nid.uID = 1001;
    m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(m_nid.szTip, L"Modern ISM - Indic Transpiler");

    m_installed = Shell_NotifyIconW(NIM_ADD, &m_nid) == TRUE;
    return m_installed;
}

void TrayIcon::Destroy() {
    if (m_installed) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_installed = false;
    }
}

void TrayIcon::UpdateTooltip(const std::wstring& tooltip) {
    if (!m_installed) return;
    wcsncpy_s(m_nid.szTip, tooltip.c_str(), 127);
    m_nid.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::ShowContextMenu(const std::vector<std::string>& fontNames,
                             size_t activeFontIndex,
                             bool isEngineOn,
                             ActionCallback onToggle,
                             FontSelectCallback onSelectFont,
                             ActionCallback onShowCheatSheet,
                             ActionCallback onExit) {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    UINT toggleFlags = MF_STRING | (isEngineOn ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hMenu, toggleFlags, 101, isEngineOn ? L"Minglish: Enabled (Ctrl+Space)" : L"Minglish: Disabled (Ctrl+Space)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);

    HMENU hFontMenu = CreatePopupMenu();
    for (size_t i = 0; i < fontNames.size(); ++i) {
        std::wstring nameW(fontNames[i].begin(), fontNames[i].end());
        UINT fontFlags = MF_STRING | (i == activeFontIndex ? MF_CHECKED : MF_UNCHECKED);
        AppendMenuW(hFontMenu, fontFlags, 200 + static_cast<UINT>(i), nameW.c_str());
    }
    AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hFontMenu), L"Target Font Profile");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);

    AppendMenuW(hMenu, MF_STRING, 102, L"Keymap Cheat Sheet (Ctrl+Shift+H)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, 103, L"Exit Modern ISM");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(m_hwndOwner);

    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwndOwner, nullptr);
    PostMessageW(m_hwndOwner, WM_NULL, 0, 0);

    if (cmd == 101 && onToggle) {
        onToggle();
    } else if (cmd == 102 && onShowCheatSheet) {
        onShowCheatSheet();
    } else if (cmd == 103 && onExit) {
        onExit();
    } else if (cmd >= 200 && static_cast<size_t>(cmd - 200) < fontNames.size() && onSelectFont) {
        onSelectFont(static_cast<size_t>(cmd - 200));
    }

    DestroyMenu(hMenu);
}

} // namespace minglish
