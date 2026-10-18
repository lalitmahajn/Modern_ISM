#include "test_framework.hpp"
#include "engine/transpiler_engine.hpp"
#include "engine/font_profile.hpp"

TEST_CASE(TestTranspilerEndToEndMultiKeyUpgrade) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");
    minglish::TranspilerEngine engine;
    engine.SetFontProfile(kruti);

    // Type 'k' -> 'd' (क)
    auto d1 = engine.OnChar('k');
    ASSERT_EQ(d1.backspaceCount, 0);
    ASSERT_EQ(d1.textToInject, "d");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "d");

    // Type 'h' -> upgrades to '[k' (ख)
    auto d2 = engine.OnChar('h');
    ASSERT_EQ(d2.backspaceCount, 1);
    ASSERT_EQ(d2.textToInject, "[k");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "[k");

    // Physical Backspace -> rolls back to 'd' (क)
    auto d3 = engine.OnBackspace();
    ASSERT_EQ(d3.backspaceCount, 2);
    ASSERT_EQ(d3.textToInject, "d");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "d");

    return true;
}

TEST_CASE(TestTranspilerEndToEndChhotiEeReordering) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");
    minglish::TranspilerEngine engine;
    engine.SetFontProfile(kruti);

    // Type 'k' -> 'd' (क)
    auto d1 = engine.OnChar('k');
    ASSERT_EQ(d1.backspaceCount, 0);
    ASSERT_EQ(d1.textToInject, "d");

    // Type 'i' -> 'fd' (कि)
    auto d2 = engine.OnChar('i');
    ASSERT_EQ(d2.backspaceCount, 1);
    ASSERT_EQ(d2.textToInject, "fd");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "fd");

    return true;
}

TEST_CASE(TestTranspilerEndToEndRephPositioning) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");
    minglish::TranspilerEngine engine;
    engine.SetFontProfile(kruti);

    // Type 'k' -> 'd' (क)
    engine.OnChar('k');
    ASSERT_EQ(engine.GetPreviouslyInjected(), "d");

    // Type 'a' -> confirms 'क', screen stays 'd'
    auto d2 = engine.OnChar('a');
    ASSERT_EQ(d2.backspaceCount, 0);
    ASSERT_EQ(d2.textToInject, "");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "d");

    // Type 'r' -> 'j' (र), screen becomes 'dj'
    auto d3 = engine.OnChar('r');
    ASSERT_EQ(d3.backspaceCount, 0);
    ASSERT_EQ(d3.textToInject, "j");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "dj");

    // Type 'm' -> karm (कर्म) -> 'deZ' (d=क, e=म, Z=Reph)
    // Common prefix of 'dj' and 'deZ' is 'd'.
    // Backspaces: 1 (erases 'j'), inject: 'eZ'
    auto d4 = engine.OnChar('m');
    ASSERT_EQ(d4.backspaceCount, 1);
    ASSERT_EQ(d4.textToInject, "eZ");
    ASSERT_EQ(engine.GetPreviouslyInjected(), "deZ");

    return true;
}

TEST_CASE(TestTranspilerEndToEndUnicode) {
    auto unicode = minglish::GetDefaultUnicodeProfile();
    minglish::TranspilerEngine engine;
    engine.SetFontProfile(unicode);

    // Type 'k'
    auto d1 = engine.OnChar('k');
    ASSERT_EQ(d1.textToInject, "क");

    // Type 'i' -> 'कि'
    auto d2 = engine.OnChar('i');
    ASSERT_EQ(engine.GetPreviouslyInjected(), "कि");

    // Reset on word boundary
    engine.Reset();
    ASSERT_EQ(engine.GetPreviouslyInjected(), "");

    return true;
}
