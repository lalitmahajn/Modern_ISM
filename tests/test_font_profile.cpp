#include "test_framework.hpp"
#include "engine/font_profile.hpp"
#include "ui/cheat_sheet_dialog.hpp"

TEST_CASE(TestLoadKrutiDevProfile) {
    auto profile = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");
    ASSERT_EQ(profile.id, "kruti_dev_010");
    ASSERT_TRUE(profile.features.chhotiEePreBase);
    ASSERT_TRUE(profile.features.rephPostBase);
    ASSERT_EQ(profile.chhotiEeGlyph, "f");
    ASSERT_EQ(profile.rephGlyph, "Z");

    auto it = profile.directMap.find(u"क");
    ASSERT_TRUE(it != profile.directMap.end());
    ASSERT_EQ(it->second, "d");

    auto itHalf = profile.halfConsonants.find(u"क्");
    ASSERT_TRUE(itHalf != profile.halfConsonants.end());
    ASSERT_EQ(itHalf->second, "D");

    return true;
}

TEST_CASE(TestDefaultUnicodeProfile) {
    auto unicode = minglish::GetDefaultUnicodeProfile();
    ASSERT_EQ(unicode.id, "unicode_devanagari");
    ASSERT_TRUE(!unicode.features.chhotiEePreBase);
    ASSERT_TRUE(!unicode.features.rephPostBase);
    return true;
}

TEST_CASE(TestUnicodeCheatSheetText) {
    auto unicode = minglish::GetDefaultUnicodeProfile();
    std::wstring text = minglish::GenerateCheatSheetText(unicode);
    ASSERT_TRUE(text.find(L"Unicode Standard") != std::wstring::npos);
    std::u16string kU16 = u"क";
    std::wstring kW(reinterpret_cast<const wchar_t*>(kU16.data()), kU16.size());
    ASSERT_TRUE(text.find(kW) != std::wstring::npos);
    std::u16string aU16 = u"अ";
    std::wstring aW(reinterpret_cast<const wchar_t*>(aU16.data()), aU16.size());
    ASSERT_TRUE(text.find(aW) != std::wstring::npos);
    ASSERT_TRUE(text.find(L"Ctrl + Shift + H") != std::wstring::npos);
    return true;
}

TEST_CASE(TestKrutiDevCheatSheetText) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");
    std::wstring text = minglish::GenerateCheatSheetText(kruti);
    ASSERT_TRUE(text.find(L"Kruti Dev 010") != std::wstring::npos);
    ASSERT_TRUE(text.find(L"Legacy Remington / ANSI") != std::wstring::npos);
    ASSERT_TRUE(text.find(L"k        -> क [d]") != std::wstring::npos);
    ASSERT_TRUE(text.find(L"(half: [D])") != std::wstring::npos);
    ASSERT_TRUE(text.find(L"FONT-SPECIFIC LIGATURES") != std::wstring::npos);
    ASSERT_TRUE(text.find(L"ज्ञ -> [K]") != std::wstring::npos);
    return true;
}
