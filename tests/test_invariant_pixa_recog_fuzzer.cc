#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

// Include the actual function declaration from the file under test
extern "C" {
    void pixa_recog_fuzzer_process(const char* input, size_t buffer_size);
}

class BufferReadSecurityTest : public ::testing::TestWithParam<std::pair<std::string, size_t>> {};

TEST_P(BufferReadSecurityTest, BufferReadsNeverExceedDeclaredLength) {
    // Invariant: Buffer reads never exceed the declared length
    auto [payload, buffer_size] = GetParam();
    
    // Create a buffer with sentinel values to detect overflow
    std::vector<char> buffer(buffer_size + 2); // Extra space for sentinels
    char* data_buffer = buffer.data() + 1; // Offset by 1 for pre-sentinel
    
    // Set sentinel values before and after the actual buffer
    buffer[0] = 0xAA; // Pre-buffer sentinel
    buffer[buffer.size() - 1] = 0xBB; // Post-buffer sentinel
    
    // Copy payload into buffer (truncate if necessary)
    size_t copy_size = std::min(payload.size(), buffer_size);
    std::memcpy(data_buffer, payload.c_str(), copy_size);
    data_buffer[copy_size] = '\0';
    
    // Call the actual production function
    pixa_recog_fuzzer_process(data_buffer, buffer_size);
    
    // Verify sentinel values remain unchanged (no buffer overflow)
    ASSERT_EQ(buffer[0], 0xAA) << "Pre-buffer sentinel corrupted - buffer underflow detected";
    ASSERT_EQ(buffer[buffer.size() - 1], 0xBB) << "Post-buffer sentinel corrupted - buffer overflow detected";
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    BufferReadSecurityTest,
    ::testing::Values(
        // Exact exploit case: significantly oversized input
        std::make_pair(std::string(1024, 'A'), 64),
        // Boundary case: input exactly at buffer size
        std::make_pair(std::string(64, 'B'), 64),
        // Valid input: well within buffer limits
        std::make_pair(std::string(32, 'C'), 64),
        // Edge case: empty string
        std::make_pair(std::string(""), 64),
        // Special characters that might trigger different code paths
        std::make_pair(std::string("../../etc/passwd\x00\xFF\xFE", 20), 64)
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}