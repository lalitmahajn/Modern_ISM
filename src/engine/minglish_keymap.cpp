#include "engine/minglish_keymap.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace minglish {

namespace {

const std::unordered_map<std::string, KeyMapping> g_keymap = {
    // Vowels
    {"a",   {u"अ", u"",    KeyCategory::Vowel}},
    {"aa",  {u"आ", u"ा",   KeyCategory::Vowel}},
    {"A",   {u"आ", u"ा",   KeyCategory::Vowel}},
    {"i",   {u"इ", u"ि",   KeyCategory::Vowel}},
    {"ee",  {u"ई", u"ी",   KeyCategory::Vowel}},
    {"I",   {u"ई", u"ी",   KeyCategory::Vowel}},
    {"u",   {u"उ", u"ु",   KeyCategory::Vowel}},
    {"oo",  {u"ऊ", u"ू",   KeyCategory::Vowel}},
    {"U",   {u"ऊ", u"ू",   KeyCategory::Vowel}},
    {"e",   {u"ए", u"े",   KeyCategory::Vowel}},
    {"ai",  {u"ऐ", u"ै",   KeyCategory::Vowel}},
    {"o",   {u"ओ", u"ो",   KeyCategory::Vowel}},
    {"au",  {u"औ", u"ौ",   KeyCategory::Vowel}},
    {"ou",  {u"औ", u"ौ",   KeyCategory::Vowel}},
    {"ru",  {u"ऋ", u"ृ",   KeyCategory::Vowel}},
    {"Ru",  {u"ऋ", u"ृ",   KeyCategory::Vowel}},
    {"E",   {u"ॲ", u"ॅ",   KeyCategory::Vowel}},
    {"c",   {u"ॲ", u"ॅ",   KeyCategory::Vowel}},
    {"O",   {u"ऑ", u"ॉ",   KeyCategory::Vowel}},

    // Modifiers
    {"M",   {u"अं", u"ं",  KeyCategory::Modifier}},
    {"H",   {u"अः", u"ः",  KeyCategory::Modifier}},
    {"ah",  {u"अः", u"ः",  KeyCategory::Modifier}},

    // Virama
    {"/",   {u"",   u"्",  KeyCategory::Virama}},
    {"~",   {u"",   u"्",  KeyCategory::Virama}},

    // Consonants
    {"k",   {u"क", u"क्",  KeyCategory::Consonant}},
    {"kh",  {u"ख", u"ख्",  KeyCategory::Consonant}},
    {"g",   {u"ग", u"ग्",  KeyCategory::Consonant}},
    {"gh",  {u"घ", u"घ्",  KeyCategory::Consonant}},
    {"ng",  {u"ङ", u"ङ्",  KeyCategory::Consonant}},

    {"ch",  {u"च", u"च्",  KeyCategory::Consonant}},
    {"chh", {u"छ", u"छ्",  KeyCategory::Consonant}},
    {"Ch",  {u"छ", u"छ्",  KeyCategory::Consonant}},
    {"j",   {u"ज", u"ज्",  KeyCategory::Consonant}},
    {"jh",  {u"झ", u"झ्",  KeyCategory::Consonant}},
    {"z",   {u"झ", u"झ्",  KeyCategory::Consonant}},
    {"ny",  {u"ञ", u"ञ्",  KeyCategory::Consonant}},
    {"nj",  {u"ञ", u"ञ्",  KeyCategory::Consonant}},

    {"T",   {u"ट", u"ट्",  KeyCategory::Consonant}},
    {"Th",  {u"ठ", u"ठ्",  KeyCategory::Consonant}},
    {"D",   {u"ड", u"ड्",  KeyCategory::Consonant}},
    {"Dh",  {u"ढ", u"ढ्",  KeyCategory::Consonant}},
    {"N",   {u"ण", u"ण्",  KeyCategory::Consonant}},

    {"t",   {u"त", u"त्",  KeyCategory::Consonant}},
    {"th",  {u"थ", u"थ्",  KeyCategory::Consonant}},
    {"d",   {u"द", u"द्",  KeyCategory::Consonant}},
    {"dh",  {u"ध", u"ध्",  KeyCategory::Consonant}},
    {"n",   {u"न", u"न्",  KeyCategory::Consonant}},

    {"p",   {u"प", u"प्",  KeyCategory::Consonant}},
    {"ph",  {u"फ", u"फ्",  KeyCategory::Consonant}},
    {"f",   {u"फ", u"फ्",  KeyCategory::Consonant}},
    {"b",   {u"ब", u"ब्",  KeyCategory::Consonant}},
    {"bh",  {u"भ", u"भ्",  KeyCategory::Consonant}},
    {"m",   {u"म", u"म्",  KeyCategory::Consonant}},

    {"y",   {u"य", u"य्",  KeyCategory::Consonant}},
    {"r",   {u"र", u"र्",  KeyCategory::Consonant}},
    {"l",   {u"ल", u"ल्",  KeyCategory::Consonant}},
    {"v",   {u"व", u"व्",  KeyCategory::Consonant}},
    {"w",   {u"व", u"व्",  KeyCategory::Consonant}},

    {"sh",  {u"श", u"श्",  KeyCategory::Consonant}},
    {"shh", {u"ष", u"ष्",  KeyCategory::Consonant}},
    {"S",   {u"ष", u"ष्",  KeyCategory::Consonant}},
    {"s",   {u"स", u"स्",  KeyCategory::Consonant}},
    {"h",   {u"ह", u"ह्",  KeyCategory::Consonant}},

    {"L",   {u"ळ", u"ळ्",  KeyCategory::Consonant}},
    {"ld",  {u"ळ", u"ळ्",  KeyCategory::Consonant}},

    {"ksh", {u"क्ष", u"क्ष्", KeyCategory::Consonant}},
    {"x",   {u"क्ष", u"क्ष्", KeyCategory::Consonant}},
    {"tr",  {u"त्र", u"त्र्", KeyCategory::Consonant}},
    {"dny", {u"ज्ञ", u"ज्ञ्", KeyCategory::Consonant}},
    {"jny", {u"ज्ञ", u"ज्ञ्", KeyCategory::Consonant}},
    {"shr", {u"श्र", u"श्र्", KeyCategory::Consonant}}
};

std::unordered_set<std::string> BuildPrefixSet() {
    std::unordered_set<std::string> prefixes;
    for (const auto& [key, _] : g_keymap) {
        for (size_t len = 1; len <= key.length(); ++len) {
            prefixes.insert(key.substr(0, len));
        }
    }
    return prefixes;
}

const std::unordered_set<std::string>& GetPrefixSet() {
    static const auto prefixSet = BuildPrefixSet();
    return prefixSet;
}

} // anonymous namespace

const KeyMapping* FindMapping(std::string_view keySeq) {
    auto it = g_keymap.find(std::string(keySeq));
    if (it != g_keymap.end()) {
        return &it->second;
    }
    return nullptr;
}

bool HasPrefix(std::string_view prefix) {
    return GetPrefixSet().contains(std::string(prefix));
}

} // namespace minglish
