#include "test_framework.hpp"

TEST_CASE(TestDummy) {
    ASSERT_EQ(1 + 1, 2);
    return true;
}

int main() {
    int failed = 0;
    int total = 0;
    std::cout << "========================================\n";
    std::cout << " Running Modern ISM Test Suite\n";
    std::cout << "========================================\n";
    for (const auto& tc : GetTestCases()) {
        total++;
        std::cout << "[RUN ] " << tc.name << "\n";
        if (tc.test()) {
            std::cout << "[PASS] " << tc.name << "\n";
        } else {
            std::cout << "[FAIL] " << tc.name << "\n";
            failed++;
        }
    }
    std::cout << "========================================\n";
    std::cout << " Tests: " << (total - failed) << " passed, " << failed << " failed\n";
    std::cout << "========================================\n";
    return failed == 0 ? 0 : 1;
}
