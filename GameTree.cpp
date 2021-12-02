#include "GamePlayer/GameTree.h"

#include "GamePlayer/GameState.h"
#include "GamePlayer/StaticEvaluator.h"
#include "GamePlayer/TranspositionTable.h"

// #include "Misc/Random.h"

#include <functional>
#include <limits>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
int constexpr SEF_QUALITY = 0;

bool shouldDoQuiescentSearch(float previousValue, float thisValue)
{
#if defined(FEATURE_QUIESCENT_SEARCH)

    // In the normal case, we would not search. However, if the situation is unsettled and we haven't reached the
    // true depth limit, then we should search the next ply.

    float const QUIESCENT_THRESHOLD = 1.0f;
    return abs(previousValue - thisValue) >= QUIESCENT_THRESHOLD;

#else   // defined(FEATURE_QUIESCENT_SEARCH)

    return false;

#endif  // defined(FEATURE_QUIESCENT_SEARCH)
}
} // anonymous namespace

namespace GamePlayer
{
GameTree::GameTree(std::shared_ptr<TranspositionTable> tt,
                   std::shared_ptr<StaticEvaluator> sef,
                   ResponseGenerator rg, int maxDepth)
    : maxDepth_(maxDepth)
    , transpositionTable_(tt)
    , staticEvaluator_(sef)
    , responseGenerator_(rg)
{
}

void GameTree::findBestResponse(std::shared_ptr<GameState> & s0) const
{
    Node root{ s0 };

#if defined(FEATURE_NEGAMAX)
    if (s0->whoseTurn() == GameState::PlayerId::FIRST)
        nextPly(&root, 1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0);
    else
        nextPly(&root, -1.0f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0);
#else   // defined(FEATURE_NEGAMAX)
    if (s0->whoseTurn() == GameState::PlayerId::FIRST)
        firstPlayerSearch(&root, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0);
    else
        secondPlayerSearch(&root, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0);
#endif  // defined(FEATURE_NEGAMAX)

#if defined(ANALYSIS_GAME_TREE)
    analysisData_.value = root.value;
#endif
}

#if defined(FEATURE_NEGAMAX)

// Recursively evaluate this move using the negamax algorithm

void GameTree::nextPly(Node * node, float playerFactor, float alpha, float beta, int depth) const
{
    int responseDepth = depth + 1;                      // Depth of responses to this state
    int quality       = maxDepth_ - depth;              // Quality of values at this depth (this is the depth of plies searched to
                                                        // get the results for this ply)
    int minResponseQuality = maxDepth_ - responseDepth; // Minimum acceptable quality of responses to this state

    // Generate a list of the possible responses to this state. They are sorted in descending order hoping that a beta cutoff will
    // occur early.
    // Note: Preliminary values of the generated states are retrieved from the transposition table or computed by
    // the static evaluation function.
    NodeList responses = generateResponses(node, depth);

    // Sort from highest to lowest
    std::sort(responses.begin(), responses.end(), descendingSorter);

    // Evaluate each of the responses and choose the one with the highest value
    bool pruned = false;
    Node bestResponse{ nullptr, -std::numeric_limits<float>::max() * playerFactor };

    for (auto & response : responses)
    {
        // If the game is not over, then let's see how the second player responds (updating the value of this response)
        if (response.value != staticEvaluator_->firstPlayerWins() * playerFactor)
        {
            // The quality of a value is basically the depth of the search tree below it. The reason for checking the quality is
            // that some of the responses have not been fully searched. If the quality of the preliminary value is not as good as
            // the minimum quality and we haven't reached the maximum depth, then do a search. Otherwise, the response's quality is
            // as good as the quality of a search, so use the response as is.
            if ((response.quality < minResponseQuality) &&
                ((responseDepth < maxDepth_) ||
                 (shouldDoQuiescentSearch(node->value, response.value) && (responseDepth < maxDepth_ + 1))))
            {
                nextPly(&response, -playerFactor, -beta, -alpha, responseDepth);
            }
        }
#if defined(DEBUG_GAME_TREE_NODE_INFO)
        printStateInfo(response, depth, alpha, beta);
#endif

        float value = response.value * playerFactor;
        // Determine if this response's value is the best so far. If so, then save the value and do alpha-beta pruning
        if (value > bestResponse.value)
        {
            // Save it
            bestResponse = response;

            // If first player wins with this response, then there is no reason to look for anything better
            if (value == staticEvaluator_->firstPlayerWins())
                break;

            // alpha-beta pruning (beta cutoff) Here's how it works:
            //
            // The second player is looking for the lowest value. The 'beta' is the value of the second player's best move found so
            // far in the previous ply. If the value of this response is higher than the beta, then the second player will abandon
            // its move leading to this response because because the result is worse than the result of a move it has already
            // found. As such, there is no reason to continue.

            if (value > beta)
            {
                // Beta cutoff
                pruned = true;
#if defined(ANALYSIS_GAME_TREE)
                ++analysisData_.betaCutoffs;
#endif
                break;
            }

            // alpha-beta pruning (alpha) Here's how it works.
            //
            // The first player is looking for the highest value. The 'alpha' is the value of the first player's best move found so
            // far. If the value of this response is higher than the alpha, then obviously it is a better move for the first
            // player. The alpha is subsequently passed to the second player's search so that if it finds a response with a lower
            // value than the alpha, it won't bother continuing because it knows that the first player already has a better move
            // and will choose it instead of allowing the second player to make a move with a lower value.

            if (value > alpha)
                alpha = value;
        }
    }

    // Return the value of this move

    node->value            = bestResponse.value;
    node->quality          = quality;
    node->state->response_ = bestResponse.state;

    // Save the value of the state in the T-table if the ply was not pruned. Pruning results in an incorrect value
    // because the search is not complete. Also, the value is stored only if its quality is better than the quality
    // of the value in the table.
    if (!pruned)
        transpositionTable_->update(*node->state, node->value, node->quality);

    // Release all generated states
}

#else // defined(FEATURE_NEGAMAX)

// Evaluate all of the first player's possible responses to the given state. The chosen response is the one with the highest value.

void GameTree::firstPlayerSearch(Node * node, float alpha, float beta, int depth) const
{
    int responseDepth = depth + 1;                      // Depth of responses to this state
    int quality       = maxDepth_ - depth;              // Quality of values at this depth (this is the depth of plies searched to
                                                        // get the results for this ply)
    int minResponseQuality = maxDepth_ - responseDepth; // Minimum acceptable quality of responses to this state

    // Generate a list of the possible responses to this state. They are sorted in descending order hoping that a beta cutoff will
    // occur early.
    // Note: Preliminary values of the generated states are retrieved from the transposition table or computed by
    // the static evaluation function.
    NodeList responses = generateResponses(node, depth);

    // Sort from highest to lowest
    std::sort(responses.begin(), responses.end(), descendingSorter);

    // Evaluate each of the responses and choose the one with the highest value
    bool pruned = false;
    Node bestResponse{ nullptr, -std::numeric_limits<float>::max() };

    for (auto & response : responses)
    {
        // If the game is not over, then let's see how the second player responds (updating the value of this response)
        if (response.value != staticEvaluator_->firstPlayerWins())
        {
            // The quality of a value is basically the depth of the search tree below it. The reason for checking the quality is
            // that some of the responses have not been fully searched. If the quality of the preliminary value is not as good as
            // the minimum quality and we haven't reached the maximum depth, then do a search. Otherwise, the response's quality is
            // as good as the quality of a search, so use the response as is.
            if ((response.quality < minResponseQuality) &&
                ((responseDepth < maxDepth_) ||
                 (shouldDoQuiescentSearch(node->value, response.value) && (responseDepth < maxDepth_ + 1))))
            {
                secondPlayerSearch(&response, alpha, beta, responseDepth);
            }
        }
#if defined(DEBUG_GAME_TREE_NODE_INFO)
        printStateInfo(response, depth, alpha, beta);
#endif

        // Determine if this response's value is the best so far. If so, then save the value and do alpha-beta pruning
        if (response.value > bestResponse.value)
        {
            // Save it
            bestResponse = response;

            // If first player wins with this response, then there is no reason to look for anything better
            if (bestResponse.value == staticEvaluator_->firstPlayerWins())
                break;

            // alpha-beta pruning (beta cutoff) Here's how it works:
            //
            // The second player is looking for the lowest value. The 'beta' is the value of the second player's best move found so
            // far in the previous ply. If the value of this response is higher than the beta, then the second player will abandon
            // its move leading to this response because because the result is worse than the result of a move it has already
            // found. As such, there is no reason to continue.

            if (bestResponse.value > beta)
            {
                // Beta cutoff
                pruned = true;
#if defined(ANALYSIS_GAME_TREE)
                ++analysisData_.betaCutoffs;
#endif
                break;
            }

            // alpha-beta pruning (alpha) Here's how it works.
            //
            // The first player is looking for the highest value. The 'alpha' is the value of the first player's best move found so
            // far. If the value of this response is higher than the alpha, then obviously it is a better move for the first
            // player. The alpha is subsequently passed to the second player's search so that if it finds a response with a lower
            // value than the alpha, it won't bother continuing because it knows that the first player already has a better move
            // and will choose it instead of allowing the second player to make a move with a lower value.

            if (bestResponse.value > alpha)
                alpha = bestResponse.value;
        }
    }

    // Return the value of this move

    node->value            = bestResponse.value;
    node->quality          = quality;
    node->state->response_ = bestResponse.state;

    // Save the value of the state in the T-table if the ply was not pruned. Pruning results in an incorrect value
    // because the search is not complete. Also, the value is stored only if its quality is better than the quality
    // of the value in the table.
    if (!pruned)
        transpositionTable_->update(node->state->fingerprint(), node->value, node->quality);

    // Release all generated states
}

// Evaluate all of the second player's possible responses to the given state. The chosen response is the one with the lowest value.

void GameTree::secondPlayerSearch(Node * node, float alpha, float beta, int depth) const
{
    int responseDepth = depth + 1;                      // Depth of responses to this state
    int quality       = maxDepth_ - depth;              // Quality of values at this depth (this is the depth of plies searched to
                                                        // get the results for this ply)
    int minResponseQuality = maxDepth_ - responseDepth; // Minimum acceptable quality of responses to this state

    // Generate a list of the possible responses to this state. They are sorted in ascending order hoping that a alpha cutoff will
    // occur early.
    // Note: Preliminary values of the generated states are retrieved from the transposition table or computed by
    // the static evaluation function.
    NodeList responses = generateResponses(node, depth);

    // Sort from lowest to highest
    std::sort(responses.begin(), responses.end(), ascendingSorter);

    // Evaluate each of the responses and choose the one with the lowest value
    Node bestResponse{ nullptr, std::numeric_limits<float>::max() };
    bool pruned = false;

    for (auto & response : responses)
    {
        // If the game is not over, then let's see how the first player responds (updating the value of this response)
        if (response.value != staticEvaluator_->secondPlayerWins())
        {
            // The quality of a value is basically the depth of the search tree below it. The reason for checking the quality is
            // that some of the responses have not been fully searched. If the quality of the preliminary value is not as good as
            // the minimum quality and we haven't reached the maximum depth, then do a search. Otherwise, the response's quality is
            // as good as the quality of a search, so use the response as is.
            if ((response.quality < minResponseQuality) &&
                ((responseDepth < maxDepth_) ||
                 (shouldDoQuiescentSearch(node->value, response.value) && (responseDepth < maxDepth_ + 1))))
            {
                firstPlayerSearch(&response, alpha, beta, responseDepth);
            }
        }
#if defined(DEBUG_GAME_TREE_NODE_INFO)
        printStateInfo(response, depth, alpha, beta);
#endif

        // Determine if this response's value is the best so far. If so, then save the value and do alpha-beta pruning
        if (response.value < bestResponse.value)
        {
            // Save it
            bestResponse = response;
#if defined(ANALYSIS_GAME_STATE)
            bestResponse = response;
#endif

            // If first player wins with this response, then there is no reason to look for anything better
            if (bestResponse.value == staticEvaluator_->secondPlayerWins())
                break;

            // alpha-beta pruning (alpha cutoff) Here's how it works:
            //
            // The first player is looking for the highest value. The 'alpha' is the value of the first player's best move found so
            // far in the previous ply. If the value of this response is lower than the alpha, then the first player will abandon
            // its move leading to this response because because the result is worse than the result of a move it has already
            // found. As such, there is no reason to continue.

            if (bestResponse.value < alpha)
            {
                // Alpha cutoff
                pruned = true;

#if defined(ANALYSIS_GAME_TREE)
                ++analysisData_.alphaCutoffs;
#endif

                break;
            }

            // alpha-beta pruning (beta) Here's how it works.
            //
            // The second player is looking for the lowest value. The 'beta' is the value of the second player's best move found so
            // far. If the value of this response is lower than the beta, then obviously it is a better move for the second
            // player. The beta is subsequently passed to the first player's search so that if it finds a response with a higher
            // value than the beta, it won't bother continuing because it knows that the second player already has a better move
            // and will choose it instead of allowing the first player to make a move with a higher value.

            if (bestResponse.value < beta)
                beta = bestResponse.value;
        }
    }

    // Return the value of this move

    node->value            = bestResponse.value;
    node->quality          = quality;
    node->state->response_ = bestResponse.state;

    // Save the value of the state in the T-table if the ply was not pruned. Pruning results in an incorrect value
    // because the search is not complete. Also, the value is stored only if its quality is better than the quality
    // of the value in the table.
    if (!pruned)
        transpositionTable_->update(node->state->fingerprint(), node->value, node->quality);
}

#endif // defined(FEATURE_NEGAMAX)

GameTree::NodeList GameTree::generateResponses(Node const * node, int depth) const
{
    std::vector<GameState *> responses = responseGenerator_(*node->state, depth);

#if defined(ANALYSIS_GAME_TREE)
    if (depth < GamePlayer::GameTree::AnalysisData::MAX_DEPTH)
        analysisData_.generatedCounts[depth] += (int)responses.size();
#endif

    NodeList rv;
    rv.resize(responses.size());

    // Create a list of response nodes
    std::transform(responses.begin(), responses.end(), rv.begin(), [this, depth] (GameState * state)
                   {
                       float value;
                       int quality;
                       getValue(*state, depth, &value, &quality);
                       return Node{ std::shared_ptr<GameState>(state), value, quality };
                   });

    return rv;
}

void GameTree::getValue(GameState const & state, int depth, float * pValue, int * pQuality) const
{
    // SEF optimization:
    //
    // Since any value of any state in the T-table has already been computed by search and/or SEF, it has a quality
    // that is at least as good as the quality of the value returned by the SEF. So, if the state being evaluated
    // is in the T-table, then the value in the T-table is used instead of running the SEF because T-table
    // lookup is so much faster than the SEF.

    // If it is in the T-table then use that value, otherwise compute the value using SEF.
    std::optional<TranspositionTable::CheckResult> result = transpositionTable_->check(state.fingerprint());
    if (result)
    {
        *pValue   = result->first;
        *pQuality = result->second;
        return;
    }

#if defined(ANALYSIS_GAME_TREE)
    if (depth < GamePlayer::GameTree::AnalysisData::MAX_DEPTH)
        ++analysisData_.evaluatedCounts[depth];
#endif

#if defined(FEATURE_INCREMENTAL_STATIC_EVALUATION)

    // Note: The static value was already computed during move generation

#if defined(FEATURE_INCREMENTAL_STATIC_EVALUATION_VALIDATION)
    ASSERT(state.value_ == staticEvaluator_->evaluate(*pState));
#endif

    *pValue   = state.value_;
    *pQuality = SEF_QUALITY;

#else   // defined(FEATURE_INCREMENTAL_STATIC_EVALUATION)

    float value = staticEvaluator_->evaluate(state);

    *pValue   = value;
    *pQuality = SEF_QUALITY;

#endif  // defined(FEATURE_INCREMENTAL_STATIC_EVALUATION)

    // Save the value of the state in the T-table
    transpositionTable_->update(state.fingerprint(), *pValue, *pQuality);
}

#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)

int GameTree::prioritize(Node const & state, int depth)
{
    int const quality = maxDepth_ - depth;

    // Prioritization strategy:
    //
    // It is assumed that the tree will do a search only if the preliminary quality is lower than the result of
    // searching. So, a preliminary quality as high as the quality at this ply is given a higher priority. The
    // result is that high-priority values are considered first and could potentially hasten the occurrences of
    // alpha-beta cut-offs because they are probably closer to the final value. The cost of a preliminary value is the cost of a
    // T-table lookup or an SEF, which is much cheaper than the cost of a search. The savings of this strategy is the saving gained
    // by earlier alpha-beta cutoffs minus the costs of the additional SEFs.

    int const PRIORITY_HIGH = 1;
    int const PRIORITY_LOW  = 0;

    return (state.quality_ > quality) ? PRIORITY_HIGH : PRIORITY_LOW;
}

#endif // defined(FEATURE_PRIORITIZED_MOVE_ORDERING)

#if defined(ANALYSIS_GAME_TREE)

GameTree::AnalysisData::AnalysisData()
    : value(0)
    , alphaCutoffs(0)
    , betaCutoffs(0)
{
    memset(generatedCounts, 0, sizeof(generatedCounts));
    memset(evaluatedCounts, 0, sizeof(evaluatedCounts));
}

void GameTree::AnalysisData::reset()
{
    memset(generatedCounts, 0, sizeof(generatedCounts));
    memset(evaluatedCounts, 0, sizeof(evaluatedCounts));

    value        = 0.0f;
    alphaCutoffs = 0;
    betaCutoffs  = 0;

#if defined(ANALYSIS_GAME_STATE)
    gsAnalysisData.reset();
#endif
}

json GameTree::AnalysisData::toJson() const
{
    json out =
    {
        { "generatedCounts", generatedCounts },
        { "evaluatedCounts", evaluatedCounts },
        { "value", value },
        { "alphaCutoffs", alphaCutoffs },
        { "betaCutoffs", betaCutoffs }

#if defined(ANALYSIS_GAME_STATE)
        , { "gameState", gsAnalysisData.toJson() }
#endif
    };
    return out;
}
#endif // defined(ANALYSIS_GAME_TREE)

#if defined(DEBUG_GAME_TREE_NODE_INFO)
void GameTree::printStateInfo(GameState const & state, int depth, int alpha, int beta)
{
    for (int i = 0; i < depth; ++i)
    {
        fprintf(stderr, "    ");
    }

    fprintf(stderr, "%6s, value = %6.2f, quality = %3d, alpha = %6.2f, beta = %6.2f\n",
            state.move_.notation().c_str(), state.value_, state.quality_, alpha, beta);
}

#endif // if defined(DEBUG_GAME_TREE_NODE_INFO)

// Sort the nodes in descending order, first by descending priority, then by descending value.
bool GameTree::descendingSorter(Node const & a, Node const & b)
{
#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
    if (a.priority > b.priority)
        return true;

    if (a.priority < b.priority)
        return false;
#endif

    return a.value > b.value;
}

// Sort the nodes in ascending order, first by descending priority, then by ascending value.
bool GameTree::ascendingSorter(Node const & a, Node const & b)
{
#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
    if (a.priority > b.priority)
        return true;

    if (a.priority < b.priority)
        return false;
#endif

    return a.value < b.value;
}
} // namespace GamePlayer
