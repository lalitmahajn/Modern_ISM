#include "test_framework.hpp"
#include "engine/differential_injector.hpp"

TEST_CASE(TestDifferentialPrefixMatch) {
    // Case 1: Appending character with shared prefix ("d" -> "dk")
    auto diff1 = minglish::ComputeDifferential("d", "dk");
    ASSERT_EQ(diff1.backspaceCount, 0);
    ASSERT_EQ(diff1.textToInject, "k");

    // Case 2: Multi-key upgrade replacing previous ("d" -> "[k")
    auto diff2 = minglish::ComputeDifferential("d", "[k");
    ASSERT_EQ(diff2.backspaceCount, 1);
    ASSERT_EQ(diff2.textToInject, "[k");

    // Case 3: Reordering prepending glyph ("d" -> "fd")
    auto diff3 = minglish::ComputeDifferential("d", "fd");
    ASSERT_EQ(diff3.backspaceCount, 1);
    ASSERT_EQ(diff3.textToInject, "fd");

    // Case 4: Initial keystroke from empty
    auto diff4 = minglish::ComputeDifferential("", "d");
    ASSERT_EQ(diff4.backspaceCount, 0);
    ASSERT_EQ(diff4.textToInject, "d");

    // Case 5: Appending Reph to existing cluster ("de" -> "deZ")
    auto diff5 = minglish::ComputeDifferential("de", "deZ");
    ASSERT_EQ(diff5.backspaceCount, 0);
    ASSERT_EQ(diff5.textToInject, "Z");

    // Case 6: Backspacing Reph ("deZ" -> "de")
    auto diff6 = minglish::ComputeDifferential("deZ", "de");
    ASSERT_EQ(diff6.backspaceCount, 1);
    ASSERT_EQ(diff6.textToInject, "");

    return true;
}

TEST_CASE(TestUnicodeDifferentialIntegrity) {
    // Upgrading क (U+0915) to ख (U+0916) in Unicode mode:
    // Must emit exactly 1 backspace and the full character u"ख", NOT raw byte slices!
    auto diff1 = minglish::ComputeDifferentialU16(u"क", u"ख");
    ASSERT_EQ(diff1.backspaceCount, 1);
    ASSERT_EQ(diff1.textToInject, u"ख");

    // Appending matra: क -> कि (U+0915 + U+093F)
    // Common prefix is u"क". Must emit 0 backspaces and u"ि"
    auto diff2 = minglish::ComputeDifferentialU16(u"क", u"कि");
    ASSERT_EQ(diff2.backspaceCount, 0);
    ASSERT_EQ(diff2.textToInject, u"ि");

    // Erasing matra: कि -> क
    // Must emit 1 backspace and empty inject string
    auto diff3 = minglish::ComputeDifferentialU16(u"कि", u"क");
    ASSERT_EQ(diff3.backspaceCount, 1);
    ASSERT_EQ(diff3.textToInject, u"");

    return true;
}

