#pragma once

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
#include <nlohmann/json_fwd.hpp>
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace GamePlayer
{
//! A map of game state values referenced by the states' fingerprints.
//!
//! A game state can be the result of different sequences of the same (or a different) set of moves. This technique is
//! used to cache the value of a game state regardless of the moves used to reach it, thus the name "transposition"
//! table. The purpose of the "transposition" table has been extended to become simply a cache of game state values, so
//! it is more aptly named "game state value cache" -- but the old name persists.
//!
//! As a speed and memory optimization in this implementation, slots in the table are not unique to the state being
//! stored, and a value may be overwritten when a new value is added. A value is overwritten only when its "quality" is
//! less than or equal to the "quality" of the value being added.
//!
//! @note    The fingerprint is assumed to be random and uniformly distributed.

class TranspositionTable
{
public:

    //! Constructor
    TranspositionTable(size_t indexSize, int maxAge);

    //! Result type returned by check().
    //! @param  _0  value of the state
    //! @param  _1  quality of the returned value
    using CheckResult = std::pair<float, int>;

    //! Returns the stored value and quality of the given state
    std::optional<CheckResult> check(uint64_t fingerprint) const;

    //! Returns the stored value and quality of the given state
    std::optional<CheckResult> check(uint64_t fingerprint, int minQ) const;

    //! Stores a value in the table if the quality of its value is higher
    void update(uint64_t fingerprint, float value, int quality);

    //! Stores a value in the table regardless of the quality
    void set(uint64_t fingerprint, float value, int quality);

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
        uint64_t fingerprint_;  // The state's fingerprint
        float value_;           // The state's value
        int16_t q_;             // The quality of the value
        mutable int16_t age_;   // The number of turns since the entry has been referenced

        static uint64_t constexpr UNUSED_ENTRY = (uint64_t)-1;
        void clear() { fingerprint_ = UNUSED_ENTRY; }
    };

    Entry const & find(uint64_t hash) const { return table_[hash % table_.size()]; }
    Entry &       find(uint64_t hash)       { return table_[hash % table_.size()]; }

    std::vector<Entry> table_;
    int maxAge_;
};
} // namespace GamePlayer
