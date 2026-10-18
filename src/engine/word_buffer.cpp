#include "engine/word_buffer.hpp"
#include "engine/minglish_keymap.hpp"
#include <vector>

namespace minglish {

void WordBuffer::PushKey(char c) {
    m_raw.push_back(c);
}

bool WordBuffer::PopKey() {
    if (m_raw.empty()) {
        return false;
    }
    m_raw.pop_back();
    return true;
}

void WordBuffer::Clear() {
    m_raw.clear();
}

bool WordBuffer::IsEmpty() const {
    return m_raw.empty();
}

std::string WordBuffer::GetRawInput() const {
    return m_raw;
}

namespace {

struct Token {
    std::string key;
    const KeyMapping* mapping = nullptr;
};

std::vector<Token> Tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < input.length()) {
        // Greedy match up to length 4
        size_t maxLen = std::min(static_cast<size_t>(4), input.length() - i);
        bool matched = false;
        for (size_t len = maxLen; len >= 1; --len) {
            std::string sub = input.substr(i, len);
            const KeyMapping* m = FindMapping(sub);
            if (m != nullptr) {
                tokens.push_back({sub, m});
                i += len;
                matched = true;
                break;
            }
        }
        if (!matched) {
            // Unmapped character, treat as individual
            tokens.push_back({input.substr(i, 1), nullptr});
            i += 1;
        }
    }
    return tokens;
}

} // anonymous namespace

std::u16string WordBuffer::ToUnicode() const {
    if (m_raw.empty()) {
        return u"";
    }

    std::vector<Token> tokens = Tokenize(m_raw);
    std::u16string result;

    KeyCategory lastCategory = KeyCategory::Unknown;
    bool lastConsonantConfirmed = false;

    for (size_t idx = 0; idx < tokens.size(); ++idx) {
        const auto& tok = tokens[idx];
        if (!tok.mapping) {
            // Append raw ASCII char as UTF-16
            result.push_back(static_cast<char16_t>(tok.key[0]));
            lastCategory = KeyCategory::Unknown;
            lastConsonantConfirmed = false;
            continue;
        }

        switch (tok.mapping->category) {
            case KeyCategory::Consonant: {
                if (lastCategory == KeyCategory::Consonant && !lastConsonantConfirmed) {
                    // Implicit virama between consecutive consonants in a cluster
                    result.push_back(u'्');
                }
                result += tok.mapping->independent;
                lastCategory = KeyCategory::Consonant;
                lastConsonantConfirmed = false;
                break;
            }

            case KeyCategory::Vowel: {
                if (tok.key == "a") {
                    if (lastCategory == KeyCategory::Consonant) {
                        if (!lastConsonantConfirmed) {
                            // Confirms inherent 'a'
                            lastConsonantConfirmed = true;
                            // Category remains Consonant so next vowel or consonant knows
                        } else {
                            // Second 'a' after inherent confirmation -> aa matra ('ा')
                            result += u"ा";
                            lastCategory = KeyCategory::Vowel;
                            lastConsonantConfirmed = false;
                        }
                    } else {
                        // Word-initial or after non-consonant -> independent 'अ'
                        result += tok.mapping->independent;
                        lastCategory = KeyCategory::Vowel;
                        lastConsonantConfirmed = false;
                    }
                } else {
                    // Non-'a' vowel (i, ee, u, oo, e, ai, o, au, E, c, O, ru, etc.)
                    if (lastCategory == KeyCategory::Consonant && !lastConsonantConfirmed) {
                        result += tok.mapping->dependent;
                    } else {
                        result += tok.mapping->independent;
                    }
                    lastCategory = KeyCategory::Vowel;
                    lastConsonantConfirmed = false;
                }
                break;
            }

            case KeyCategory::Modifier: {
                result += tok.mapping->dependent;
                lastCategory = KeyCategory::Modifier;
                lastConsonantConfirmed = false;
                break;
            }

            case KeyCategory::Virama: {
                result += tok.mapping->dependent;
                lastCategory = KeyCategory::Virama;
                lastConsonantConfirmed = false;
                break;
            }

            default: {
                lastCategory = KeyCategory::Unknown;
                lastConsonantConfirmed = false;
                break;
            }
        }
    }

    return result;
}

} // namespace minglish
