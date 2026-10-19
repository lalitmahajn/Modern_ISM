#include "ui/status_pill.hpp"

namespace minglish {

namespace {
const wchar_t* STATUS_PILL_CLASS = L"ModernISM_StatusPill";

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    if (needed <= 0) return L"";
    std::wstring out(needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &out[0], needed);
    return out;
}
}

StatusPill::StatusPill() = default;

StatusPill::~StatusPill() {
    Destroy();
}

bool StatusPill::Create(HINSTANCE hInstance) {
    if (m_hwnd) {
        return true;
    }

    if (!hInstance) {
        hInstance = GetModuleHandleW(nullptr);
    }
    m_hInstance = hInstance;

    static bool s_classRegistered = false;
    if (!s_classRegistered) {
        WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = m_hInstance;
        wc.lpszClassName = STATUS_PILL_CLASS;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr; // Double-buffered GDI custom paint
        RegisterClassExW(&wc);
        s_classRegistered = true;
    }

    RECT workArea;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.right - m_width - 24;
    int y = workArea.bottom - m_height - 24;

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        STATUS_PILL_CLASS,
        L"ModernISM Status",
        WS_POPUP,
        x, y, m_width, m_height,
        nullptr, nullptr, m_hInstance, this
    );

    if (m_hwnd) {
        m_alpha = 255;
        SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
        // Set continuous capsule region (circle caps on both left & right)
        HRGN hRgn = CreateRoundRectRgn(0, 0, m_width + 1, m_height + 1, m_height, m_height);
        SetWindowRgn(m_hwnd, hRgn, TRUE);
    }

    return m_hwnd != nullptr;
}

void StatusPill::Destroy() {
    if (m_hwnd) {
        KillTimer(m_hwnd, TIMER_AUTOHIDE);
        KillTimer(m_hwnd, TIMER_FADE);
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void StatusPill::TriggerPopup() {
    if (!m_hwnd) return;
    KillTimer(m_hwnd, TIMER_FADE);
    m_alpha = 255;
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    PositionWindow();
    ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    InvalidateRect(m_hwnd, nullptr, FALSE);

    if (m_autoHide) {
        StartAutoHideTimer();
    }
}

void StatusPill::StartAutoHideTimer() {
    if (!m_hwnd) return;
    KillTimer(m_hwnd, TIMER_FADE);
    KillTimer(m_hwnd, TIMER_AUTOHIDE);
    SetTimer(m_hwnd, TIMER_AUTOHIDE, AUTOHIDE_DELAY_MS, nullptr);
}

void StatusPill::StepFade() {
    if (!m_hwnd) return;
    if (m_alpha <= FADE_STEP_ALPHA) {
        m_alpha = 0;
        KillTimer(m_hwnd, TIMER_FADE);
        ShowWindow(m_hwnd, SW_HIDE);
    } else {
        m_alpha -= FADE_STEP_ALPHA;
        SetLayeredWindowAttributes(m_hwnd, 0, m_alpha, LWA_ALPHA);
    }
}

void StatusPill::Update(bool isEngineOn, const std::string& fontName, const std::string& previewText) {
    bool stateChanged = (isEngineOn != m_engineOn) || (fontName != m_fontName) || (previewText != m_previewText);
    m_engineOn = isEngineOn;
    m_fontName = fontName;
    m_previewText = previewText;

    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
        if (stateChanged) {
            TriggerPopup();
        }
    }
}

void StatusPill::Show() {
    if (m_hwnd) {
        if (m_autoHide) {
            TriggerPopup();
        } else {
            KillTimer(m_hwnd, TIMER_AUTOHIDE);
            KillTimer(m_hwnd, TIMER_FADE);
            m_alpha = 255;
            SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
            PositionWindow();
            ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
    }
}

void StatusPill::Hide() {
    if (m_hwnd) {
        KillTimer(m_hwnd, TIMER_AUTOHIDE);
        KillTimer(m_hwnd, TIMER_FADE);
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

bool StatusPill::IsVisible() const {
    return m_hwnd && IsWindowVisible(m_hwnd);
}

void StatusPill::SetAutoHide(bool autoHide) {
    m_autoHide = autoHide;
    if (!m_autoHide) {
        Show();
    } else {
        TriggerPopup();
    }
}

bool StatusPill::IsAutoHide() const {
    return m_autoHide;
}

void StatusPill::PositionWindow() {
    if (!m_hwnd) return;
    RECT workArea;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.right - m_width - 24;
    int y = workArea.bottom - m_height - 24;
    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, m_width, m_height, SWP_NOACTIVATE | SWP_NOSIZE);
}

void StatusPill::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, m_width, m_height);
    HGDIOBJ oldBitmap = SelectObject(memDC, memBitmap);

    // 1. Fluent Dark Capsule Background & Subtle Top-Lit Border
    HBRUSH bgBrush = CreateSolidBrush(RGB(22, 23, 27));
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(52, 56, 66));
    HGDIOBJ oldBrush = SelectObject(memDC, bgBrush);
    HGDIOBJ oldPen = SelectObject(memDC, borderPen);

    RoundRect(memDC, 0, 0, m_width, m_height, m_height, m_height);

    SelectObject(memDC, oldBrush);
    DeleteObject(bgBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(borderPen);

    // 2. Left Icon Badge (Circular Container: 30x30)
    int badgeX = 8;
    int badgeY = 7;
    int badgeSize = 30;

    HBRUSH badgeBgBrush = m_engineOn ? CreateSolidBrush(RGB(24, 62, 40)) : CreateSolidBrush(RGB(38, 41, 48));
    HPEN badgeBorderPen = m_engineOn ? CreatePen(PS_SOLID, 1, RGB(46, 160, 90)) : CreatePen(PS_SOLID, 1, RGB(65, 70, 82));
    oldBrush = SelectObject(memDC, badgeBgBrush);
    oldPen = SelectObject(memDC, badgeBorderPen);

    RoundRect(memDC, badgeX, badgeY, badgeX + badgeSize, badgeY + badgeSize, badgeSize, badgeSize);

    SelectObject(memDC, oldBrush);
    DeleteObject(badgeBgBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(badgeBorderPen);

    // 3. Draw Badge Icon ("अ" in Devanagari when ON, "A" when OFF)
    SetBkMode(memDC, TRANSPARENT);
    HFONT hBadgeFont = CreateFontW(
        m_engineOn ? -15 : -14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        m_engineOn ? L"Nirmala UI" : L"Segoe UI"
    );
    HGDIOBJ oldFont = SelectObject(memDC, hBadgeFont);

    RECT badgeTextRect = {badgeX, badgeY, badgeX + badgeSize, badgeY + badgeSize};
    SetTextColor(memDC, m_engineOn ? RGB(46, 204, 113) : RGB(165, 170, 180));
    DrawTextW(memDC, m_engineOn ? L"अ" : L"A", -1, &badgeTextRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    SelectObject(memDC, oldFont);
    DeleteObject(hBadgeFont);

    // 4. Draw Label & Font Information
    HFONT hTextFont = CreateFontW(
        -13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );
    oldFont = SelectObject(memDC, hTextFont);

    RECT textRect = {46, 0, m_width - 16, m_height};
    std::wstring displayMsg;
    if (m_engineOn) {
        displayMsg = L"Minglish: ON  •  " + Utf8ToWide(m_fontName);
        if (!m_previewText.empty()) {
            displayMsg += L"  (" + Utf8ToWide(m_previewText) + L")";
        }
        SetTextColor(memDC, RGB(245, 245, 247));
    } else {
        displayMsg = L"English (Bypass)  •  Ctrl+Space";
        SetTextColor(memDC, RGB(170, 175, 185));
    }

    DrawTextW(memDC, displayMsg.c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);

    SelectObject(memDC, oldFont);
    DeleteObject(hTextFont);

    // 5. Blit to Screen
    BitBlt(hdc, 0, 0, m_width, m_height, memDC, 0, 0, SRCCOPY);

    // Cleanup Device Context
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK StatusPill::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    StatusPill* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<StatusPill*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<StatusPill*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    switch (msg) {
        case WM_TIMER: {
            if (self) {
                if (wParam == TIMER_AUTOHIDE) {
                    KillTimer(hwnd, TIMER_AUTOHIDE);
                    SetTimer(hwnd, TIMER_FADE, FADE_STEP_MS, nullptr);
                    return 0;
                } else if (wParam == TIMER_FADE) {
                    self->StepFade();
                    return 0;
                }
            }
            break;
        }
        case WM_PAINT:
            if (self) {
                self->OnPaint(hwnd);
                return 0;
            }
            break;
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        case WM_DESTROY:
            if (self) {
                KillTimer(hwnd, TIMER_AUTOHIDE);
                KillTimer(hwnd, TIMER_FADE);
            }
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace minglish
