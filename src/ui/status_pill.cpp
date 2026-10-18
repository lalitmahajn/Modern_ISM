#include "ui/status_pill.hpp"

namespace minglish {

namespace {
const wchar_t* STATUS_PILL_CLASS = L"ModernISM_StatusPill";
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

    WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = STATUS_PILL_CLASS;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // Double-buffered GDI custom paint

    RegisterClassExW(&wc);

    RECT workArea;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.right - m_width - 24;
    int y = workArea.bottom - m_height - 24;

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        STATUS_PILL_CLASS,
        L"ModernISM Status",
        WS_POPUP,
        x, y, m_width, m_height,
        nullptr, nullptr, m_hInstance, this
    );

    if (m_hwnd) {
        // Set rounded window region for native pill look
        HRGN hRgn = CreateRoundRectRgn(0, 0, m_width + 1, m_height + 1, 16, 16);
        SetWindowRgn(m_hwnd, hRgn, TRUE);
    }

    return m_hwnd != nullptr;
}

void StatusPill::Destroy() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void StatusPill::Update(bool isEngineOn, const std::string& fontName, const std::string& previewText) {
    m_engineOn = isEngineOn;
    m_fontName = fontName;
    m_previewText = previewText;

    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void StatusPill::Show() {
    if (m_hwnd) {
        PositionWindow();
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    }
}

void StatusPill::Hide() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_HIDE);
    }
}

bool StatusPill::IsVisible() const {
    return m_hwnd && IsWindowVisible(m_hwnd);
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

    // Dark slate background: RGB(32, 34, 37)
    HBRUSH bgBrush = CreateSolidBrush(RGB(32, 34, 37));
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(55, 58, 64));
    HGDIOBJ oldBrush = SelectObject(memDC, bgBrush);
    HGDIOBJ oldPen = SelectObject(memDC, borderPen);

    RoundRect(memDC, 0, 0, m_width, m_height, 16, 16);

    // Draw status dot: emerald green if ON, neutral gray if OFF
    COLORREF dotColor = m_engineOn ? RGB(46, 204, 113) : RGB(128, 132, 142);
    HBRUSH dotBrush = CreateSolidBrush(dotColor);
    HPEN dotPen = CreatePen(PS_SOLID, 1, dotColor);
    SelectObject(memDC, dotBrush);
    SelectObject(memDC, dotPen);
    Ellipse(memDC, 14, 14, 26, 26);
    DeleteObject(dotBrush);
    DeleteObject(dotPen);

    // Text rendering with Segoe UI
    SetBkMode(memDC, TRANSPARENT);
    HFONT hFont = CreateFontW(
        -13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );
    HGDIOBJ oldFont = SelectObject(memDC, hFont);

    RECT textRect = {34, 0, m_width - 12, m_height};
    std::wstring displayMsg;
    if (m_engineOn) {
        displayMsg = L"Minglish: ON  |  ";
        displayMsg.append(m_fontName.begin(), m_fontName.end());
        if (!m_previewText.empty()) {
            displayMsg += L" (";
            displayMsg.append(m_previewText.begin(), m_previewText.end());
            displayMsg += L")";
        }
        SetTextColor(memDC, RGB(245, 245, 247));
    } else {
        displayMsg = L"English (Bypass)";
        SetTextColor(memDC, RGB(160, 164, 172));
    }

    DrawTextW(memDC, displayMsg.c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);

    // Blit to screen
    BitBlt(hdc, 0, 0, m_width, m_height, memDC, 0, 0, SRCCOPY);

    // Cleanup GDI objects
    SelectObject(memDC, oldFont);
    DeleteObject(hFont);
    SelectObject(memDC, oldBrush);
    DeleteObject(bgBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(borderPen);
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
        case WM_PAINT:
            if (self) {
                self->OnPaint(hwnd);
                return 0;
            }
            break;
        case WM_NCHITTEST:
            // HTTRANSPARENT ensures clicks pass through to the document window below
            return HTTRANSPARENT;
        case WM_DESTROY:
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace minglish
