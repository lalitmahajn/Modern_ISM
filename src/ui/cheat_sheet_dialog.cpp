#include "ui/cheat_sheet_dialog.hpp"

namespace minglish {

namespace {
const wchar_t* CHEAT_SHEET_CLASS = L"ModernISM_CheatSheet";

const wchar_t* CHEAT_SHEET_TEXT =
    L"=== MINGLISH PHONETIC KEYMAP QUICK REFERENCE ===\r\n\r\n"
    L"VOWELS & MATRAS:\r\n"
    L"  a = अ    aa / A = आ / ा    i = इ / ि    ee / I = ई / ी\r\n"
    L"  u = उ / ु    oo / U = ऊ / ू    e = ए / े    ai = ऐ / ै\r\n"
    L"  o = ओ / ो    au = औ / ौ    Ru = ऋ / ृ\r\n\r\n"
    L"MODIFIERS & SPECIALS:\r\n"
    L"  M = Anusvara (ं)     H = Visarga (ः)\r\n"
    L"  c / E = Chandra E (ॅ)   O = Chandra O (ॉ)\r\n"
    L"  ~ = Explicit Halant (्)\r\n\r\n"
    L"MARATHI FIRST-CLASS SUPPORT:\r\n"
    L"  L / lh = ळ          dny / jny = ज्ञ         ksh = क्ष\r\n"
    L"  rh / ry = ऱ्ह / ऱ्या (कुऱ्हाड / वाऱ्याचा - Eyelash Ra)\r\n\r\n"
    L"3 FORMS OF 'R':\r\n"
    L"  1. Reph (रफ़ार):  k-a-r-m = कर्म (Flying Top Hook)\r\n"
    L"  2. Rakar (रकार):  k-r-a = क्र (Bottom Diagonal Stroke)\r\n"
    L"  3. Caret Rakar:   t-r-a = ट्र (Stemless Caret for ट, ठ, ड, ढ, छ)\r\n\r\n"
    L"NUMERALS: 0-9 = ०-९ (Devanagari Digits)\r\n\r\n"
    L"GLOBAL HOTKEYS:\r\n"
    L"  Ctrl + Space     : Toggle Minglish Transpiler ON / OFF\r\n"
    L"  Ctrl + Shift + F : Quick Font Switcher Modal\r\n"
    L"  Ctrl + Shift + H : This Keymap Cheat Sheet\r\n"
    L"  Esc              : Close this window";
}

CheatSheetDialog* CheatSheetDialog::s_instance = nullptr;

bool CheatSheetDialog::IsOpen() {
    return s_instance != nullptr && s_instance->m_hwnd != nullptr && IsWindow(s_instance->m_hwnd);
}

void CheatSheetDialog::Close() {
    if (s_instance && s_instance->m_hwnd && IsWindow(s_instance->m_hwnd)) {
        DestroyWindow(s_instance->m_hwnd);
    }
}

void CheatSheetDialog::Show(HINSTANCE hInstance, HWND hTargetApp) {
    if (IsOpen()) {
        SetForegroundWindow(s_instance->m_hwnd);
        return;
    }

    if (!hInstance) {
        hInstance = GetModuleHandleW(nullptr);
    }

    s_instance = new CheatSheetDialog();
    s_instance->m_hInstance = hInstance;
    s_instance->m_hTargetApp = hTargetApp;

    WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CHEAT_SHEET_CLASS;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassExW(&wc);

    int width = 560;
    int height = 500;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - width) / 2;
    int y = (screenH - height) / 2;

    s_instance->m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
        CHEAT_SHEET_CLASS,
        L"Minglish Keymap Cheat Sheet (Press Esc to Close)",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        nullptr, nullptr, hInstance, s_instance
    );

    if (s_instance->m_hwnd) {
        SetForegroundWindow(s_instance->m_hwnd);
    }
}

LRESULT CALLBACK CheatSheetDialog::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* self = reinterpret_cast<CheatSheetDialog*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));

        HFONT hFont = CreateFontW(
            -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
        );

        HWND hEdit = CreateWindowExW(
            0, L"EDIT", CHEAT_SHEET_TEXT,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            12, 12, 520, 436,
            hwnd, nullptr, self->m_hInstance, nullptr
        );
        SendMessageW(hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
        SetFocus(hEdit);

        return 0;
    }

    auto* self = reinterpret_cast<CheatSheetDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
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
