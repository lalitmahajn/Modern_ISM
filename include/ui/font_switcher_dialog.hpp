#pragma once
#include "engine/font_profile.hpp"
#include <vector>
#include <functional>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

class FontSwitcherDialog {
public:
    using OnSelectCallback = std::function<void(const FontProfile&)>;

    static void Show(HINSTANCE hInstance,
                     HWND hTargetApp,
                     const std::vector<FontProfile>& availableProfiles,
                     OnSelectCallback onSelect);

    static bool IsOpen();
    static void Close();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void FilterProfiles(const std::wstring& query);
    void SelectCurrent();

    HWND m_hwnd = nullptr;
    HWND m_hEdit = nullptr;
    HWND m_hListBox = nullptr;
    HWND m_hTargetApp = nullptr;
    HINSTANCE m_hInstance = nullptr;
    std::vector<FontProfile> m_profiles;
    std::vector<size_t> m_filteredIndices;
    OnSelectCallback m_onSelect;
    WNDPROC m_origEditProc = nullptr;

    static FontSwitcherDialog* s_instance;
};

} // namespace minglish
