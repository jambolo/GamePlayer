#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "GamePlayer/GameState.h"
#include "GamePlayer/StaticEvaluator.h"

namespace GamePlayer
{

// StaticEvaluator is an abstract class, so all we can do is check that it compiles and that the interface hasn't changed.
class TestStaticEvaluator : public StaticEvaluator
{
public:
    TestStaticEvaluator() = default;

    float evaluate(GameState const & state) const override
    {
        return 0.0f; // Dummy implementation
    }

    float aliceWinsValue() const override
    {
        return 1.0f; // Dummy implementation
    }

    float bobWinsValue() const override
    {
        return -1.0f; // Dummy implementation
    }
};

TEST(StaticEvaluator, Constructor)
{
    ASSERT_NO_THROW(TestStaticEvaluator());
}

TEST(StaticEvaluator, Evaluate)
{
    TestStaticEvaluator evaluator;
    class DummyGameState : public GameState
    {
    public:
        DummyGameState() = default;
        uint64_t fingerprint() const override { return 0; }
        PlayerId whoseTurn() const override { return PlayerId::ALICE; }
    };
    DummyGameState dummyState;
    EXPECT_THAT(evaluator.evaluate(dummyState), ::testing::FloatEq(0.0f));
}

TEST(StaticEvaluator, AliceWinsValue)
{
    TestStaticEvaluator evaluator;
    EXPECT_THAT(evaluator.aliceWinsValue(), ::testing::FloatEq(1.0f));
}

TEST(StaticEvaluator, BobWinsValue)
{
    TestStaticEvaluator evaluator;
    EXPECT_THAT(evaluator.bobWinsValue(), ::testing::FloatEq(-1.0f));
}

TEST(StaticEvaluator, WinBoundsAreOrdered)
{
    TestStaticEvaluator evaluator;
    EXPECT_LT(evaluator.bobWinsValue(), evaluator.aliceWinsValue());
}

} // namespace GamePlayer
