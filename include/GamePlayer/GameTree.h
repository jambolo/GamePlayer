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

class GameTree
{
public:

    GameTree(std::shared_ptr<TranspositionTable> tt, std::shared_ptr<StaticEvaluator> sef, int maxDepth);

    //! Searches for the best response and returns it
    void findBestResponse(std::shared_ptr<GameState> & s0) const;

#if defined(ANALYSIS_GAME_TREE)

    // Analysis data for the last move

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

    // Sets the value of the node to the value of the first player's best response
    void firstPlayerSearch(Node * node, float alpha, float beta, int depth) const;

    // Sets the value of the node to the value of the second player's best response
    void secondPlayerSearch(Node * node, float alpha, float beta, int depth) const;

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
