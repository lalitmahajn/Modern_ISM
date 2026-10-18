#include "test_framework.hpp"
#include "hook/mouse_hook.hpp"
#include "hook/keyboard_hook.hpp"

TEST_CASE(TestMouseClickFlushesBuffer) {
    auto& kbd = minglish::KeyboardHookManager::Instance();
    auto& mouse = minglish::MouseHookManager::Instance();

    kbd.SetEngineEnabled(true);
    kbd.GetEngine().Reset();

    // Type 'k' -> buffer has 'क'
    kbd.ProcessKeyEvent('K', false, 0, false, false, false, 'k');
    ASSERT_TRUE(!kbd.GetEngine().GetCurrentUnicode().empty());

    // Left mouse click should flush the buffer without backspacing
    bool resetTriggered = mouse.ProcessMouseEvent(WM_LBUTTONDOWN);
    ASSERT_TRUE(resetTriggered);
    ASSERT_TRUE(kbd.GetEngine().GetCurrentUnicode().empty());
    ASSERT_EQ(kbd.GetEngine().GetPreviouslyInjected(), "");

    return true;
}

TEST_CASE(TestRightMouseClickFlushesBuffer) {
    auto& kbd = minglish::KeyboardHookManager::Instance();
    auto& mouse = minglish::MouseHookManager::Instance();

    kbd.SetEngineEnabled(true);
    kbd.GetEngine().Reset();

    // Type 'k'
    kbd.ProcessKeyEvent('K', false, 0, false, false, false, 'k');
    ASSERT_TRUE(!kbd.GetEngine().GetCurrentUnicode().empty());

    // Right mouse click flushes buffer
    bool resetTriggered = mouse.ProcessMouseEvent(WM_RBUTTONDOWN);
    ASSERT_TRUE(resetTriggered);
    ASSERT_TRUE(kbd.GetEngine().GetCurrentUnicode().empty());

    return true;
}

TEST_CASE(TestMouseMoveDoesNotFlushBuffer) {
    auto& kbd = minglish::KeyboardHookManager::Instance();
    auto& mouse = minglish::MouseHookManager::Instance();

    kbd.SetEngineEnabled(true);
    kbd.GetEngine().Reset();

    // Type 'k'
    kbd.ProcessKeyEvent('K', false, 0, false, false, false, 'k');
    ASSERT_TRUE(!kbd.GetEngine().GetCurrentUnicode().empty());

    // Mouse move should NOT flush buffer
    bool resetTriggered = mouse.ProcessMouseEvent(WM_MOUSEMOVE);
    ASSERT_TRUE(!resetTriggered);
    ASSERT_TRUE(!kbd.GetEngine().GetCurrentUnicode().empty());

    return true;
}

TEST_CASE(TestWindowFocusChangeFlushesBuffer) {
    auto& kbd = minglish::KeyboardHookManager::Instance();
    auto& mouse = minglish::MouseHookManager::Instance();

    kbd.SetEngineEnabled(true);
    kbd.GetEngine().Reset();

    // Establish initial window
    HWND win1 = reinterpret_cast<HWND>(0x1234);
    HWND win2 = reinterpret_cast<HWND>(0x5678);

    mouse.CheckForegroundWindow(win1);

    // Type 'k'
    kbd.ProcessKeyEvent('K', false, 0, false, false, false, 'k');
    ASSERT_TRUE(!kbd.GetEngine().GetCurrentUnicode().empty());

    // Check same window: no flush
    bool changed1 = mouse.CheckForegroundWindow(win1);
    ASSERT_TRUE(!changed1);
    ASSERT_TRUE(!kbd.GetEngine().GetCurrentUnicode().empty());

    // Switch to win2: flushes buffer
    bool changed2 = mouse.CheckForegroundWindow(win2);
    ASSERT_TRUE(changed2);
    ASSERT_TRUE(kbd.GetEngine().GetCurrentUnicode().empty());

    return true;
}
