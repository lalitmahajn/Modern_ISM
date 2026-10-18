#pragma once
#include <string>

namespace minglish {

class WordBuffer {
public:
    WordBuffer() = default;

    void PushKey(char c);
    bool PopKey();
    void Clear();
    bool IsEmpty() const;
    std::string GetRawInput() const;
    std::u16string ToUnicode() const;

private:
    std::string m_raw;
};

} // namespace minglish
