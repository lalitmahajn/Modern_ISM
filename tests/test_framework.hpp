#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>

struct TestCase {
    std::string name;
    std::function<bool()> test;
};

inline std::vector<TestCase>& GetTestCases() {
    static std::vector<TestCase> cases;
    return cases;
}

#define TEST_CASE(name) \
    bool name(); \
    static bool reg_##name = []() { \
        GetTestCases().push_back({#name, name}); \
        return true; \
    }(); \
    bool name()

inline std::string ToString(const std::string& s) { return s; }
inline std::string ToString(std::string_view s) { return std::string(s); }
inline std::string ToString(int v) { return std::to_string(v); }
inline std::string ToString(size_t v) { return std::to_string(v); }
inline std::string ToString(bool v) { return v ? "true" : "false"; }
inline std::string ToString(const std::u16string& u16) {
    std::string out;
    for (char16_t c : u16) {
        if (c < 0x80) out.push_back(static_cast<char>(c));
        else if (c < 0x800) {
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
inline std::string ToString(const char16_t* p) {
    return ToString(std::u16string(p));
}

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        std::cerr << "Assertion failed at line " << __LINE__ \
                  << ": expected [" << ToString(expected) << "] got [" << ToString(actual) << "]\n"; \
        return false; \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed at line " << __LINE__ << ": expected true\n"; \
        return false; \
    }
