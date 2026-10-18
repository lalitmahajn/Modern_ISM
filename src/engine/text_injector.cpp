#include "engine/text_injector.hpp"

namespace minglish {

std::vector<INPUT> BuildSendInputEvents(const InjectionDiffU16& diff) {
    std::vector<INPUT> inputs;
    inputs.reserve(diff.backspaceCount * 2 + diff.textToInject.length() * 2);

    // 1. Queue all backspaces (down + up)
    for (size_t b = 0; b < diff.backspaceCount; ++b) {
        INPUT down{};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = VK_BACK;
        down.ki.dwFlags = 0;
        inputs.push_back(down);

        INPUT up{};
        up.type = INPUT_KEYBOARD;
        up.ki.wVk = VK_BACK;
        up.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(up);
    }

    // 2. Queue all characters to inject (down + up)
    for (char16_t ch : diff.textToInject) {
        INPUT down{};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = 0;
        down.ki.wScan = static_cast<WORD>(ch);
        down.ki.dwFlags = KEYEVENTF_UNICODE;
        inputs.push_back(down);

        INPUT up{};
        up.type = INPUT_KEYBOARD;
        up.ki.wVk = 0;
        up.ki.wScan = static_cast<WORD>(ch);
        up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        inputs.push_back(up);
    }

    return inputs;
}

bool Win32SendInputInjector::Inject(const InjectionDiffU16& diff) {
    std::vector<INPUT> events = BuildSendInputEvents(diff);
    if (events.empty()) {
        return true;
    }
    // Single atomic SendInput call to eliminate visual flicker and race conditions
    UINT sent = ::SendInput(static_cast<UINT>(events.size()), events.data(), sizeof(INPUT));
    return sent == events.size();
}

bool ClipboardFallbackInjector::Inject(const InjectionDiffU16& diff) {
    if (diff.backspaceCount > 0) {
        std::vector<INPUT> bsEvents;
        bsEvents.reserve(diff.backspaceCount * 2);
        for (size_t b = 0; b < diff.backspaceCount; ++b) {
            INPUT down{}; down.type = INPUT_KEYBOARD; down.ki.wVk = VK_BACK; down.ki.dwFlags = 0;
            bsEvents.push_back(down);
            INPUT up{}; up.type = INPUT_KEYBOARD; up.ki.wVk = VK_BACK; up.ki.dwFlags = KEYEVENTF_KEYUP;
            bsEvents.push_back(up);
        }
        ::SendInput(static_cast<UINT>(bsEvents.size()), bsEvents.data(), sizeof(INPUT));
    }

    if (!diff.textToInject.empty()) {
        if (::OpenClipboard(nullptr)) {
            ::EmptyClipboard();
            size_t bytes = (diff.textToInject.length() + 1) * sizeof(char16_t);
            HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, bytes);
            if (hMem) {
                void* pMem = ::GlobalLock(hMem);
                if (pMem) {
                    memcpy(pMem, diff.textToInject.data(), bytes);
                    ::GlobalUnlock(hMem);
                    ::SetClipboardData(CF_UNICODETEXT, hMem);
                }
            }
            ::CloseClipboard();

            INPUT paste[4]{};
            paste[0].type = INPUT_KEYBOARD; paste[0].ki.wVk = VK_CONTROL;
            paste[1].type = INPUT_KEYBOARD; paste[1].ki.wVk = 'V';
            paste[2].type = INPUT_KEYBOARD; paste[2].ki.wVk = 'V'; paste[2].ki.dwFlags = KEYEVENTF_KEYUP;
            paste[3].type = INPUT_KEYBOARD; paste[3].ki.wVk = VK_CONTROL; paste[3].ki.dwFlags = KEYEVENTF_KEYUP;
            ::SendInput(4, paste, sizeof(INPUT));
        }
    }
    return true;
}

} // namespace minglish
