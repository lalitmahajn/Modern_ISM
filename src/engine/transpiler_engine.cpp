#include "engine/transpiler_engine.hpp"

namespace minglish {

namespace {

std::string Utf16ToUtf8(const std::u16string& u16) {
    std::string out;
    for (char16_t c : u16) {
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else if (c < 0x800) {
            out.push_back(static_cast<char>(0xC0 | ((c >> 6) & 0x1F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | ((c >> 12) & 0x0F)));
            out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return out;
}

} // namespace

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

    std::u16string newTargetU16;
    if (m_profile.IsUnicode()) {
        newTargetU16 = unicodeText;
    } else {
        newTargetU16.reserve(newTargetText.size());
        for (unsigned char ch : newTargetText) {
            newTargetU16.push_back(static_cast<char16_t>(ch));
        }
    }
    m_previouslyInjectedU16 = newTargetU16;

    return diff;
}

InjectionDiffU16 TranspilerEngine::OnCharU16(char c) {
    m_buffer.PushKey(c);
    std::u16string unicodeText = m_buffer.ToUnicode();
    std::u16string newTargetU16;

    if (m_profile.IsUnicode()) {
        newTargetU16 = unicodeText;
        m_previouslyInjected = Utf16ToUtf8(unicodeText);
    } else {
        std::string newTargetText = TranspileToTargetFont(unicodeText, m_profile);
        m_previouslyInjected = newTargetText;
        newTargetU16.reserve(newTargetText.size());
        for (unsigned char ch : newTargetText) {
            newTargetU16.push_back(static_cast<char16_t>(ch));
        }
    }

    InjectionDiffU16 diff = ComputeDifferentialU16(m_previouslyInjectedU16, newTargetU16);
    m_previouslyInjectedU16 = newTargetU16;
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

    std::u16string newTargetU16;
    if (m_profile.IsUnicode()) {
        newTargetU16 = unicodeText;
    } else {
        newTargetU16.reserve(newTargetText.size());
        for (unsigned char ch : newTargetText) {
            newTargetU16.push_back(static_cast<char16_t>(ch));
        }
    }
    m_previouslyInjectedU16 = newTargetU16;

    return diff;
}

InjectionDiffU16 TranspilerEngine::OnBackspaceU16() {
    if (m_buffer.IsEmpty()) {
        return {0, u""};
    }

    m_buffer.PopKey();
    std::u16string unicodeText = m_buffer.ToUnicode();
    std::u16string newTargetU16;

    if (m_profile.IsUnicode()) {
        newTargetU16 = unicodeText;
        m_previouslyInjected = Utf16ToUtf8(unicodeText);
    } else {
        std::string newTargetText = TranspileToTargetFont(unicodeText, m_profile);
        m_previouslyInjected = newTargetText;
        newTargetU16.reserve(newTargetText.size());
        for (unsigned char ch : newTargetText) {
            newTargetU16.push_back(static_cast<char16_t>(ch));
        }
    }

    InjectionDiffU16 diff = ComputeDifferentialU16(m_previouslyInjectedU16, newTargetU16);
    m_previouslyInjectedU16 = newTargetU16;
    return diff;
}

void TranspilerEngine::Reset() {
    m_buffer.Clear();
    m_previouslyInjected.clear();
    m_previouslyInjectedU16.clear();
}

std::string TranspilerEngine::GetPreviouslyInjected() const {
    return m_previouslyInjected;
}

std::u16string TranspilerEngine::GetPreviouslyInjectedU16() const {
    return m_previouslyInjectedU16;
}

std::u16string TranspilerEngine::GetCurrentUnicode() const {
    return m_buffer.ToUnicode();
}

} // namespace minglish
