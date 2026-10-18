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

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        std::cerr << "Assertion failed at line " << __LINE__ \
                  << ": expected [" << (expected) << "] got [" << (actual) << "]\n"; \
        return false; \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed at line " << __LINE__ << ": expected true\n"; \
        return false; \
    }
