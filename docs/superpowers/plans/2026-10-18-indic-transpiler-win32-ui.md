# Universal Indic Phonetic Transpiler ("Modern ISM") Win32 Hooks & Overlay UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the low-level Windows keyboard and mouse hooks, click-through corner status pill overlay, system tray icon with context menu, Spotlight-style quick font switcher modal, and standalone executable `ModernISM.exe`.

**Architecture:** Win32 low-level hooks (`WH_KEYBOARD_LL`, `WH_MOUSE_LL`) intercepting user keystrokes and dispatching to `TranspilerEngine` + `Win32SendInputInjector`, accompanied by non-activating, click-through overlay windows and single-instance message pump.

**Tech Stack:** C++20, CMake 3.20+, MinGW-w64 (GCC 16.1 UCRT), Win32 API (`user32.lib`, `gdi32.lib`, `shell32.lib`).

**Spec:** [docs/superpowers/specs/2026-10-18-indic-phonetic-transpiler-design.md](file:///d:/Learning/Modern%20ISM/docs/superpowers/specs/2026-10-18-indic-phonetic-transpiler-design.md)

## Global Constraints
- Target platform: Windows 10/11 x64.
- Zero external runtime DLL dependencies beyond Windows UCRT.
- Hook latency: Hook callback must complete under 5 milliseconds to satisfy Windows `LowLevelHooksTimeout`.
- Overlay non-interference: Status pill window must use `WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW` and NEVER steal focus or intercept mouse clicks.
- Single-instance execution: Prevent multiple instances via a named Win32 mutex.

## Review Focus
1. **Loop prevention with LLKHF_INJECTED:** Verify hook ignores synthetic events emitted by `SendInput` to prevent infinite recursion.
2. **Modifier chord passthrough:** Verify `Ctrl`, `Alt`, and `Win` chords (`Ctrl+C`, `Ctrl+V`, `Ctrl+Z`, `Alt+Tab`) flush buffer and pass untouched.
3. **Mouse click boundary flush:** Verify `WM_LBUTTONDOWN` / `WM_RBUTTONDOWN` flushes active word buffer without emitting backspaces.
4. **Font switcher focus return:** Verify selecting a font in `Ctrl+Shift+F` modal returns keyboard focus to previous active window immediately.
5. **System tray cleanup on exit:** Verify tray icon is cleanly removed (`NIM_DELETE`) on application shutdown.

---

### Task 1: Low-Level Keyboard Hook & Event Filter (`include/hook/keyboard_hook.hpp`, `src/hook/keyboard_hook.cpp`)

**Files:**
- Create: `include/hook/keyboard_hook.hpp`
- Create: `src/hook/keyboard_hook.cpp`
- Test: `tests/test_keyboard_hook.cpp`

**Interfaces:**
- Consumes: `transpiler_engine.hpp`, `text_injector.hpp`
- Produces:
  ```cpp
  namespace minglish {
      class KeyboardHookManager {
      public:
          static KeyboardHookManager& Instance();
          bool Install(HINSTANCE hInstance);
          void Uninstall();
          bool IsInstalled() const;

          void SetEngineEnabled(bool enabled);
          bool IsEngineEnabled() const;

          void SetFontProfile(const FontProfile& profile);
          const FontProfile& GetFontProfile() const;

          TranspilerEngine& GetEngine();
          ITextInjector& GetInjector();

          // Hook filter logic (testable without installing global OS hook)
          bool ProcessKeyEvent(DWORD vkCode, bool isKeyUp, DWORD flags);
      };
  }
  ```

- [ ] **Step 1: Write unit tests in `tests/test_keyboard_hook.cpp` testing key event filtering logic**
- [ ] **Step 2: Run test to verify it fails**
- [ ] **Step 3: Implement `include/hook/keyboard_hook.hpp` and `src/hook/keyboard_hook.cpp`**
- [ ] **Step 4: Run test to verify it passes**
- [ ] **Step 5: Commit**

---

### Task 2: Mouse Hook & Focus Boundary Detection (`include/hook/mouse_hook.hpp`, `src/hook/mouse_hook.cpp`)

**Files:**
- Create: `include/hook/mouse_hook.hpp`
- Create: `src/hook/mouse_hook.cpp`
- Test: `tests/test_mouse_hook.cpp`

**Interfaces:**
- Consumes: `keyboard_hook.hpp`
- Produces:
  ```cpp
  namespace minglish {
      class MouseHookManager {
      public:
          static MouseHookManager& Instance();
          bool Install(HINSTANCE hInstance);
          void Uninstall();
          bool IsInstalled() const;

          // Pure testable filter function
          bool ProcessMouseEvent(UINT msg);
      };
  }
  ```

- [ ] **Step 1: Write unit tests in `tests/test_mouse_hook.cpp` verifying buffer flush on clicks**
- [ ] **Step 2: Run test to verify it fails**
- [ ] **Step 3: Implement `include/hook/mouse_hook.hpp` and `src/hook/mouse_hook.cpp`**
- [ ] **Step 4: Run test to verify it passes**
- [ ] **Step 5: Commit**

---

### Task 3: Click-Through Corner Status Pill Overlay (`include/ui/status_pill.hpp`, `src/ui/status_pill.cpp`)

**Files:**
- Create: `include/ui/status_pill.hpp`
- Create: `src/ui/status_pill.cpp`

**Interfaces:**
- Consumes: Win32 GDI / Layered Window APIs
- Produces:
  ```cpp
  namespace minglish {
      class StatusPill {
      public:
          StatusPill();
          ~StatusPill();
          bool Create(HINSTANCE hInstance);
          void Update(bool isEngineOn, const std::string& fontName, const std::string& previewText = "");
          void Show();
          void Hide();
          HWND GetHwnd() const;
      };
  }
  ```

- [ ] **Step 1: Implement `include/ui/status_pill.hpp` and `src/ui/status_pill.cpp` with `WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW` and double-buffered GDI rendering**
- [ ] **Step 2: Build and verify compilation**
- [ ] **Step 3: Commit**

---

### Task 4: Quick Font Switcher Modal & Cheat Sheet Dialog (`include/ui/font_switcher_dialog.hpp`, `src/ui/font_switcher_dialog.cpp`, `include/ui/cheat_sheet_dialog.hpp`, `src/ui/cheat_sheet_dialog.cpp`)

**Files:**
- Create: `include/ui/font_switcher_dialog.hpp`
- Create: `src/ui/font_switcher_dialog.cpp`
- Create: `include/ui/cheat_sheet_dialog.hpp`
- Create: `src/ui/cheat_sheet_dialog.cpp`

**Interfaces:**
- Consumes: `font_profile.hpp`
- Produces:
  ```cpp
  namespace minglish {
      class FontSwitcherDialog {
      public:
          static void Show(HINSTANCE hInstance, HWND hParent,
                           const std::vector<FontProfile>& availableProfiles,
                           std::function<void(const FontProfile&)> onSelect);
      };

      class CheatSheetDialog {
      public:
          static void Show(HINSTANCE hInstance, HWND hParent);
      };
  }
  ```

- [ ] **Step 1: Implement `font_switcher_dialog.hpp/.cpp` and `cheat_sheet_dialog.hpp/.cpp` with centered window, search filter, and instant focus return**
- [ ] **Step 2: Build and verify compilation**
- [ ] **Step 3: Commit**

---

### Task 5: System Tray Host, Single-Instance Application & Main Entry Point (`include/ui/tray_icon.hpp`, `src/ui/tray_icon.cpp`, `src/main.cpp`)

**Files:**
- Create: `include/ui/tray_icon.hpp`
- Create: `src/ui/tray_icon.cpp`
- Create: `src/main.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: All engine, hook, and UI components
- Produces: Standalone binary `build/bin/ModernISM.exe` (or `build/ModernISM.exe`)

- [ ] **Step 1: Implement `include/ui/tray_icon.hpp` and `src/ui/tray_icon.cpp` with right-click context menu and global hotkeys**
- [ ] **Step 2: Implement `src/main.cpp` WinMain entry point with named mutex, hotkey registration, and message loop**
- [ ] **Step 3: Update `CMakeLists.txt` with executable target `ModernISM`**
- [ ] **Step 4: Build full project and verify binary output**
- [ ] **Step 5: Run complete test suite and verify 100% pass**
- [ ] **Step 6: Commit**
