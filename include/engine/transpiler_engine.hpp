#pragma once
#include "engine/word_buffer.hpp"
#include "engine/font_profile.hpp"
#include "engine/visual_reorderer.hpp"
#include "engine/differential_injector.hpp"
#include <string>

namespace minglish {

class TranspilerEngine {
public:
    TranspilerEngine();
    explicit TranspilerEngine(const FontProfile& profile);

    void SetFontProfile(const FontProfile& profile);
    const FontProfile& GetFontProfile() const;

    // Called on printable character input
    InjectionDiff OnChar(char c);
    InjectionDiffU16 OnCharU16(char c);

    // Called on physical backspace press
    InjectionDiff OnBackspace();
    InjectionDiffU16 OnBackspaceU16();

    // Called on word boundary keys (Space, Enter, Tab, Escape, arrow keys)
    void Reset();

    std::string GetPreviouslyInjected() const;
    std::u16string GetPreviouslyInjectedU16() const;
    std::u16string GetCurrentUnicode() const;

private:
    WordBuffer m_buffer;
    FontProfile m_profile;
    std::string m_previouslyInjected;
    std::u16string m_previouslyInjectedU16;
};

} // namespace minglish
