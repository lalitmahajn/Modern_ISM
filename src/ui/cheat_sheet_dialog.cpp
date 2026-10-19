#include "ui/cheat_sheet_dialog.hpp"
#include <sstream>
#include <vector>
#include <iomanip>

namespace minglish {

namespace {
const wchar_t* CHEAT_SHEET_CLASS = L"ModernISM_CheatSheet";

std::wstring Utf8ToWide(std::string_view utf8) {
    if (utf8.empty()) return L"";
    int needed = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    if (needed <= 0) return L"";
    std::wstring out(needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), &out[0], needed);
    return out;
}

std::wstring U16ToWide(std::u16string_view u16) {
    return std::wstring(reinterpret_cast<const wchar_t*>(u16.data()), u16.size());
}
} // anonymous namespace

std::wstring GenerateCheatSheetText(const FontProfile& profile) {
    std::wostringstream ss;
    bool isUni = profile.IsUnicode();

    ss << L"======================================================================\r\n";
    ss << L"  MODERN ISM - MINGLISH PHONETIC KEYMAP REFERENCE\r\n";
    ss << L"  Active Font Profile : " << Utf8ToWide(profile.name)
       << (isUni ? L"  [Unicode Standard]\r\n" : L"  [Legacy Remington / ANSI]\r\n");
    ss << L"======================================================================\r\n\r\n";

    auto getDirect = [&](const std::u16string& u16) -> std::wstring {
        if (isUni) return L"";
        auto it = profile.directMap.find(u16);
        if (it != profile.directMap.end() && !it->second.empty()) {
            return L" [" + Utf8ToWide(it->second) + L"]";
        }
        return L"";
    };

    auto getHalf = [&](const std::u16string& u16half) -> std::wstring {
        if (isUni) return L"";
        auto it = profile.halfConsonants.find(u16half);
        if (it != profile.halfConsonants.end() && !it->second.empty()) {
            return L" (half: [" + Utf8ToWide(it->second) + L"])";
        }
        return L"";
    };

    // 1. VOWELS & MATRAS
    ss << L"1. VOWELS & MATRAS:\r\n";
    struct VowelEntry {
        const wchar_t* keys;
        std::u16string indep;
        std::u16string matra;
    };

    std::vector<VowelEntry> vowels = {
        {L"a",       u"अ", u""},
        {L"aa / A",  u"आ", u"ा"},
        {L"i",       u"इ", u"ि"},
        {L"ee / I",  u"ई", u"ी"},
        {L"u",       u"उ", u"ु"},
        {L"oo / U",  u"ऊ", u"ू"},
        {L"e",       u"ए", u"े"},
        {L"ai",      u"ऐ", u"ै"},
        {L"o",       u"ओ", u"ो"},
        {L"au / ou", u"औ", u"ौ"},
        {L"Ru",      u"ऋ", u"ृ"},
        {L"c / E",   u"ॲ", u"ॅ"},
        {L"O",       u"ऑ", u"ॉ"}
    };

    for (size_t i = 0; i < vowels.size(); i += 2) {
        auto formatVowel = [&](const VowelEntry& v) -> std::wstring {
            std::wostringstream vss;
            vss << L"   " << std::left << std::setw(9) << v.keys << L" -> " << U16ToWide(v.indep) << getDirect(v.indep);
            if (!v.matra.empty()) {
                vss << L" (matra: " << U16ToWide(v.matra) << getDirect(v.matra) << L")";
            }
            return vss.str();
        };

        std::wstring col1 = formatVowel(vowels[i]);
        if (i + 1 < vowels.size()) {
            if (col1.size() < 36) col1.append(36 - col1.size(), L' ');
            else col1.append(L"  ");
            ss << col1 << formatVowel(vowels[i + 1]) << L"\r\n";
        } else {
            ss << col1 << L"\r\n";
        }
    }
    ss << L"\r\n";

    // 2. CONSONANTS (VARGA GROUPS)
    ss << L"2. CONSONANTS (व्यंजन):\r\n";

    struct ConsEntry {
        const wchar_t* key;
        std::u16string base;
        std::u16string half;
    };

    auto printGroup = [&](const wchar_t* title, const std::vector<ConsEntry>& list) {
        ss << L"   [" << title << L"]\r\n";
        for (size_t i = 0; i < list.size(); i += 2) {
            auto formatCons = [&](const ConsEntry& c) -> std::wstring {
                std::wostringstream css;
                css << L"     " << std::left << std::setw(8) << c.key << L" -> " << U16ToWide(c.base) << getDirect(c.base);
                std::wstring h = getHalf(c.half);
                if (!h.empty()) css << L" " << h;
                return css.str();
            };

            std::wstring col1 = formatCons(list[i]);
            if (i + 1 < list.size()) {
                if (col1.size() < 36) col1.append(36 - col1.size(), L' ');
                else col1.append(L"  ");
                ss << col1 << formatCons(list[i + 1]) << L"\r\n";
            } else {
                ss << col1 << L"\r\n";
            }
        }
        ss << L"\r\n";
    };

    printGroup(L"Velar (क-वर्ग)", {
        {L"k", u"क", u"क्"}, {L"kh", u"ख", u"ख्"}, {L"g", u"ग", u"ग्"}, {L"gh", u"घ", u"घ्"}, {L"ng", u"ङ", u"ङ्"}
    });

    printGroup(L"Palatal (च-वर्ग)", {
        {L"ch", u"च", u"च्"}, {L"chh/Ch", u"छ", u"छ्"}, {L"j", u"ज", u"ज्"}, {L"jh / z", u"झ", u"झ्"}, {L"ny / nj", u"ञ", u"ञ्"}
    });

    printGroup(L"Retroflex (ट-वर्ग)", {
        {L"T", u"ट", u"ट्"}, {L"Th", u"ठ", u"ठ्"}, {L"D", u"ड", u"ड्"}, {L"Dh", u"ढ", u"ढ्"}, {L"N", u"ण", u"ण्"}
    });

    printGroup(L"Dental (त-वर्ग)", {
        {L"t", u"त", u"त्"}, {L"th", u"थ", u"थ्"}, {L"d", u"द", u"द्"}, {L"dh", u"ध", u"ध्"}, {L"n", u"न", u"न्"}
    });

    printGroup(L"Labial (प-वर्ग)", {
        {L"p", u"प", u"प्"}, {L"ph / f", u"फ", u"फ्"}, {L"b", u"ब", u"ब्"}, {L"bh", u"भ", u"भ्"}, {L"m", u"म", u"म्"}
    });

    printGroup(L"Semivowels & Fricatives (अन्तस्थ / ऊष्म)", {
        {L"y", u"य", u"य्"}, {L"r", u"र", u"र्"}, {L"l", u"ल", u"ल्"}, {L"v / w", u"व", u"व्"},
        {L"sh", u"श", u"श्"}, {L"shh / S", u"ष", u"ष्"}, {L"s", u"स", u"स्"}, {L"h", u"ह", u"ह्"}, {L"L / ld", u"ळ", u"ळ्"}
    });

    printGroup(L"Common Conjuncts (संयुक्त व्यंजन)", {
        {L"ksh / x", u"क्ष", u"क्ष्"}, {L"tr", u"त्र", u"त्र्"}, {L"dny/jny", u"ज्ञ", u"ज्ञ्"}, {L"shr", u"श्र", u"श्र्"}
    });

    // 3. MODIFIERS & SPECIALS
    ss << L"3. MODIFIERS & SPECIALS:\r\n";
    ss << L"   M         ->  Anusvara (ं)" << getDirect(u"ं") << L"\r\n";
    ss << L"   H         ->  Visarga (ः)" << getDirect(u"ः") << L"\r\n";
    ss << L"   ~ or /    ->  Explicit Virama / Halant (्)"
       << (isUni ? L"" : L" [" + Utf8ToWide(profile.viramaGlyph) + L"]") << L"\r\n\r\n";

    // 4. R-FORMS & MARATHI SUPPORT
    ss << L"4. 3 FORMS OF 'R' & MARATHI SUPPORT:\r\n";
    ss << L"   1. Reph (रफ़ार):  k-a-r-m = कर्म (Flying Top Hook"
       << (isUni ? L"" : L" glyph: [" + Utf8ToWide(profile.rephGlyph) + L"]") << L")\r\n";
    ss << L"   2. Rakar (रकार):  k-r-a = क्र (Bottom Diagonal Stroke"
       << (isUni ? L"" : L" glyph: [" + Utf8ToWide(profile.rakarGlyph) + L"]") << L")\r\n";
    ss << L"   3. Caret Rakar:   t-r-a = ट्र (Stemless Caret for ट, ठ, ड, ढ, छ"
       << (isUni ? L"" : L" glyph: [" + Utf8ToWide(profile.rakarStemlessGlyph) + L"]") << L")\r\n";
    ss << L"   4. Eyelash Ra:    rh / ry = ऱ्ह / ऱ्या (कुऱ्हाड / वाऱ्याचा)\r\n\r\n";

    // 5. FONT-SPECIFIC LIGATURES
    if (!profile.ligatures.empty()) {
        ss << L"5. FONT-SPECIFIC LIGATURES (Loaded from " << Utf8ToWide(profile.id) << L".json):\r\n";
        int count = 0;
        for (const auto& [u16lig, glyph] : profile.ligatures) {
            std::wostringstream lss;
            lss << L"   " << U16ToWide(u16lig) << L" -> [" << Utf8ToWide(glyph) << L"]";
            std::wstring itemStr = lss.str();
            if (itemStr.size() < 18) itemStr.append(18 - itemStr.size(), L' ');
            ss << itemStr;
            count++;
            if (count % 4 == 0) {
                ss << L"\r\n";
            }
        }
        if (count % 4 != 0) {
            ss << L"\r\n";
        }
        ss << L"\r\n";
    }

    // 6. NUMERALS
    ss << L"6. NUMERALS:\r\n";
    ss << L"   0-9       ->  ०-९ (Devanagari Digits"
       << (isUni ? L"" : L" -> Remington output codes 0-9") << L")\r\n\r\n";

    // 7. GLOBAL HOTKEYS
    ss << L"7. GLOBAL HOTKEYS:\r\n";
    ss << L"   Ctrl + Space      : Toggle Modern ISM Transpiler (ON / OFF)\r\n";
    ss << L"   Ctrl + Shift + F  : Quick Font Switcher Modal (Select active font)\r\n";
    ss << L"   Ctrl + Shift + H  : This Keymap Cheat Sheet\r\n";
    ss << L"   Esc               : Close this window and return focus to active app\r\n";

    return ss.str();
}

CheatSheetDialog* CheatSheetDialog::s_instance = nullptr;

bool CheatSheetDialog::IsOpen() {
    return s_instance != nullptr && s_instance->m_hwnd != nullptr && IsWindow(s_instance->m_hwnd);
}

void CheatSheetDialog::Close() {
    if (s_instance && s_instance->m_hwnd && IsWindow(s_instance->m_hwnd)) {
        DestroyWindow(s_instance->m_hwnd);
    }
}

void CheatSheetDialog::Show(HINSTANCE hInstance, HWND hTargetApp, const FontProfile& activeProfile) {
    if (IsOpen()) {
        s_instance->m_profile = activeProfile;
        std::wstring text = GenerateCheatSheetText(activeProfile);
        SetWindowTextW(s_instance->m_hEdit, text.c_str());
        std::wstring title = L"Modern ISM Keymap Cheat Sheet - " + Utf8ToWide(activeProfile.name) + L" (Press Esc to Close)";
        SetWindowTextW(s_instance->m_hwnd, title.c_str());
        SetForegroundWindow(s_instance->m_hwnd);
        return;
    }

    if (!hInstance) {
        hInstance = GetModuleHandleW(nullptr);
    }

    s_instance = new CheatSheetDialog();
    s_instance->m_hInstance = hInstance;
    s_instance->m_hTargetApp = hTargetApp;
    s_instance->m_profile = activeProfile;

    static bool s_classRegistered = false;
    if (!s_classRegistered) {
        WNDCLASSEXW wc = {sizeof(WNDCLASSEXW)};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CHEAT_SHEET_CLASS;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        RegisterClassExW(&wc);
        s_classRegistered = true;
    }

    int width = 640;
    int height = 560;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - width) / 2;
    int y = (screenH - height) / 2;

    std::wstring title = L"Modern ISM Keymap Cheat Sheet - " + Utf8ToWide(activeProfile.name) + L" (Press Esc to Close)";

    s_instance->m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
        CHEAT_SHEET_CLASS,
        title.c_str(),
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        nullptr, nullptr, hInstance, s_instance
    );

    if (s_instance->m_hwnd) {
        SetForegroundWindow(s_instance->m_hwnd);
    }
}

void CheatSheetDialog::Show(HINSTANCE hInstance, HWND hTargetApp) {
    Show(hInstance, hTargetApp, GetDefaultUnicodeProfile());
}

LRESULT CALLBACK CheatSheetDialog::EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (s_instance) {
        if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
            DestroyWindow(s_instance->m_hwnd);
            return 0;
        }
        return CallWindowProcW(s_instance->m_origEditProc, hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK CheatSheetDialog::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* self = reinterpret_cast<CheatSheetDialog*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));

        self->m_hFont = CreateFontW(
            -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
        );

        RECT rc;
        GetClientRect(hwnd, &rc);
        int editW = rc.right > 24 ? rc.right - 24 : 590;
        int editH = rc.bottom > 24 ? rc.bottom - 24 : 490;

        std::wstring sheetText = GenerateCheatSheetText(self->m_profile);

        self->m_hEdit = CreateWindowExW(
            0, L"EDIT", sheetText.c_str(),
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            12, 12, editW, editH,
            hwnd, nullptr, self->m_hInstance, nullptr
        );
        SendMessageW(self->m_hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(self->m_hFont), TRUE);
        self->m_origEditProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(self->m_hEdit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(EditWndProc))
        );
        SetFocus(self->m_hEdit);

        return 0;
    }

    auto* self = reinterpret_cast<CheatSheetDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_SIZE: {
            if (self && self->m_hEdit) {
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                MoveWindow(self->m_hEdit, 12, 12, w > 24 ? w - 24 : w, h > 24 ? h - 24 : h, TRUE);
            }
            return 0;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        case WM_DESTROY: {
            if (self) {
                HWND target = self->m_hTargetApp;
                if (self->m_hFont) {
                    DeleteObject(self->m_hFont);
                    self->m_hFont = nullptr;
                }
                delete self;
                s_instance = nullptr;
                if (target && IsWindow(target)) {
                    SetForegroundWindow(target);
                }
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace minglish
