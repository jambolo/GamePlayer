#pragma once

#if !defined(GAMEPLAYER_GAMETREE_H)
#define GAMEPLAYER_GAMETREE_H

#include <memory>
#include <vector>
#if defined(ANALYSIS_GAME_TREE)
#include <nlohmann/json.hpp>
#endif

namespace GamePlayer
{
class GameState;
class StaticEvaluator;
class TranspositionTable;

//! An game tree search implementation using min-max strategy, alpha-beta pruning, and a transposition table.

class GameTree
{
public:
    //! Constructor.
    //! 
    //! @param 	tt          A transposition table to be used in a search. The table is assumed to be persistent.
    //! @param 	sef         The static evaluation function
    //! @param 	maxDepth    The maximum number of plies to search
    GameTree(std::shared_ptr<TranspositionTable> tt, std::shared_ptr<StaticEvaluator> sef, int maxDepth);

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
        int generatedCounts[MAX_DEPTH];
        int evaluatedCounts[MAX_DEPTH];
        float value;
        int alphaCutoffs;
        int betaCutoffs;
#if defined(ANALYSIS_GAME_STATE)
        GameState::AnalysisData gsAnalysisData;
#endif

        AnalysisData();
        void           reset();
        nlohmann::json toJson() const;
    };

    //! Analysis data for the last move
    mutable AnalysisData analysisData_;

#endif  // defined(ANALYSIS_GAME_TREE)

private:

    struct Node
    {
        std::shared_ptr<GameState> state;
        float value;       // Value of the state
        int quality;       // Quality of the value
#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
        int priority;      // Higher priority states should be searched first
#endif
    };
    using NodeList = std::vector<GameTree::Node>;

#if defined(FEATURE_NEGAMAX)
    // Sets the value of the node to the value of the best response
    void nextPly(Node * node, float playerFactor, float alpha, float beta, int depth) const;
#else // defined(FEATURE_NEGAMAX)
    // Sets the value of the node to the value of the first player's best response
    void firstPlayerSearch(Node * node, float alpha, float beta, int depth) const;

    // Sets the value of the node to the value of the second player's best response
    void secondPlayerSearch(Node * node, float alpha, float beta, int depth) const;
#endif // defined(FEATURE_NEGAMAX)

    // Generates a list of responses to the given node
    void generateResponses(Node const * node, int depth, NodeList & responses) const;

    // Get the value of the state from the static evaluator or the transposition table
    void getValue(GameState const & state, int depth, float * pValue, int * pQuality) const;

#if defined(FEATURE_PRIORITIZED_MOVE_ORDERING)
    // Computes a search priority for the state
    void prioritize(GameState * pstate, int depth);
#endif

#if defined(DEBUG_GAME_TREE_NODE_INFO)
    void printStateInfo(GameState const & state, int depth, float alpha, float beta);
#endif
    static bool descendingSorter(Node const & a, Node const & b);
    static bool ascendingSorter(Node const & a, Node const & b);

    int maxDepth_;                                              // How deep to search
    std::shared_ptr<TranspositionTable> transpositionTable_;    // Transposition table (persistent)
    std::shared_ptr<StaticEvaluator> staticEvaluator_;          // Transposition table (persistent)
};
} // namespace GamePlayer

#endif // !defined(GAMEPLAYER_GAMETREE_H)
