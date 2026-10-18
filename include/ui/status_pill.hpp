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
    void Show();
    void Hide();
    bool IsVisible() const;

    HWND GetHwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnPaint(HWND hwnd);
    void PositionWindow();

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    bool m_engineOn = true;
    std::string m_fontName = "Unicode";
    std::string m_previewText = "";
    int m_width = 240;
    int m_height = 40;
};

} // namespace minglish
