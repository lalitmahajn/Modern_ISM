#pragma once
#include <functional>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

namespace minglish {

class TrayIcon {
public:
    using ActionCallback = std::function<void()>;
    using FontSelectCallback = std::function<void(size_t)>;

    TrayIcon();
    ~TrayIcon();

    bool Create(HWND hwndOwner, HINSTANCE hInstance);
    void Destroy();

    void UpdateTooltip(const std::wstring& tooltip);
    void ShowContextMenu(const std::vector<std::string>& fontNames,
                         size_t activeFontIndex,
                         bool isEngineOn,
                         ActionCallback onToggle,
                         FontSelectCallback onSelectFont,
                         ActionCallback onShowCheatSheet,
                         ActionCallback onExit);

    static const UINT WM_TRAYICON = WM_USER + 100;

private:
    HWND m_hwndOwner = nullptr;
    HINSTANCE m_hInstance = nullptr;
    NOTIFYICONDATAW m_nid = {sizeof(NOTIFYICONDATAW)};
    bool m_installed = false;
};

} // namespace minglish
