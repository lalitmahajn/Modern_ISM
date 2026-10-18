#include "engine/visual_reorderer.hpp"
#include <iostream>

namespace minglish {

namespace {

bool IsDevanagariConsonant(char16_t c) {
    // Basic Devanagari consonants
    return (c >= 0x0915 && c <= 0x0939) || (c >= 0x0958 && c <= 0x095F);
}

bool IsDevanagariMatra(char16_t c) {
    return (c >= 0x093E && c <= 0x094C) || (c >= 0x094E && c <= 0x094F) ||
           (c >= 0x0955 && c <= 0x0957) || (c == 0x0962 || c == 0x0963);
}

bool IsDevanagariModifier(char16_t c) {
    return (c >= 0x0901 && c <= 0x0903) || (c == 0x093C);
}

std::string Utf16ToUtf8(const std::u16string& u16) {
    std::string out;
    for (char16_t c : u16) {
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else if (c < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (c >> 6)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (c >> 12)));
            out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return out;
}

} // anonymous namespace

std::vector<AksharaCluster> ParseAksharas(const std::u16string& text) {
    std::vector<AksharaCluster> clusters;
    size_t i = 0;
    const size_t len = text.length();

    while (i < len) {
        AksharaCluster cluster;
        size_t startPos = i;

        // Check for initial Reph: 'र' (0x0930) + '्' (0x094D) followed by a consonant
        if (text[i] == 0x0930 && (i + 1 < len) && text[i + 1] == 0x094D && (i + 2 < len) && IsDevanagariConsonant(text[i + 2])) {
            cluster.hasReph = true;
            i += 2; // skip र and virama
        }

        // Parse any half consonants (Consonant + Virama)
        while (i < len && IsDevanagariConsonant(text[i]) && (i + 1 < len) && text[i + 1] == 0x094D) {
            // If there's another consonant after virama, text[i..i+1] is a half consonant
            if (i + 2 < len && IsDevanagariConsonant(text[i + 2])) {
                std::u16string hc = text.substr(i, 2);
                cluster.halfConsonants.push_back(hc);
                i += 2;
            } else {
                break;
            }
        }

        // Base consonant or independent character
        if (i < len) {
            // Check if current char + next is a composite or virama at word end
            cluster.baseConsonant.push_back(text[i]);
            i++;
            if (i < len && text[i] == 0x094D) {
                // Halant directly on base consonant at end
                cluster.baseConsonant.push_back(text[i]);
                i++;
            }
        }

        // Matra (vowel sign)
        if (i < len && IsDevanagariMatra(text[i])) {
            cluster.matra = text[i];
            i++;
        }

        // Modifiers (Anusvara, Visarga, Nukta)
        while (i < len && IsDevanagariModifier(text[i])) {
            cluster.modifiers.push_back(text[i]);
            i++;
        }

        cluster.rawCluster = text.substr(startPos, i - startPos);
        clusters.push_back(cluster);
    }

    return clusters;
}

std::string TranspileToTargetFont(const std::u16string& unicodeWord, const FontProfile& profile) {
    if (unicodeWord.empty()) {
        return "";
    }

    // Default Unicode profile: pass through canonical UTF-8
    if (profile.id == "unicode_devanagari" || (!profile.features.chhotiEePreBase && !profile.features.rephPostBase)) {
        return Utf16ToUtf8(unicodeWord);
    }

    std::vector<AksharaCluster> clusters = ParseAksharas(unicodeWord);
    std::string result;

    for (const auto& cluster : clusters) {
        // 1. Chhoti Ee ('ि', 0x093F) placed pre-base in legacy visual fonts
        if (cluster.matra == 0x093F && profile.features.chhotiEePreBase) {
            result += profile.chhotiEeGlyph; // e.g. "f" in Kruti Dev
        }

        // 2. Half Consonants (placed before base consonant)
        for (const auto& hc : cluster.halfConsonants) {
            auto itHalf = profile.halfConsonants.find(hc);
            if (itHalf != profile.halfConsonants.end()) {
                result += itHalf->second;
            } else {
                // Fallback to direct map
                auto itDirect = profile.directMap.find(hc);
                if (itDirect != profile.directMap.end()) {
                    result += itDirect->second;
                } else {
                    result += Utf16ToUtf8(hc);
                }
            }
        }

        // 3. Base Consonant
        if (!cluster.baseConsonant.empty()) {
            // Check ligatures first
            auto itLig = profile.ligatures.find(cluster.baseConsonant);
            if (itLig != profile.ligatures.end()) {
                result += itLig->second;
            } else {
                auto itDirect = profile.directMap.find(cluster.baseConsonant);
                if (itDirect != profile.directMap.end()) {
                    result += itDirect->second;
                } else {
                    // Check half consonant map if base has virama
                    auto itHalf = profile.halfConsonants.find(cluster.baseConsonant);
                    if (itHalf != profile.halfConsonants.end()) {
                        result += itHalf->second;
                    } else {
                        result += Utf16ToUtf8(cluster.baseConsonant);
                    }
                }
            }
        }

        // 4. Post-Base Matra (all matras except Chhoti Ee)
        if (cluster.matra != 0 && cluster.matra != 0x093F) {
            std::u16string mStr(1, cluster.matra);
            auto itMatra = profile.directMap.find(mStr);
            if (itMatra != profile.directMap.end()) {
                result += itMatra->second;
            } else {
                result += Utf16ToUtf8(mStr);
            }
        }

        // 5. Reph (Flying "R") placed post-base and post-matra in legacy fonts
        if (cluster.hasReph && profile.features.rephPostBase) {
            result += profile.rephGlyph; // e.g. "Z" in Kruti Dev
        }

        // 6. Modifiers (Anusvara, Visarga)
        for (char16_t mod : cluster.modifiers) {
            std::u16string modStr(1, mod);
            auto itMod = profile.directMap.find(modStr);
            if (itMod != profile.directMap.end()) {
                result += itMod->second;
            } else {
                result += Utf16ToUtf8(modStr);
            }
        }
    }

    return result;
}

} // namespace minglish
