#pragma once
#include <string>
#include <string_view>

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

} // namespace minglish
