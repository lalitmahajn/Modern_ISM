#include "hook/keyboard_hook.hpp"
#include "hook/mouse_hook.hpp"
#include "ui/status_pill.hpp"
#include "ui/tray_icon.hpp"
#include "ui/font_switcher_dialog.hpp"
#include "ui/cheat_sheet_dialog.hpp"
#include "engine/font_profile.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_set>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

namespace {
const wchar_t* HOST_WINDOW_CLASS = L"ModernISM_HostWindowClass";
const int HOTKEY_TOGGLE_ID = 101;
const int HOTKEY_FONT_SWITCHER_ID = 102;
const int HOTKEY_CHEAT_SHEET_ID = 103;
const UINT_PTR TIMER_UPDATE_ID = 1;

struct AppState {
    HINSTANCE hInstance = nullptr;
    HWND hwndHost = nullptr;
    StatusPill statusPill;
    TrayIcon trayIcon;
    std::vector<FontProfile> profiles;
    size_t activeProfileIndex = 0;
};

AppState* g_appState = nullptr;

void UpdateUI() {
    if (!g_appState) return;
    auto& kbd = KeyboardHookManager::Instance();
    bool enabled = kbd.IsEngineEnabled();
    const std::string& fontName = g_appState->profiles[g_appState->activeProfileIndex].name;
    std::string previewText = kbd.GetEngine().GetPreviouslyInjected();

    g_appState->statusPill.Update(enabled, fontName, previewText);

    std::wstring tip = L"Modern ISM: " + std::wstring(enabled ? L"ON (" : L"OFF (") +
                       std::wstring(fontName.begin(), fontName.end()) + L")";
    g_appState->trayIcon.UpdateTooltip(tip);
}

void SetActiveProfile(size_t index) {
    if (!g_appState || index >= g_appState->profiles.size()) return;
    g_appState->activeProfileIndex = index;
    KeyboardHookManager::Instance().SetFontProfile(g_appState->profiles[index]);
    UpdateUI();
}

LRESULT CALLBACK HostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY: {
            int id = static_cast<int>(wParam);
            if (id == HOTKEY_TOGGLE_ID) {
                KeyboardHookManager::Instance().ToggleEngine();
                UpdateUI();
            } else if (id == HOTKEY_FONT_SWITCHER_ID) {
                HWND hTarget = GetForegroundWindow();
                FontSwitcherDialog::Show(
                    g_appState->hInstance, hTarget, g_appState->profiles,
                    [](const FontProfile& selected) {
                        for (size_t i = 0; i < g_appState->profiles.size(); ++i) {
                            if (g_appState->profiles[i].id == selected.id) {
                                SetActiveProfile(i);
                                break;
                            }
                        }
                    }
                );
            } else if (id == HOTKEY_CHEAT_SHEET_ID) {
                HWND hTarget = GetForegroundWindow();
                CheatSheetDialog::Show(g_appState->hInstance, hTarget);
            }
            return 0;
        }

        case TrayIcon::WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP) {
                std::vector<std::string> fontNames;
                for (const auto& p : g_appState->profiles) {
                    fontNames.push_back(p.name);
                }

                g_appState->trayIcon.ShowContextMenu(
                    fontNames,
                    g_appState->activeProfileIndex,
                    KeyboardHookManager::Instance().IsEngineEnabled(),
                    g_appState->statusPill.IsAutoHide(),
                    []() {
                        KeyboardHookManager::Instance().ToggleEngine();
                        UpdateUI();
                    },
                    []() {
                        bool current = g_appState->statusPill.IsAutoHide();
                        g_appState->statusPill.SetAutoHide(!current);
                    },
                    [](size_t index) {
                        SetActiveProfile(index);
                    },
                    []() {
                        CheatSheetDialog::Show(g_appState->hInstance, GetForegroundWindow());
                    },
                    []() {
                        PostQuitMessage(0);
                    }
                );
            } else if (lParam == WM_LBUTTONDBLCLK) {
                KeyboardHookManager::Instance().ToggleEngine();
                UpdateUI();
            }
            return 0;
        }

        case WM_TIMER: {
            if (wParam == TIMER_UPDATE_ID && g_appState) {
                HWND hFg = GetForegroundWindow();
                MouseHookManager::Instance().CheckForegroundWindow(hFg);
                UpdateUI();
            }
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace
} // namespace minglish

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Single instance mutex check
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"ModernISM_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"Modern ISM is already running in the Windows System Tray.",
                    L"Modern ISM", MB_OK | MB_ICONINFORMATION);
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    minglish::AppState state;
    state.hInstance = hInstance;
    minglish::g_appState = &state;

    // Load default Unicode profile and dynamically discover all installed JSON font profiles
    state.profiles.push_back(minglish::GetDefaultUnicodeProfile());
    std::unordered_set<std::string> loadedIds = {state.profiles[0].id};

    const std::vector<std::string> searchPaths = {"config/fonts", "../config/fonts", "build/config/fonts"};
    for (const auto& dirPath : searchPaths) {
        std::error_code ec;
        if (std::filesystem::exists(dirPath, ec) && std::filesystem::is_directory(dirPath, ec)) {
            for (const auto& entry : std::filesystem::directory_iterator(dirPath, ec)) {
                if (entry.is_regular_file(ec) && entry.path().extension() == ".json") {
                    try {
                        auto prof = minglish::LoadFontProfileFromFile(entry.path().string());
                        if (!prof.id.empty() && loadedIds.find(prof.id) == loadedIds.end()) {
                            state.profiles.push_back(prof);
                            loadedIds.insert(prof.id);
                        }
                    } catch (...) {}
                }
            }
        }
    }

    // Register host window class for message pumping
    WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
    wc.lpfnWndProc = minglish::HostWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = minglish::HOST_WINDOW_CLASS;
    RegisterClassExW(&wc);

    state.hwndHost = CreateWindowExW(
        0, minglish::HOST_WINDOW_CLASS, L"ModernISM Host",
        WS_POPUP, 0, 0, 0, 0,
        nullptr, nullptr, hInstance, nullptr
    );

    // Initialize OS Hooks
    minglish::KeyboardHookManager::Instance().Install(hInstance);
    minglish::KeyboardHookManager::Instance().SetFontProfile(state.profiles[state.activeProfileIndex]);
    minglish::MouseHookManager::Instance().Install(hInstance);

    // Initialize UI Overlays
    state.statusPill.Create(hInstance);
    state.statusPill.Show();
    state.trayIcon.Create(state.hwndHost, hInstance);

    // Register Global Hotkeys
    RegisterHotKey(state.hwndHost, minglish::HOTKEY_TOGGLE_ID, MOD_CONTROL, VK_SPACE);
    RegisterHotKey(state.hwndHost, minglish::HOTKEY_FONT_SWITCHER_ID, MOD_CONTROL | MOD_SHIFT, 'F');
    RegisterHotKey(state.hwndHost, minglish::HOTKEY_CHEAT_SHEET_ID, MOD_CONTROL | MOD_SHIFT, 'H');

    // UI update timer (50ms)
    SetTimer(state.hwndHost, minglish::TIMER_UPDATE_ID, 50, nullptr);

    minglish::UpdateUI();

    // Standard Win32 Message Loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup resources on exit
    KillTimer(state.hwndHost, minglish::TIMER_UPDATE_ID);
    UnregisterHotKey(state.hwndHost, minglish::HOTKEY_TOGGLE_ID);
    UnregisterHotKey(state.hwndHost, minglish::HOTKEY_FONT_SWITCHER_ID);
    UnregisterHotKey(state.hwndHost, minglish::HOTKEY_CHEAT_SHEET_ID);

    minglish::FontSwitcherDialog::Close();
    minglish::CheatSheetDialog::Close();

    minglish::KeyboardHookManager::Instance().Uninstall();
    minglish::MouseHookManager::Instance().Uninstall();

    state.statusPill.Destroy();
    state.trayIcon.Destroy();

    if (state.hwndHost) {
        DestroyWindow(state.hwndHost);
    }

    minglish::g_appState = nullptr;
    if (hMutex) {
        CloseHandle(hMutex);
    }

    return 0;
}
