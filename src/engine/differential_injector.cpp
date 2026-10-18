#include "engine/differential_injector.hpp"
#include <algorithm>

namespace minglish {

InjectionDiff ComputeDifferential(std::string_view previousText, std::string_view newText) {
    size_t lcp = 0;
    size_t minLen = std::min(previousText.length(), newText.length());

    while (lcp < minLen && previousText[lcp] == newText[lcp]) {
        lcp++;
    }

    InjectionDiff diff;
    diff.backspaceCount = previousText.length() - lcp;
    diff.textToInject = std::string(newText.substr(lcp));

    return diff;
}

} // namespace minglish
