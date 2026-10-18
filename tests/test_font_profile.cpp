#include "test_framework.hpp"
#include "engine/font_profile.hpp"

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
