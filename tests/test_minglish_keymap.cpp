#include "test_framework.hpp"
#include "engine/minglish_keymap.hpp"

TEST_CASE(TestConsonantLookup) {
    auto m1 = minglish::FindMapping("k");
    ASSERT_TRUE(m1 != nullptr);
    ASSERT_EQ(m1->independent, u"क");
    ASSERT_TRUE(m1->category == minglish::KeyCategory::Consonant);

    auto m2 = minglish::FindMapping("kh");
    ASSERT_TRUE(m2 != nullptr);
    ASSERT_EQ(m2->independent, u"ख");

    auto m3 = minglish::FindMapping("dny");
    ASSERT_TRUE(m3 != nullptr);
    ASSERT_EQ(m3->independent, u"ज्ञ");

    auto m4 = minglish::FindMapping("L");
    ASSERT_TRUE(m4 != nullptr);
    ASSERT_EQ(m4->independent, u"ळ");

    return true;
}

TEST_CASE(TestVowelAndModifierLookup) {
    auto v1 = minglish::FindMapping("i");
    ASSERT_TRUE(v1 != nullptr);
    ASSERT_EQ(v1->dependent, u"ि");
    ASSERT_EQ(v1->independent, u"इ");
    ASSERT_TRUE(v1->category == minglish::KeyCategory::Vowel);

    auto m1 = minglish::FindMapping("M");
    ASSERT_TRUE(m1 != nullptr);
    ASSERT_EQ(m1->dependent, u"ं");
    ASSERT_TRUE(m1->category == minglish::KeyCategory::Modifier);

    auto c1 = minglish::FindMapping("E");
    ASSERT_TRUE(c1 != nullptr);
    ASSERT_EQ(c1->dependent, u"ॅ");

    auto c2 = minglish::FindMapping("O");
    ASSERT_TRUE(c2 != nullptr);
    ASSERT_EQ(c2->dependent, u"ॉ");

    return true;
}

TEST_CASE(TestHasPrefix) {
    ASSERT_TRUE(minglish::HasPrefix("k"));
    ASSERT_TRUE(minglish::HasPrefix("kh"));
    ASSERT_TRUE(minglish::HasPrefix("dn"));
    ASSERT_TRUE(minglish::HasPrefix("dny"));
    ASSERT_TRUE(!minglish::HasPrefix("xyz"));
    return true;
}
