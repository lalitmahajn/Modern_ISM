#include "test_framework.hpp"
#include "engine/text_injector.hpp"

TEST_CASE(TestBuildSendInputEventsBatching) {
    minglish::InjectionDiffU16 diff;
    diff.backspaceCount = 2;
    diff.textToInject = u"fd";

    std::vector<INPUT> events = minglish::BuildSendInputEvents(diff);

    // 2 backspaces * 2 events (down+up) + 2 chars * 2 events (down+up) = 8 events
    ASSERT_EQ(events.size(), static_cast<size_t>(8));

    // First event: VK_BACK keydown
    ASSERT_EQ(events[0].type, static_cast<DWORD>(INPUT_KEYBOARD));
    ASSERT_EQ(events[0].ki.wVk, static_cast<WORD>(VK_BACK));
    ASSERT_EQ(events[0].ki.dwFlags, static_cast<DWORD>(0));

    // Second event: VK_BACK keyup
    ASSERT_EQ(events[1].type, static_cast<DWORD>(INPUT_KEYBOARD));
    ASSERT_EQ(events[1].ki.wVk, static_cast<WORD>(VK_BACK));
    ASSERT_EQ(events[1].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));

    // Fifth event: 'f' keydown with KEYEVENTF_UNICODE
    ASSERT_EQ(events[4].type, static_cast<DWORD>(INPUT_KEYBOARD));
    ASSERT_EQ(events[4].ki.wScan, static_cast<WORD>(u'f'));
    ASSERT_EQ(events[4].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE));

    // Sixth event: 'f' keyup with KEYEVENTF_UNICODE | KEYEVENTF_KEYUP
    ASSERT_EQ(events[5].type, static_cast<DWORD>(INPUT_KEYBOARD));
    ASSERT_EQ(events[5].ki.wScan, static_cast<WORD>(u'f'));
    ASSERT_EQ(events[5].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE | KEYEVENTF_KEYUP));

    return true;
}
