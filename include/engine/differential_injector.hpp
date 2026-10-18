#pragma once
#include <string>
#include <string_view>

namespace minglish {

struct InjectionDiff {
    size_t backspaceCount = 0;
    std::string textToInject;
};

InjectionDiff ComputeDifferential(std::string_view previousText, std::string_view newText);

} // namespace minglish
