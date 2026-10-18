#pragma once
#include <string>
#include <string_view>

namespace minglish {

struct InjectionDiff {
    size_t backspaceCount = 0;
    std::string textToInject;
};

struct InjectionDiffU16 {
    size_t backspaceCount = 0;
    std::u16string textToInject;
};

InjectionDiff ComputeDifferential(std::string_view previousText, std::string_view newText);
InjectionDiffU16 ComputeDifferentialU16(std::u16string_view previousText, std::u16string_view newText);

} // namespace minglish
