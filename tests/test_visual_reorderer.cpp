#include "test_framework.hpp"
#include "engine/font_profile.hpp"
#include "engine/visual_reorderer.hpp"

TEST_CASE(TestChhotiEeReordering) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");

    // कि -> fd
    std::string s1 = minglish::TranspileToTargetFont(u"कि", kruti);
    ASSERT_EQ(s1, "fd");

    // स्मि -> fLe (f precedes half-s 'L' and base-m 'e')
    std::string s2 = minglish::TranspileToTargetFont(u"स्मि", kruti);
    ASSERT_EQ(s2, "fLe");

    return true;
}

TEST_CASE(TestRephPostPositioning) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");

    // कर्म -> deZ (d = क, e = म, Z = Reph)
    std::string s1 = minglish::TranspileToTargetFont(u"कर्म", kruti);
    ASSERT_EQ(s1, "deZ");

    // कर्मी -> dehZ (d = क, e = म, h = ी, Z = Reph)
    std::string s2 = minglish::TranspileToTargetFont(u"कर्मी", kruti);
    ASSERT_EQ(s2, "dehZ");

    return true;
}

TEST_CASE(TestUnicodeProfilePassthrough) {
    auto unicode = minglish::GetDefaultUnicodeProfile();

    std::string s1 = minglish::TranspileToTargetFont(u"कि", unicode);
    ASSERT_EQ(s1, "कि");

    std::string s2 = minglish::TranspileToTargetFont(u"कर्म", unicode);
    ASSERT_EQ(s2, "कर्म");

    return true;
}

TEST_CASE(TestKrutiDevLigatures) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");

    // ज्ञान -> Kku (K for ज्ञ, k for ा, u for न)
    std::string s1 = minglish::TranspileToTargetFont(u"ज्ञान", kruti);
    ASSERT_EQ(s1, "Kku");

    // क्षण -> {k.k ({k for क्ष, .k for ण)
    std::string s2 = minglish::TranspileToTargetFont(u"क्षण", kruti);
    ASSERT_EQ(s2, "{k.k");

    // त्रिशूल -> f='kwy (f= for त्रि, 'kw for शू, y for ल)
    std::string s3 = minglish::TranspileToTargetFont(u"त्रिशूल", kruti);
    ASSERT_EQ(s3, "f='kwy");

    return true;
}

TEST_CASE(TestKrutiDevRakar) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");

    // प्र -> iz (i for प, z for Rakar)
    std::string s1 = minglish::TranspileToTargetFont(u"प्र", kruti);
    ASSERT_EQ(s1, "iz");

    // क्रम -> dze (d for क, z for Rakar, e for म)
    std::string s2 = minglish::TranspileToTargetFont(u"क्रम", kruti);
    ASSERT_EQ(s2, "dze");

    return true;
}

