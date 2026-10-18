#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

namespace minglish {

struct FontFeatures {
    bool chhotiEePreBase = false;
    bool rephPostBase = false;
    bool hasHalfConsonants = false;
};

struct FontProfile {
    std::string id;
    std::string name;
    std::string encoding;
    FontFeatures features;
    std::string chhotiEeGlyph;
    std::string rephGlyph;
    std::string viramaGlyph;
    std::string rakarGlyph;
    std::string rakarStemlessGlyph;
    std::unordered_map<std::u16string, std::string> directMap;
    std::unordered_map<std::u16string, std::string> halfConsonants;
    std::unordered_map<std::u16string, std::string> ligatures;

    bool IsUnicode() const {
        return encoding == "unicode" || encoding == "UTF-8" || encoding == "utf-8";
    }
};

FontProfile LoadFontProfileFromJson(std::string_view jsonContent);
FontProfile LoadFontProfileFromFile(std::string_view filePath);
FontProfile GetDefaultUnicodeProfile();

} // namespace minglish
