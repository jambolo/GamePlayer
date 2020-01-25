#pragma once

#if !defined(GAMEPLAYER_TRANSPOSITIONTABLE_H)
#define GAMEPLAYER_TRANSPOSITIONTABLE_H

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
#include <nlohmann/json.hpp>
#endif

namespace GamePlayer
{
class GameState;

//! A map of game state values referenced by the states' fingerprints.
//!
//! A game state can be the result of different sequences of the same (or a different) set of moves. This technique is used to
//! cache the value of a game state regardless of the moves used to reach it, thus the name "transposition" table. The purpose of
//! the "transposition" table has been extended to become simply a cache of game state values, so it is more aptly named
//! "game state value cache" -- but the old name persists.
//!
//! As a speed and memory optimization in this implementation, slots in the table are not unique to the state being stored, and a
//! value may be overwritten when a new value is added. A value is overwritten only when its "quality" is less than or equal to the
//! "quality" of the value being added.
//!
//! @note    The fingerprint is assumed to be random and uniformly distibuted.
    
class TranspositionTable
{
public:

    static int constexpr INDEX_SIZE = 19;               //!< Number of bits in the table's index
    static int constexpr SIZE       = 1 << INDEX_SIZE;  //!< Number of entries in the table
    static int constexpr MAX_AGE    = 1;                //!< Entries not referenced in this many turns are removed by age()

    //! Constructor
    TranspositionTable();

    //! Returns the stored value and quality of the given state
    bool check(GameState const & state,
               float *           pReturnedValue   = nullptr,
               int *             pReturnedQuality = nullptr) const;

    //! Returns the stored value and quality of the given state
    bool check(GameState const & state,
               int               minQ,
               float *           pReturnedValue   = nullptr,
               int *             pReturnedQuality = nullptr) const;

    //! Stores a value in the table if the quality of the value it would overwrite (if any) is lower than its quality
    void update(GameState const & state, float value, int quality);

    //! Stores a value in the table regardless of the quality
    void set(GameState const & state, float value, int quality);

    //! Bumps the ages of table entries so that they can eventually be replaced by newer entries.
    void age();

#if defined(ANALYSIS_TRANSPOSITION_TABLE)

    // Analysis data

    struct AnalysisData
    {
        int checkCount;     // The number of checks
        int updateCount;    // The number of updates
        int hitCount;       // The number of times a state was found
        int collisionCount; // The number of times a different state was found
        int rejected;       // The number of times an update was rejected
        int overwritten;    // The number of times a state was overwritten by a different state
        int refreshed;      // The number of times a state was updated with a newer value
        int usage;          // The number of slots in use

        AnalysisData();
        void           reset();
        nlohmann::json toJson() const;
    };

    mutable AnalysisData analysisData_;

#endif  // defined(ANALYSIS_TRANSPOSITION_TABLE)

private:

    struct Entry
    {
        uint64_t fingerprint_; // The state's fingerprint
        float value_;          // The state's value
        int8_t q_;             // The quality of the value
        mutable int8_t age_;   // The number of turns since the entry has been referenced

        static uint64_t constexpr UNUSED_ENTRY = (uint64_t)-1;
        void clear() { fingerprint_ = UNUSED_ENTRY; }
    };

    Entry const & find(uint64_t hash) const { return table_[hash % SIZE]; }
    Entry &       find(uint64_t hash)       { return table_[hash % SIZE]; }

    Entry table_[SIZE];
};
} // namespace GamePlayer

#endif // !defined(GAMEPLAYER_TRANSPOSITIONTABLE_H)
