#include "engine/transpiler_engine.hpp"

namespace minglish {

TranspilerEngine::TranspilerEngine()
    : m_profile(GetDefaultUnicodeProfile()) {}

TranspilerEngine::TranspilerEngine(const FontProfile& profile)
    : m_profile(profile) {}

void TranspilerEngine::SetFontProfile(const FontProfile& profile) {
    m_profile = profile;
    Reset();
}

const FontProfile& TranspilerEngine::GetFontProfile() const {
    return m_profile;
}

InjectionDiff TranspilerEngine::OnChar(char c) {
    m_buffer.PushKey(c);
    std::u16string unicodeText = m_buffer.ToUnicode();
    std::string newTargetText = TranspileToTargetFont(unicodeText, m_profile);

    InjectionDiff diff = ComputeDifferential(m_previouslyInjected, newTargetText);
    m_previouslyInjected = newTargetText;
    return diff;
}

InjectionDiff TranspilerEngine::OnBackspace() {
    if (m_buffer.IsEmpty()) {
        return {0, ""};
    }

    m_buffer.PopKey();
    std::u16string unicodeText = m_buffer.ToUnicode();
    std::string newTargetText = TranspileToTargetFont(unicodeText, m_profile);

    InjectionDiff diff = ComputeDifferential(m_previouslyInjected, newTargetText);
    m_previouslyInjected = newTargetText;
    return diff;
}

void TranspilerEngine::Reset() {
    m_buffer.Clear();
    m_previouslyInjected.clear();
}

std::string TranspilerEngine::GetPreviouslyInjected() const {
    return m_previouslyInjected;
}

std::u16string TranspilerEngine::GetCurrentUnicode() const {
    return m_buffer.ToUnicode();
}

} // namespace minglish
