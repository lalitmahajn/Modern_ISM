#pragma once
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

class StatusPill {
public:
    StatusPill();
    ~StatusPill();

    bool Create(HINSTANCE hInstance = nullptr);
    void Destroy();

    void Update(bool isEngineOn, const std::string& fontName, const std::string& previewText = "");
    void TriggerPopup();
    void Show();
    void Hide();
    bool IsVisible() const;

    void SetAutoHide(bool autoHide);
    bool IsAutoHide() const;

    HWND GetHwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnPaint(HWND hwnd);
    void PositionWindow();
    void StartAutoHideTimer();
    void StepFade();

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    bool m_engineOn = true;
    std::string m_fontName = "Unicode";
    std::string m_previewText = "";
    bool m_autoHide = true;
    BYTE m_alpha = 255;
    int m_width = 280;
    int m_height = 44;

    static const UINT_PTR TIMER_AUTOHIDE = 1001;
    static const UINT_PTR TIMER_FADE = 1002;
    static const UINT AUTOHIDE_DELAY_MS = 2000;
    static const UINT FADE_STEP_MS = 16;
    static const BYTE FADE_STEP_ALPHA = 20;
};

} // namespace minglish
