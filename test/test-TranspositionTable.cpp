#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "GamePlayer/TranspositionTable.h"

namespace GamePlayer
{

TEST(TranspositionTable, Constructor)
{
    ASSERT_NO_THROW(TranspositionTable(256, 1));                          // Make sure the constructor works with valid parameters
    EXPECT_DEATH(TranspositionTable(0, 1), "Assertion failed: .*");       // Should not allow 0 index size
    EXPECT_DEATH(TranspositionTable(256, 0), "Assertion failed: .*");     // Should not allow 0 max age
    EXPECT_DEATH(TranspositionTable(256, 32767), "Assertion failed: .*"); // Age is int16_t, so maxAge >= 32767 should fail
}

TEST(TranspositionTable, Set_Check)
{
    // Verify that set() and check() asserts when called with the UNUSED fingerprint value.
    {
        TranspositionTable tt(256, 1);
        uint64_t const     UNUSED = (uint64_t)-1; // The assumed UNUSED fingerprint value

        ASSERT_DEATH(tt.set(UNUSED, 0.0f, 0), "Assertion failed: .*"); // Should not allow UNUSED fingerprint
        ASSERT_DEATH(tt.check(UNUSED), "Assertion failed: .*");        // Should not allow UNUSED fingerprint
        ASSERT_DEATH(tt.set(1, 0.0f, -1), "Assertion failed: .*");     // Quality is expected to be non-negative
        ASSERT_DEATH(tt.set(1, 0.0f, 32768), "Assertion failed: .*");  // Quality is int16_t, so 32768 should fail
    }
    // Verify that a new table is empty and that check() returns nullopt for empty entries.
    // Note: This assumes that table indexes are fingerprint MOD table size.
    {
        TranspositionTable tt(256, 1);
        for (uint64_t i = 0; i < 256; ++i)
        {
            auto result = tt.check(i);
            EXPECT_THAT(result, ::testing::Eq(std::nullopt));
        }
    }
    // Verify that set() and check() work correctly.
    // Note: This assumes that table indexes are fingerprint MOD table size.
    {
        TranspositionTable tt(256, 1);
        for (uint64_t i = 0; i < 256; ++i)
        {
            tt.set(0x0123456789abcdefLL + i, static_cast<float>((i * 65537) % 257) / 3.0f, (i * 838853) % 32749);
        }
        for (uint64_t i = 0; i < 256; ++i)
        {
            auto result = tt.check(0x0123456789abcdefLL + i);
            EXPECT_TRUE(result.has_value()); // Should find the entry
            if (!result.has_value())
                break; // If not found, give up because the rest will not be found either
            EXPECT_THAT(result->value, ::testing::FloatEq(static_cast<float>((i * 65537) % 257) / 3.0f));
            EXPECT_EQ(result->quality, (i * 838853) % 32749);                       // Check quality
        }
    }
}

TEST(TranspositionTable, Update)
{
    TranspositionTable tt(16, 5);

    tt.update(1, 1.0f, 1);
    auto r1 = tt.check(1);
    ASSERT_TRUE(r1.has_value());
    EXPECT_THAT(r1->value, ::testing::FloatEq(1.0f));
    EXPECT_EQ(r1->quality, 1);

    tt.update(1, 2.0f, 0);
    auto r2 = tt.check(1);
    ASSERT_TRUE(r2.has_value());
    EXPECT_THAT(r2->value, ::testing::FloatEq(1.0f));
    EXPECT_EQ(r2->quality, 1);

    tt.update(1, 3.0f, 1);
    auto r3 = tt.check(1);
    ASSERT_TRUE(r3.has_value());
    EXPECT_THAT(r3->value, ::testing::FloatEq(3.0f));
    EXPECT_EQ(r3->quality, 1);

    tt.update(1, 4.0f, 2);
    auto r4 = tt.check(1);
    ASSERT_TRUE(r4.has_value());
    EXPECT_THAT(r4->value, ::testing::FloatEq(4.0f));
    EXPECT_EQ(r4->quality, 2);
}

TEST(TranspositionTable, Age)
{
    TranspositionTable tt(8, 2);

    tt.set(100, 1.5f, 1);
    ASSERT_TRUE(tt.check(100).has_value());

    tt.age();
    tt.age();
    tt.age();
    EXPECT_FALSE(tt.check(100).has_value());
}

} // namespace GamePlayer
