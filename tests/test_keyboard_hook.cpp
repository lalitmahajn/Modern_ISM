#include "test_framework.hpp"
#include "hook/keyboard_hook.hpp"

TEST_CASE(TestKeyboardHookInjectedBypass) {
    auto& hook = minglish::KeyboardHookManager::Instance();
    hook.SetEngineEnabled(true);

    // If LLKHF_INJECTED is set, must NOT intercept (return false / pass through)
    bool handled = hook.ProcessKeyEvent('K', false, LLKHF_INJECTED, false, false, false, 'k');
    ASSERT_TRUE(!handled);

    return true;
}

TEST_CASE(TestKeyboardHookModifierBypass) {
    auto& hook = minglish::KeyboardHookManager::Instance();
    hook.SetEngineEnabled(true);

    // If Ctrl is down (e.g. Ctrl+C), must bypass and flush
    bool handled = hook.ProcessKeyEvent('C', false, 0, true, false, false, 'c');
    ASSERT_TRUE(!handled);
    ASSERT_EQ(hook.GetEngine().GetPreviouslyInjected(), "");

    return true;
}

TEST_CASE(TestKeyboardHookBoundaryKeyReset) {
    auto& hook = minglish::KeyboardHookManager::Instance();
    hook.SetEngineEnabled(true);

    // Type 'k'
    hook.ProcessKeyEvent('K', false, 0, false, false, false, 'k');
    ASSERT_TRUE(!hook.GetEngine().GetCurrentUnicode().empty());

    // Space key passes through and resets buffer
    bool handledSpace = hook.ProcessKeyEvent(VK_SPACE, false, 0, false, false, false, ' ');
    ASSERT_TRUE(!handledSpace);
    ASSERT_TRUE(hook.GetEngine().GetCurrentUnicode().empty());

    return true;
}

TEST_CASE(TestKeyboardHookPrintableIntercept) {
    auto& hook = minglish::KeyboardHookManager::Instance();
    hook.SetEngineEnabled(true);
    hook.GetEngine().Reset();

    // Normal typing: printable character 'k' is intercepted (returns true)
    bool handled = hook.ProcessKeyEvent('K', false, 0, false, false, false, 'k');
    ASSERT_TRUE(handled);
    ASSERT_EQ(hook.GetEngine().GetCurrentUnicode(), u"क");

    // Physical Backspace when buffer is active: intercepted (returns true)
    bool handledBS = hook.ProcessKeyEvent(VK_BACK, false, 0, false, false, false, 0);
    ASSERT_TRUE(handledBS);
    ASSERT_TRUE(hook.GetEngine().GetCurrentUnicode().empty());

    // Physical Backspace when buffer is empty: passed through (returns false)
    bool handledEmptyBS = hook.ProcessKeyEvent(VK_BACK, false, 0, false, false, false, 0);
    ASSERT_TRUE(!handledEmptyBS);

    return true;
}
