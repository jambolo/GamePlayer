#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>
#if defined(ANALYSIS_GAME_TREE)
#if defined(ANALYSIS_GAME_STATE)
#include "GamePlayer/GameState.h"
#endif // defined(ANALYSIS_GAME_STATE)
#include <nlohmann/json_fwd.hpp>
#endif // defined(ANALYSIS_GAME_TREE)

namespace GamePlayer
{
class GameState;
class StaticEvaluator;
class TranspositionTable;

//! A game tree search implementation using min-max strategy, alpha-beta pruning, and a transposition table.

class GameTree
{
public:
    //! Response generator function object type.
    //! @param  state   state to respond to
    //! @param  depth   current ply
    //! @return list of all possible responses
    //! @note   The caller gains ownership of the returned states.
    //! @note   Returning no responses simply indicates that neither player can continue. It does not indicate the that game is
    //!         over or that the player has passed. If passing is allowed, then it must be included in the responses, especially
    //!         if is the only legal move. Similarly, if the inability to move results a loss, then the loss must be included as a
    //!         response.
    using ResponseGenerator = std::function<std::vector<GameState *>(GameState const & state, int depth)>;

    //! Constructor.
    //!
    //! @param 	tt          A transposition table to be used in a search. The table is assumed to be persistent.
    //! @param 	sef         The static evaluation function
    //! @param  rg          The response generator
    //! @param 	maxDepth    The maximum number of plies to search
    GameTree(std::shared_ptr<TranspositionTable> tt, std::shared_ptr<StaticEvaluator> sef, ResponseGenerator rg, int maxDepth);

    //! Searches for the best response to the given state.
    //!
    //! @param  s0  The current state
    //!
    //! @return     The chosen response is returned in s0->response_.
    void findBestResponse(std::shared_ptr<GameState> & s0) const;

#if defined(ANALYSIS_GAME_TREE)

    //! Analysis data relevant to the game tree's operation
    struct AnalysisData
    {
        static size_t constexpr MAX_DEPTH = 10; // Maximum number of plies tracked
        int   generatedCounts[MAX_DEPTH];
        int   evaluatedCounts[MAX_DEPTH];
        float value;
        int   alphaCutoffs;
        int   betaCutoffs;
#if defined(ANALYSIS_GAME_STATE)
        GameState::AnalysisData gsAnalysisData;
#endif // defined(ANALYSIS_GAME_STATE)

        AnalysisData();
        void           reset();
        nlohmann::json toJson() const;
    };

    //! Analysis data for the last move
    mutable AnalysisData analysisData_;

#endif // defined(ANALYSIS_GAME_TREE)

private:
    struct Node
    {
        std::shared_ptr<GameState> state;
        float                      value;   // Value of the state
        int                        quality; // Quality of the value
#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
        int priority; // Higher priority states should be searched first
#endif                // defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
    };
    using NodeList = std::vector<GameTree::Node>;

#if defined(FEATURE_NEGAMAX)
    // Sets the value of the node to the value of the best response
    void nextPly(Node * node, float playerFactor, float alpha, float beta, int depth) const;
#else  // defined(FEATURE_NEGAMAX)
    // Sets the value of the node to the value of Alice's best response
    void aliceSearch(Node * node, float alpha, float beta, int depth) const;

    // Sets the value of the node to the value of Bob's best response
    void bobSearch(Node * node, float alpha, float beta, int depth) const;
#endif // defined(FEATURE_NEGAMAX)

    // Generates a list of responses to the given node
    NodeList generateResponses(Node const * node, int depth) const;

    // Get the value of the state from the static evaluator or the transposition table
    void getValue(GameState const & state, int depth, float * pValue, int * pQuality) const;

#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
    // Computes a search priority for the state
    void prioritize(GameState * pstate, int depth);
#endif // defined(FEATURE_PRIORITIZED_MOVE_ORDERING)

#if defined(DEBUG_GAME_TREE_NODE_INFO)
    void printStateInfo(Node const & state, int depth, float alpha, float beta) const;
#endif // defined(DEBUG_GAME_TREE_NODE_INFO)

    // Used to sort nodes by value
    static bool descendingSorter(Node const & a, Node const & b);
    static bool ascendingSorter(Node const & a, Node const & b);

    int                                 maxDepth_;           // How deep to search
    std::shared_ptr<TranspositionTable> transpositionTable_; // Transposition table (persistent)
    std::shared_ptr<StaticEvaluator>    staticEvaluator_;    // Static evaluator (persistent)
    ResponseGenerator                   responseGenerator_;
};
} // namespace GamePlayer
