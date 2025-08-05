#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "GamePlayer/GameState.h"
#include "GamePlayer/StaticEvaluator.h"
#include "GamePlayer/TranspositionTable.h"

#include <memory>
#include <vector>

#include "GamePlayer/GameTree.h"

namespace GamePlayer
{

namespace
{

class TestTreeState : public GameState
{
public:
    TestTreeState(uint64_t fp, PlayerId turn, float score)
        : fingerprint_(fp)
        , turn_(turn)
        , score_(score)
    {
    }

    uint64_t fingerprint() const override { return fingerprint_; }
    PlayerId whoseTurn() const override { return turn_; }

    uint64_t fingerprint_;
    PlayerId turn_;
    float    score_;
};

class TestTreeEvaluator : public StaticEvaluator
{
public:
    float evaluate(GameState const & state) const override
    {
        auto const & s = dynamic_cast<TestTreeState const &>(state);
        return s.score_;
    }

    float aliceWinsValue() const override { return 100.0f; }
    float bobWinsValue() const override { return -100.0f; }
};

} // namespace

TEST(GameTree, Constructor)
{
    auto tt  = std::make_shared<TranspositionTable>(256, 2);
    auto sef = std::make_shared<TestTreeEvaluator>();
    GameTree::ResponseGenerator rg = [](GameState const &, int)
    {
        return std::vector<GameState *>{};
    };

    EXPECT_NO_THROW(GameTree(tt, sef, rg, 2));
}

TEST(GameTree, ResponseGenerator)
{
    auto tt  = std::make_shared<TranspositionTable>(256, 2);
    auto sef = std::make_shared<TestTreeEvaluator>();

    int callCount = 0;
    GameTree::ResponseGenerator rg = [&callCount](GameState const & state, int depth)
    {
        ++callCount;
        auto const & s = dynamic_cast<TestTreeState const &>(state);
        if (depth == 0)
        {
            return std::vector<GameState *>{new TestTreeState(s.fingerprint_ + 1, GameState::PlayerId::BOB, 1.0f),
                                            new TestTreeState(s.fingerprint_ + 2, GameState::PlayerId::BOB, 3.0f),
                                            new TestTreeState(s.fingerprint_ + 3, GameState::PlayerId::BOB, 2.0f)};
        }
        return std::vector<GameState *>{};
    };

    GameTree tree(tt, sef, rg, 1);

    std::shared_ptr<GameState> root = std::make_shared<TestTreeState>(1000, GameState::PlayerId::ALICE, 0.0f);
    tree.findBestResponse(root);

    ASSERT_THAT(root->response_, ::testing::NotNull());
    auto const * best = dynamic_cast<TestTreeState const *>(root->response_.get());
    ASSERT_NE(best, nullptr);
    EXPECT_EQ(best->fingerprint_, 1002u);
    EXPECT_GE(callCount, 1);
}

TEST(GameTree, FindBestResponse)
{
    auto tt  = std::make_shared<TranspositionTable>(256, 2);
    auto sef = std::make_shared<TestTreeEvaluator>();

    GameTree::ResponseGenerator rg = [](GameState const & state, int depth)
    {
        auto const & s = dynamic_cast<TestTreeState const &>(state);
        if (depth == 0)
        {
            if (s.whoseTurn() == GameState::PlayerId::ALICE)
            {
                return std::vector<GameState *>{new TestTreeState(2001, GameState::PlayerId::BOB, 5.0f),
                                                new TestTreeState(2002, GameState::PlayerId::BOB, 1.0f)};
            }
            return std::vector<GameState *>{new TestTreeState(3001, GameState::PlayerId::ALICE, 4.0f),
                                            new TestTreeState(3002, GameState::PlayerId::ALICE, -2.0f)};
        }
        return std::vector<GameState *>{};
    };

    GameTree tree(tt, sef, rg, 1);

    {
        std::shared_ptr<GameState> aliceRoot = std::make_shared<TestTreeState>(2000, GameState::PlayerId::ALICE, 0.0f);
        tree.findBestResponse(aliceRoot);
        ASSERT_THAT(aliceRoot->response_, ::testing::NotNull());
        auto const * best = dynamic_cast<TestTreeState const *>(aliceRoot->response_.get());
        ASSERT_NE(best, nullptr);
        EXPECT_EQ(best->fingerprint_, 2001u);
    }

    {
        std::shared_ptr<GameState> bobRoot = std::make_shared<TestTreeState>(3000, GameState::PlayerId::BOB, 0.0f);
        tree.findBestResponse(bobRoot);
        ASSERT_THAT(bobRoot->response_, ::testing::NotNull());
        auto const * best = dynamic_cast<TestTreeState const *>(bobRoot->response_.get());
        ASSERT_NE(best, nullptr);
        EXPECT_EQ(best->fingerprint_, 3002u);
    }
}

} // namespace GamePlayer
