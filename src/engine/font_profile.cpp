#include "engine/font_profile.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

namespace minglish {

namespace {

std::u16string Utf8ToUtf16(std::string_view utf8) {
    std::u16string out;
    size_t i = 0;
    while (i < utf8.length()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char16_t>(c));
            i += 1;
        } else if ((c >> 5) == 0x06) {
            if (i + 1 < utf8.length()) {
                char16_t ch = ((c & 0x1F) << 6) | (static_cast<unsigned char>(utf8[i + 1]) & 0x3F);
                out.push_back(ch);
            }
            i += 2;
        } else if ((c >> 4) == 0x0E) {
            if (i + 2 < utf8.length()) {
                char16_t ch = ((c & 0x0F) << 12) |
                              ((static_cast<unsigned char>(utf8[i + 1]) & 0x3F) << 6) |
                              (static_cast<unsigned char>(utf8[i + 2]) & 0x3F);
                out.push_back(ch);
            }
            i += 3;
        } else {
            i += 4;
        }
    }
    return out;
}

// Simple JSON tokenizer and parser for font configuration
class SimpleJsonReader {
public:
    explicit SimpleJsonReader(std::string_view content) : m_src(content), m_pos(0) {}

    void SkipWhitespace() {
        while (m_pos < m_src.length() && (std::isspace(static_cast<unsigned char>(m_src[m_pos])) || m_src[m_pos] == ',')) {
            m_pos++;
        }
    }

    bool Match(char expected) {
        SkipWhitespace();
        if (m_pos < m_src.length() && m_src[m_pos] == expected) {
            m_pos++;
            return true;
        }
        return false;
    }

    std::string ReadString() {
        SkipWhitespace();
        if (m_pos >= m_src.length() || m_src[m_pos] != '"') {
            return "";
        }
        m_pos++; // skip opening quote
        std::string s;
        while (m_pos < m_src.length()) {
            char c = m_src[m_pos++];
            if (c == '"') {
                break;
            }
            if (c == '\\' && m_pos < m_src.length()) {
                char next = m_src[m_pos++];
                if (next == '"') s.push_back('"');
                else if (next == '\\') s.push_back('\\');
                else if (next == 'n') s.push_back('\n');
                else if (next == 't') s.push_back('\t');
                else s.push_back(next);
            } else {
                s.push_back(c);
            }
        }
        return s;
    }

    bool ReadBool() {
        SkipWhitespace();
        if (m_src.substr(m_pos, 4) == "true") {
            m_pos += 4;
            return true;
        }
        if (m_src.substr(m_pos, 5) == "false") {
            m_pos += 5;
            return false;
        }
        return false;
    }

    std::unordered_map<std::u16string, std::string> ReadStringMap() {
        std::unordered_map<std::u16string, std::string> map;
        if (!Match('{')) {
            return map;
        }
        while (m_pos < m_src.length()) {
            SkipWhitespace();
            if (m_pos < m_src.length() && m_src[m_pos] == '}') {
                m_pos++;
                break;
            }
            std::string key = ReadString();
            if (key.empty()) {
                break;
            }
            if (!Match(':')) {
                break;
            }
            std::string val = ReadString();
            map[Utf8ToUtf16(key)] = val;
        }
        return map;
    }

    std::string_view m_src;
    size_t m_pos;
};

} // anonymous namespace

FontProfile LoadFontProfileFromJson(std::string_view jsonContent) {
    FontProfile profile;
    SimpleJsonReader reader(jsonContent);

    if (!reader.Match('{')) {
        return profile;
    }

    while (reader.m_pos < reader.m_src.length()) {
        reader.SkipWhitespace();
        if (reader.m_pos < reader.m_src.length() && reader.m_src[reader.m_pos] == '}') {
            reader.m_pos++;
            break;
        }

        std::string section = reader.ReadString();
        if (section.empty() || !reader.Match(':')) {
            break;
        }

        if (section == "profile_id") {
            profile.id = reader.ReadString();
        } else if (section == "name") {
            profile.name = reader.ReadString();
        } else if (section == "encoding") {
            profile.encoding = reader.ReadString();
        } else if (section == "features") {
            if (reader.Match('{')) {
                while (reader.m_pos < reader.m_src.length()) {
                    reader.SkipWhitespace();
                    if (reader.m_pos < reader.m_src.length() && reader.m_src[reader.m_pos] == '}') {
                        reader.m_pos++;
                        break;
                    }
                    std::string fKey = reader.ReadString();
                    if (reader.Match(':')) {
                        if (fKey == "chhoti_ee_pre_base") profile.features.chhotiEePreBase = reader.ReadBool();
                        else if (fKey == "reph_post_base") profile.features.rephPostBase = reader.ReadBool();
                        else if (fKey == "has_half_consonants") profile.features.hasHalfConsonants = reader.ReadBool();
                        else reader.ReadBool();
                    }
                }
            }
        } else if (section == "glyphs") {
            if (reader.Match('{')) {
                while (reader.m_pos < reader.m_src.length()) {
                    reader.SkipWhitespace();
                    if (reader.m_pos < reader.m_src.length() && reader.m_src[reader.m_pos] == '}') {
                        reader.m_pos++;
                        break;
                    }
                    std::string gKey = reader.ReadString();
                    if (reader.Match(':')) {
                        std::string gVal = reader.ReadString();
                        if (gKey == "chhoti_ee") profile.chhotiEeGlyph = gVal;
                        else if (gKey == "reph") profile.rephGlyph = gVal;
                        else if (gKey == "virama") profile.viramaGlyph = gVal;
                        else if (gKey == "rakar") profile.rakarGlyph = gVal;
                        else if (gKey == "rakar_stemless") profile.rakarStemlessGlyph = gVal;
                    }
                }
            }
        } else if (section == "direct_map") {
            profile.directMap = reader.ReadStringMap();
        } else if (section == "half_consonants") {
            profile.halfConsonants = reader.ReadStringMap();
        } else if (section == "ligatures") {
            profile.ligatures = reader.ReadStringMap();
        } else {
            // Skip unknown structure (e.g. contextual_rules array)
            reader.SkipWhitespace();
            if (reader.m_pos < reader.m_src.length() && reader.m_src[reader.m_pos] == '[') {
                int depth = 1;
                reader.m_pos++;
                while (reader.m_pos < reader.m_src.length() && depth > 0) {
                    if (reader.m_src[reader.m_pos] == '[') depth++;
                    else if (reader.m_src[reader.m_pos] == ']') depth--;
                    reader.m_pos++;
                }
            }
        }
    }

    return profile;
}

FontProfile LoadFontProfileFromFile(std::string_view filePath) {
    std::string path(filePath);
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        file.open("../" + path, std::ios::binary);
    }
    if (!file.is_open()) {
        file.open("../../" + path, std::ios::binary);
    }
    if (!file.is_open()) {
        std::cerr << "Failed to open font profile file: " << filePath << "\n";
        return GetDefaultUnicodeProfile();
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return LoadFontProfileFromJson(ss.str());
}

FontProfile GetDefaultUnicodeProfile() {
    FontProfile profile;
    profile.id = "unicode_devanagari";
    profile.name = "Unicode Standard Devanagari";
    profile.encoding = "utf16";
    profile.features.chhotiEePreBase = false;
    profile.features.rephPostBase = false;
    profile.features.hasHalfConsonants = false;
    profile.chhotiEeGlyph = "ि";
    profile.rephGlyph = "र्";
    profile.viramaGlyph = "्";
    profile.rakarGlyph = "्र";
    return profile;
}

} // namespace minglish
