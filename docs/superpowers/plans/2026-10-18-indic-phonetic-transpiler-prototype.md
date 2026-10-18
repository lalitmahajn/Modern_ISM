# Universal Indic Phonetic Transpiler ("Minglish" Engine) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the high-performance Core Transpiler and Differential Backspace Engine (Stages 1–4) for the Universal Indic Phonetic Transpiler ("Minglish" Engine for Windows) in Modern C++20 with automated verification of multi-key upgrading, Chhoti Ee visual reordering, and Reph post-positioning.

**Architecture:** A deterministic 4-stage pipeline translating user keystrokes into Unicode Devanagari, performing linguistic akshara visual reordering for target font encodings (Unicode and Kruti Dev 010), and computing minimal differential backspaces and injected UTF-16 code units.

**Tech Stack:** C++20, CMake 3.20+, MinGW-w64 (GCC 16.1 UCRT), native C++ unit test runner.

**Spec:** [docs/superpowers/specs/2026-10-18-indic-phonetic-transpiler-design.md](file:///d:/Learning/Modern%20ISM/docs/superpowers/specs/2026-10-18-indic-phonetic-transpiler-design.md)

## Global Constraints
- Target platform: Windows 10/11 x64.
- Zero external runtime DLL dependencies beyond Windows UCRT.
- String representations: Canonical Unicode Devanagari stored in `std::u16string` / `std::string` (UTF-8).
- Legacy font mapping: Configurable via external JSON profiles without hardcoded font tables in C++.
- Latency constraint: Stage 1–4 transpilation per keystroke must execute in < 1 millisecond.

## Review Focus
1. **Multi-key upgrade rollbacks:** Typing `k` (`क`), then `h` (`ख`), then user presses Backspace should cleanly roll back to `क`.
2. **Pre-base matra in complex conjuncts:** Typing `s` + `m` + `i` in Kruti Dev profile must place `f` (Chhoti Ee) before the entire half-cluster (`fLe`), not between `L` and `e`.
3. **Reph + Trailing Matra interactions:** Typing `k-a-r-m-ee` in Kruti Dev must place `Z` (Reph) after both consonant `e` and matra `h` (`dehZ`).
4. **Repeated character transitions:** Typing `k` + `a` (confirms `क`), typing another `a` must upgrade to `का` (`ka` -> `क`, `kaa` -> `का`).
5. **Differential backspace prefix match:** When target string shares prefix (e.g., `क` [d] to `का` [dk]), differential injector must emit 0 backspaces and append only `k`, avoiding screen flicker.

---

### Task 1: CMake Setup, Project Scaffolding & Native Test Harness

**Files:**
- Create: `CMakeLists.txt`
- Create: `tests/CMakeLists.txt`
- Create: `tests/test_framework.hpp`
- Create: `tests/test_main.cpp`

**Interfaces:**
- Consumes: Standard C++20 library
- Produces: `minglish_core` static library target and `minglish_tests` executable target runnable via `ctest` or direct invocation.

- [ ] **Step 1: Write the initial test framework and failing dummy test in `tests/test_framework.hpp` and `tests/test_main.cpp`**

```cpp
// tests/test_framework.hpp
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>

struct TestCase {
    std::string name;
    std::function<bool()> test;
};

inline std::vector<TestCase>& GetTestCases() {
    static std::vector<TestCase> cases;
    return cases;
}

#define TEST_CASE(name) \
    bool name(); \
    static bool reg_##name = []() { \
        GetTestCases().push_back({#name, name}); \
        return true; \
    }(); \
    bool name()

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        std::cerr << "Assertion failed at line " << __LINE__ \
                  << ": expected [" << (expected) << "] got [" << (actual) << "]\n"; \
        return false; \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed at line " << __LINE__ << ": expected true\n"; \
        return false; \
    }
```

```cpp
// tests/test_main.cpp
#include "test_framework.hpp"

TEST_CASE(TestDummy) {
    ASSERT_EQ(1 + 1, 2);
    return true;
}

int main() {
    int failed = 0;
    for (const auto& tc : GetTestCases()) {
        std::cout << "[RUN ] " << tc.name << "\n";
        if (tc.test()) {
            std::cout << "[PASS] " << tc.name << "\n";
        } else {
            std::cout << "[FAIL] " << tc.name << "\n";
            failed++;
        }
    }
    return failed == 0 ? 0 : 1;
}
```

- [ ] **Step 2: Create root `CMakeLists.txt` and `tests/CMakeLists.txt`**

Root `CMakeLists.txt` configuration defining C++20 standard, include directories (`include/`), library target `minglish_core`, and subdirectories.

- [ ] **Step 3: Run CMake configuration and test build to verify test harness runs**

Run: `cmake -B build -G "MinGW Makefiles"` then `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: `[PASS] TestDummy` with 100% tests passing.

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt tests/test_framework.hpp tests/test_main.cpp
git commit -m "chore: setup CMake build and native test harness"
```

---

### Task 2: Minglish Keymap Lookup & Character Classifier

**Files:**
- Create: `include/engine/minglish_keymap.hpp`
- Create: `src/engine/minglish_keymap.cpp`
- Test: `tests/test_minglish_keymap.cpp`

**Interfaces:**
- Consumes: C++ stdlib (`string_view`, `u16string`, `unordered_map`)
- Produces:
  ```cpp
  namespace minglish {
      enum class KeyCategory {
          Consonant,
          Vowel,
          Modifier, // Anusvara, Visarga, Chandra
          Virama,
          Unknown
      };

      struct KeyMapping {
          std::u16string independent; // e.g. U"अ", U"क"
          std::u16string dependent;   // e.g. matra U"ा", half consonant U"क्"
          KeyCategory category;
      };

      const KeyMapping* FindMapping(std::string_view keySeq);
      bool HasPrefix(std::string_view prefix);
  }
  ```

- [ ] **Step 1: Write failing unit tests for keymap lookup in `tests/test_minglish_keymap.cpp`**

```cpp
#include "test_framework.hpp"
#include "engine/minglish_keymap.hpp"

TEST_CASE(TestConsonantLookup) {
    auto m1 = minglish::FindMapping("k");
    ASSERT_TRUE(m1 != nullptr);
    ASSERT_EQ(m1->independent, u"क");
    ASSERT_EQ(m1->category == minglish::KeyCategory::Consonant, true);

    auto m2 = minglish::FindMapping("kh");
    ASSERT_TRUE(m2 != nullptr);
    ASSERT_EQ(m2->independent, u"ख");

    auto m3 = minglish::FindMapping("dny");
    ASSERT_TRUE(m3 != nullptr);
    ASSERT_EQ(m3->independent, u"ज्ञ");

    return true;
}

TEST_CASE(TestVowelAndModifierLookup) {
    auto v1 = minglish::FindMapping("i");
    ASSERT_TRUE(v1 != nullptr);
    ASSERT_EQ(v1->dependent, u"ि");

    auto m1 = minglish::FindMapping("M");
    ASSERT_TRUE(m1 != nullptr);
    ASSERT_EQ(m1->dependent, u"ं");

    auto c1 = minglish::FindMapping("E");
    ASSERT_TRUE(c1 != nullptr);
    ASSERT_EQ(c1->dependent, u"ॅ");

    return true;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`
Expected: Compilation failure due to missing `minglish_keymap.hpp` and implementation.

- [ ] **Step 3: Implement `include/engine/minglish_keymap.hpp` and `src/engine/minglish_keymap.cpp`**

Implement the complete static trie/hash map covering all consonants, vowels, and modifiers specified in Section 5 of the design doc.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: `[PASS] TestConsonantLookup`, `[PASS] TestVowelAndModifierLookup`.

- [ ] **Step 5: Commit**

```bash
git add include/engine/minglish_keymap.hpp src/engine/minglish_keymap.cpp tests/test_minglish_keymap.cpp
git commit -m "feat(engine): implement Minglish phonetic keymap lookup"
```

---

### Task 3: Word Buffer & Indic State Machine (Stage 2)

**Files:**
- Create: `include/engine/word_buffer.hpp`
- Create: `src/engine/word_buffer.cpp`
- Test: `tests/test_word_buffer.cpp`

**Interfaces:**
- Consumes: `minglish_keymap.hpp`
- Produces:
  ```cpp
  namespace minglish {
      class WordBuffer {
      public:
          void PushKey(char c);
          bool PopKey(); // returns true if char was popped, false if buffer was empty
          void Clear();
          bool IsEmpty() const;
          std::string GetRawInput() const;
          std::u16string ToUnicode() const;
      };
  }
  ```

- [ ] **Step 1: Write failing unit tests in `tests/test_word_buffer.cpp`**

```cpp
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
    return true;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`
Expected: Compilation failure due to missing `word_buffer.hpp`.

- [ ] **Step 3: Implement `include/engine/word_buffer.hpp` and `src/engine/word_buffer.cpp`**

Implement greedy tokenization, multi-key upgrades, inherent `a` confirmation, implicit Virama for consonant-consonant clusters, and pre-consonant `r` into Reph (`र` + `्`).

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: All tests pass.

- [ ] **Step 5: Commit**

```bash
git add include/engine/word_buffer.hpp src/engine/word_buffer.cpp tests/test_word_buffer.cpp
git commit -m "feat(engine): implement WordBuffer Indic state machine"
```

---

### Task 4: Font Profile Schema & JSON Loader (Stage 3 Foundation)

**Files:**
- Create: `config/fonts/unicode_devanagari.json`
- Create: `config/fonts/kruti_dev_010.json`
- Create: `include/engine/font_profile.hpp`
- Create: `src/engine/font_profile.cpp`
- Test: `tests/test_font_profile.cpp`

**Interfaces:**
- Consumes: C++ stdlib, lightweight JSON parsing
- Produces:
  ```cpp
  namespace minglish {
      struct FontFeatures {
          bool chhotiEePreBase = false;
          bool rephPostBase = false;
          bool hasHalfConsonants = false;
      };

      struct FontProfile {
          std::string id;
          std::string name;
          FontFeatures features;
          std::unordered_map<std::u16string, std::string> directMap;
          std::unordered_map<std::u16string, std::string> halfConsonants;
          std::unordered_map<std::u16string, std::string> ligatures;
          std::string chhotiEeGlyph;
          std::string rephGlyph;
          std::string viramaGlyph;
          std::string rakarGlyph;
      };

      FontProfile LoadFontProfileFromJson(std::string_view jsonContent);
      FontProfile LoadFontProfileFromFile(std::string_view filePath);
      FontProfile GetDefaultUnicodeProfile();
  }
  ```

- [ ] **Step 1: Write failing unit test in `tests/test_font_profile.cpp`**

```cpp
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
    return true;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`
Expected: Fail to find `font_profile.hpp` and JSON files.

- [ ] **Step 3: Create JSON profile files and implement `font_profile.hpp` and `font_profile.cpp`**

Write `config/fonts/unicode_devanagari.json`, `config/fonts/kruti_dev_010.json`, and the compact JSON parser in `font_profile.cpp`.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: `[PASS] TestLoadKrutiDevProfile`.

- [ ] **Step 5: Commit**

```bash
git add config/fonts/ include/engine/font_profile.hpp src/engine/font_profile.cpp tests/test_font_profile.cpp
git commit -m "feat(engine): add font profile schema and Kruti Dev 010 configuration"
```

---

### Task 5: Akshara Cluster Parser & Visual Reorderer (Stage 3)

**Files:**
- Create: `include/engine/visual_reorderer.hpp`
- Create: `src/engine/visual_reorderer.cpp`
- Test: `tests/test_visual_reorderer.cpp`

**Interfaces:**
- Consumes: `font_profile.hpp`
- Produces:
  ```cpp
  namespace minglish {
      struct AksharaCluster {
          std::u16string halfConsonants; // e.g. u"स्म्"
          char16_t baseConsonant = 0;   // e.g. u'म'
          char16_t matra = 0;           // e.g. u'ि' or u'ी'
          bool hasReph = false;         // true if cluster starts with 'र्'
          std::u16string modifiers;     // e.g. Anusvara u"ं"
      };

      std::vector<AksharaCluster> ParseAksharas(const std::u16string& unicodeWord);
      std::string TranspileToTargetFont(const std::u16string& unicodeWord, const FontProfile& profile);
  }
  ```

- [ ] **Step 1: Write failing unit tests for visual reordering in `tests/test_visual_reorderer.cpp`**

```cpp
#include "test_framework.hpp"
#include "engine/font_profile.hpp"
#include "engine/visual_reorderer.hpp"

TEST_CASE(TestChhotiEeReordering) {
    auto kruti = minglish::LoadFontProfileFromFile("config/fonts/kruti_dev_010.json");

    // कि -> fd
    std::string s1 = minglish::TranspileToTargetFont(u"कि", kruti);
    ASSERT_EQ(s1, "fd");

    // स्मि -> fLe (f before half-s 'L' and base-m 'e')
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
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`
Expected: Compilation failure due to missing `visual_reorderer.hpp`.

- [ ] **Step 3: Implement `include/engine/visual_reorderer.hpp` and `src/engine/visual_reorderer.cpp`**

Implement akshara clustering (`ParseAksharas`) and glyph substitution with pre-base placement of `chhoti_ee` (`f`) and post-base placement of `reph` (`Z`).

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: `[PASS] TestChhotiEeReordering`, `[PASS] TestRephPostPositioning`.

- [ ] **Step 5: Commit**

```bash
git add include/engine/visual_reorderer.hpp src/engine/visual_reorderer.cpp tests/test_visual_reorderer.cpp
git commit -m "feat(engine): implement Akshara parsing and visual reordering for legacy fonts"
```

---

### Task 6: Differential Backspace & Keystroke Generation (Stage 4)

**Files:**
- Create: `include/engine/differential_injector.hpp`
- Create: `src/engine/differential_injector.cpp`
- Test: `tests/test_differential_injector.cpp`

**Interfaces:**
- Consumes: C++ stdlib
- Produces:
  ```cpp
  namespace minglish {
      struct InjectionDiff {
          size_t backspaceCount = 0;
          std::string textToInject;
      };

      InjectionDiff ComputeDifferential(std::string_view previousText, std::string_view newText);
  }
  ```

- [ ] **Step 1: Write failing unit tests in `tests/test_differential_injector.cpp`**

```cpp
#include "test_framework.hpp"
#include "engine/differential_injector.hpp"

TEST_CASE(TestDifferentialPrefixMatch) {
    // Typing k -> "d", then ka -> "d", then kaa -> "dk"
    auto diff1 = minglish::ComputeDifferential("d", "dk");
    ASSERT_EQ(diff1.backspaceCount, 0);
    ASSERT_EQ(diff1.textToInject, "k");

    // Typing k -> "d", then kh -> "[k"
    auto diff2 = minglish::ComputeDifferential("d", "[k");
    ASSERT_EQ(diff2.backspaceCount, 1);
    ASSERT_EQ(diff2.textToInject, "[k");

    // In Kruti Dev: "d" -> "fd" (ki)
    auto diff3 = minglish::ComputeDifferential("d", "fd");
    ASSERT_EQ(diff3.backspaceCount, 1);
    ASSERT_EQ(diff3.textToInject, "fd");

    return true;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`
Expected: Compilation failure due to missing `differential_injector.hpp`.

- [ ] **Step 3: Implement `include/engine/differential_injector.hpp` and `src/engine/differential_injector.cpp`**

Calculate the Longest Common Prefix (LCP) between `previousText` and `newText`, returning `backspaceCount = previousText.length() - LCP` and `textToInject = newText.substr(LCP)`.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: `[PASS] TestDifferentialPrefixMatch`.

- [ ] **Step 5: Commit**

```bash
git add include/engine/differential_injector.hpp src/engine/differential_injector.cpp tests/test_differential_injector.cpp
git commit -m "feat(engine): implement differential backspace calculation"
```

---

### Task 7: Core Transpiler Engine Integration & End-to-End Verification

**Files:**
- Create: `include/engine/transpiler_engine.hpp`
- Create: `src/engine/transpiler_engine.cpp`
- Test: `tests/test_transpiler_engine.cpp`

**Interfaces:**
- Consumes: `word_buffer.hpp`, `font_profile.hpp`, `visual_reorderer.hpp`, `differential_injector.hpp`
- Produces:
  ```cpp
  namespace minglish {
      class TranspilerEngine {
      public:
          TranspilerEngine();
          void SetFontProfile(const FontProfile& profile);
          InjectionDiff OnChar(char c);
          InjectionDiff OnBackspace();
          void Reset();
          std::string GetPreviouslyInjected() const;
      private:
          WordBuffer m_buffer;
          FontProfile m_activeProfile;
          std::string m_previouslyInjected;
      };
  }
  ```

- [ ] **Step 1: Write failing end-to-end unit tests in `tests/test_transpiler_engine.cpp`**

Verify typing sequences in Kruti Dev and Unicode end-to-end:
- Typing `k` then `h` (verifying backspace + upgrade)
- Typing `k` then `i` (verifying `fd` / `कि` reordering)
- Typing `k` + `a` + `r` + `m` (verifying `deZ` / `कर्म` Reph positioning)
- Backspace handling rolling back buffer correctly

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`
Expected: Compilation failure due to missing `transpiler_engine.hpp`.

- [ ] **Step 3: Implement `include/engine/transpiler_engine.hpp` and `src/engine/transpiler_engine.cpp`**

Connect `WordBuffer`, `TranspileToTargetFont`, and `ComputeDifferential` into a unified, high-performance engine interface.

- [ ] **Step 4: Run full test suite to verify 100% pass rate**

Run: `cmake --build build` then `ctest --test-dir build --output-on-failure`
Expected: All tests pass with 0 failures.

- [ ] **Step 5: Commit**

```bash
git add include/engine/transpiler_engine.hpp src/engine/transpiler_engine.cpp tests/test_transpiler_engine.cpp
git commit -m "feat(engine): complete Core Transpiler and Differential Backspace Engine"
```
