#include "ui/font_switcher_dialog.hpp"
#include <algorithm>

namespace minglish {

namespace {
const wchar_t* FONT_SWITCHER_CLASS = L"ModernISM_FontSwitcher";
const int IDC_SEARCH_EDIT = 1001;
const int IDC_PROFILE_LIST = 1002;
}

FontSwitcherDialog* FontSwitcherDialog::s_instance = nullptr;

bool FontSwitcherDialog::IsOpen() {
    return s_instance != nullptr && s_instance->m_hwnd != nullptr && IsWindow(s_instance->m_hwnd);
}

void FontSwitcherDialog::Close() {
    if (s_instance && s_instance->m_hwnd && IsWindow(s_instance->m_hwnd)) {
        DestroyWindow(s_instance->m_hwnd);
    }
}

void FontSwitcherDialog::Show(HINSTANCE hInstance,
                              HWND hTargetApp,
                              const std::vector<FontProfile>& availableProfiles,
                              OnSelectCallback onSelect) {
    if (IsOpen()) {
        SetForegroundWindow(s_instance->m_hwnd);
        return;
    }

    if (!hInstance) {
        hInstance = GetModuleHandleW(nullptr);
    }

    s_instance = new FontSwitcherDialog();
    s_instance->m_hInstance = hInstance;
    s_instance->m_hTargetApp = hTargetApp;
    s_instance->m_profiles = availableProfiles;
    s_instance->m_onSelect = std::move(onSelect);

    WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = FONT_SWITCHER_CLASS;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassExW(&wc);

    int width = 420;
    int height = 280;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - width) / 2;
    int y = (screenH - height) / 3; // Position slightly above vertical center (Spotlight style)

    s_instance->m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
        FONT_SWITCHER_CLASS,
        L"Quick Font Switcher (Ctrl+Shift+F)",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        nullptr, nullptr, hInstance, s_instance
    );

    if (s_instance->m_hwnd) {
        SetForegroundWindow(s_instance->m_hwnd);
        SetFocus(s_instance->m_hEdit);
    }
}

void FontSwitcherDialog::FilterProfiles(const std::wstring& query) {
    m_filteredIndices.clear();
    SendMessageW(m_hListBox, LB_RESETCONTENT, 0, 0);

    std::wstring lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::towlower);

    for (size_t i = 0; i < m_profiles.size(); ++i) {
        const auto& p = m_profiles[i];
        std::wstring nameW(p.name.begin(), p.name.end());
        std::wstring lowerName = nameW;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::towlower);

        if (lowerQuery.empty() || lowerName.find(lowerQuery) != std::wstring::npos) {
            m_filteredIndices.push_back(i);
            std::wstring itemText = nameW + L" (" + std::wstring(p.encoding.begin(), p.encoding.end()) + L")";
            SendMessageW(m_hListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(itemText.c_str()));
        }
    }

    if (!m_filteredIndices.empty()) {
        SendMessageW(m_hListBox, LB_SETCURSEL, 0, 0);
    }
}

void FontSwitcherDialog::SelectCurrent() {
    LRESULT curSel = SendMessageW(m_hListBox, LB_GETCURSEL, 0, 0);
    if (curSel != LB_ERR && static_cast<size_t>(curSel) < m_filteredIndices.size()) {
        size_t profileIndex = m_filteredIndices[static_cast<size_t>(curSel)];
        if (m_onSelect && profileIndex < m_profiles.size()) {
            m_onSelect(m_profiles[profileIndex]);
        }
    }

    HWND target = m_hTargetApp;
    DestroyWindow(m_hwnd);

    if (target && IsWindow(target)) {
        SetForegroundWindow(target);
    }
}

LRESULT CALLBACK FontSwitcherDialog::EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (s_instance) {
        if (msg == WM_KEYDOWN) {
            if (wParam == VK_RETURN) {
                s_instance->SelectCurrent();
                return 0;
            } else if (wParam == VK_ESCAPE) {
                HWND target = s_instance->m_hTargetApp;
                DestroyWindow(s_instance->m_hwnd);
                if (target && IsWindow(target)) {
                    SetForegroundWindow(target);
                }
                return 0;
            } else if (wParam == VK_UP || wParam == VK_DOWN) {
                SendMessageW(s_instance->m_hListBox, msg, wParam, lParam);
                return 0;
            }
        }
        return CallWindowProcW(s_instance->m_origEditProc, hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK FontSwitcherDialog::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* self = reinterpret_cast<FontSwitcherDialog*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));

        HFONT hFont = CreateFontW(
            -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
        );

        // Edit box for search
        self->m_hEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            16, 16, 388, 28,
            hwnd, reinterpret_cast<HMENU>(IDC_SEARCH_EDIT),
            self->m_hInstance, nullptr
        );
        SendMessageW(self->m_hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

        // Subclass Edit box to intercept Enter, Esc, Up, Down
        self->m_origEditProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(self->m_hEdit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(EditWndProc))
        );

        // List box for profiles
        self->m_hListBox = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"LISTBOX", L"",
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
            16, 52, 388, 160,
            hwnd, reinterpret_cast<HMENU>(IDC_PROFILE_LIST),
            self->m_hInstance, nullptr
        );
        SendMessageW(self->m_hListBox, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

        // Populate initially
        self->FilterProfiles(L"");

        return 0;
    }

    auto* self = reinterpret_cast<FontSwitcherDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == IDC_SEARCH_EDIT && wmEvent == EN_CHANGE && self) {
                wchar_t buf[128] = {0};
                GetWindowTextW(self->m_hEdit, buf, 128);
                self->FilterProfiles(buf);
                return 0;
            } else if (wmId == IDC_PROFILE_LIST && wmEvent == LBN_DBLCLK && self) {
                self->SelectCurrent();
                return 0;
            }
            break;
        }
        case WM_DESTROY: {
            if (self) {
                HWND target = self->m_hTargetApp;
                delete self;
                s_instance = nullptr;
                if (target && IsWindow(target)) {
                    SetForegroundWindow(target);
                }
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace minglish
