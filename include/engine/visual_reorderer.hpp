#pragma once
#include "engine/font_profile.hpp"
#include <string>
#include <vector>

namespace minglish {

struct AksharaCluster {
    std::u16string rawCluster;
    bool hasReph = false;
    std::vector<std::u16string> halfConsonants;
    std::u16string baseConsonant;
    char16_t matra = 0;
    std::u16string modifiers;
};

std::vector<AksharaCluster> ParseAksharas(const std::u16string& unicodeWord);
std::string TranspileToTargetFont(const std::u16string& unicodeWord, const FontProfile& profile);

} // namespace minglish
