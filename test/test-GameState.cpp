#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "GamePlayer/GameState.h"

namespace GamePlayer
{

// GameState is an abstract class, so all we can do is check that it compiles and that the interface hasn't changed.
class TestGameState : public GameState
{
public:
    using PlayerId = GameState::PlayerId;

    TestGameState() = default;
    uint64_t fingerprint() const override
    {
        return 0; // Dummy implementation
    }
    PlayerId whoseTurn() const override
    {
        return PlayerId::ALICE; // Dummy implementation
    }
};

TEST(GameState, Constructor)
{
    ASSERT_NO_THROW(TestGameState());
}

TEST(GameState, Fingerprint)
{
    TestGameState state;
    ASSERT_EQ(state.fingerprint(), 0);
}

TEST(GameState, WhoseTurn)
{
    TestGameState state;
    ASSERT_EQ(state.whoseTurn(), TestGameState::PlayerId::ALICE);
}

TEST(GameState, Response)
{
    TestGameState state;
    ASSERT_THAT(state.response_, ::testing::IsNull());
    state.response_ = std::make_shared<TestGameState>();
    ASSERT_THAT(state.response_, ::testing::NotNull());
}

class GameStateOtherPlayerTest : public ::testing::TestWithParam<TestGameState::PlayerId>
{
};

TEST_P(GameStateOtherPlayerTest, ReturnsOppositePlayer)
{
    TestGameState state;
    auto          player = GetParam();
    auto          other  = state.otherPlayer(player);
    EXPECT_NE(other, player);
    EXPECT_EQ(state.otherPlayer(other), player);
}

INSTANTIATE_TEST_SUITE_P(AllPlayers,
                         GameStateOtherPlayerTest,
                         ::testing::Values(TestGameState::PlayerId::ALICE, TestGameState::PlayerId::BOB));

} // namespace GamePlayer
