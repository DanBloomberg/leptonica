#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

class SecurityTest : public ::testing::TestWithParam<std::string> {
protected:
    void SetUp() override {
        char filename[256];
        sprintf(filename, "/tmp/libfuzzer.%d", getppid());
        remove(filename);
    }
};

TEST_P(SecurityTest, FuzzerEntryPointMaintainsSecurityBoundary) {
    // Invariant: Fuzzer entry point must not create predictable temporary files
    // that persist after execution or expose system to path traversal
    std::string payload = GetParam();
    
    std::vector<uint8_t> data(payload.begin(), payload.end());
    int result = LLVMFuzzerTestOneInput(data.data(), data.size());
    
    // Security property: Temporary file must not exist after execution
    char filename[256];
    sprintf(filename, "/tmp/libfuzzer.%d", getppid());
    
    FILE *fp = fopen(filename, "rb");
    EXPECT_EQ(fp, nullptr) << "Temporary file persists after execution - security boundary violated";
    if (fp) fclose(fp);
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    SecurityTest,
    ::testing::Values(
        // Exact exploit: Path traversal attempt
        "../../../../etc/passwd",
        // Boundary case: Maximum size that could overflow buffer
        std::string(255, 'A'),
        // Valid input: Normal fuzzer payload
        "valid input"
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}