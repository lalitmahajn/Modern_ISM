#pragma once
#include "engine/transpiler_engine.hpp"
#include "engine/text_injector.hpp"
#include <memory>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

class KeyboardHookManager {
public:
    static KeyboardHookManager& Instance();

    bool Install(HINSTANCE hInstance = nullptr);
    void Uninstall();
    bool IsInstalled() const;

    void SetEngineEnabled(bool enabled);
    bool IsEngineEnabled() const;
    void ToggleEngine();

    void SetFontProfile(const FontProfile& profile);
    const FontProfile& GetFontProfile() const;

    TranspilerEngine& GetEngine();
    ITextInjector* GetInjector();
    void SetInjector(std::unique_ptr<ITextInjector> injector);

    // Event filter function (used by hook proc and unit tests)
    bool ProcessKeyEvent(DWORD vkCode, bool isKeyUp, DWORD flags, bool ctrlDown, bool altDown, bool winDown, char asciiChar);

private:
    KeyboardHookManager();
    ~KeyboardHookManager();

    KeyboardHookManager(const KeyboardHookManager&) = delete;
    KeyboardHookManager& operator=(const KeyboardHookManager&) = delete;

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK m_hook = nullptr;
    bool m_enabled = true;
    TranspilerEngine m_engine;
    std::unique_ptr<ITextInjector> m_injector;
};

} // namespace minglish
