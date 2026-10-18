#include "test_framework.hpp"
#include "engine/word_buffer.hpp"

TEST_CASE(TestMultiKeyUpgrade) {
    minglish::WordBuffer buf;
    buf.PushKey('k');
    ASSERT_EQ(buf.ToUnicode(), u"क");

    buf.PushKey('h');
    ASSERT_EQ(buf.ToUnicode(), u"ख");

    buf.PopKey();
    ASSERT_EQ(buf.ToUnicode(), u"क");
    return true;
}

TEST_CASE(TestVowelMatraAttachment) {
    minglish::WordBuffer buf;
    buf.PushKey('k');
    buf.PushKey('i');
    ASSERT_EQ(buf.ToUnicode(), u"कि");

    buf.Clear();
    buf.PushKey('k');
    buf.PushKey('a'); // inherent vowel confirmation
    ASSERT_EQ(buf.ToUnicode(), u"क");
    buf.PushKey('a'); // aa -> A matra
    ASSERT_EQ(buf.ToUnicode(), u"का");
    return true;
}

TEST_CASE(TestRephAndJodakshar) {
    minglish::WordBuffer buf;
    // k-a-r-m -> कर्म
    buf.PushKey('k');
    buf.PushKey('a');
    buf.PushKey('r');
    buf.PushKey('m');
    ASSERT_EQ(buf.ToUnicode(), u"कर्म");

    // s-m-i -> स्मि
    buf.Clear();
    buf.PushKey('s');
    buf.PushKey('m');
    buf.PushKey('i');
    ASSERT_EQ(buf.ToUnicode(), u"स्मि");

    // d-n-y-a-a-n -> ज्ञान
    buf.Clear();
    buf.PushKey('d');
    buf.PushKey('n');
    buf.PushKey('y');
    buf.PushKey('a');
    buf.PushKey('a');
    buf.PushKey('n');
    ASSERT_EQ(buf.ToUnicode(), u"ज्ञान");

    // b-h-a-a-g-y-a -> भाग्य
    buf.Clear();
    buf.PushKey('b');
    buf.PushKey('h');
    buf.PushKey('a');
    buf.PushKey('a');
    buf.PushKey('g');
    buf.PushKey('y');
    buf.PushKey('a');
    ASSERT_EQ(buf.ToUnicode(), u"भाग्य");

    // k-a-M -> कं
    buf.Clear();
    buf.PushKey('k');
    buf.PushKey('a');
    buf.PushKey('M');
    ASSERT_EQ(buf.ToUnicode(), u"कं");

    return true;
}

TEST_CASE(TestInherentConsonantWithHAndRu) {
    minglish::WordBuffer buf;
    // maha -> मह (m + a + h + a), NOT मःअ
    buf.PushKey('m');
    buf.PushKey('a');
    buf.PushKey('h');
    buf.PushKey('a');
    ASSERT_EQ(buf.ToUnicode(), u"मह");

    // guru -> गुरु (g + u + r + u), NOT गुऋ
    buf.Clear();
    buf.PushKey('g');
    buf.PushKey('u');
    buf.PushKey('r');
    buf.PushKey('u');
    ASSERT_EQ(buf.ToUnicode(), u"गुरु");

    // Explicit Visarga with H: namaH -> नमः
    buf.Clear();
    buf.PushKey('n');
    buf.PushKey('a');
    buf.PushKey('m');
    buf.PushKey('a');
    buf.PushKey('H');
    ASSERT_EQ(buf.ToUnicode(), u"नमः");

    // Vocalic R with Ru: Ru -> ऋ
    buf.Clear();
    buf.PushKey('R');
    buf.PushKey('u');
    ASSERT_EQ(buf.ToUnicode(), u"ऋ");

    return true;
}

