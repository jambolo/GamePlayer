#include "GamePlayer/TranspositionTable.h"

#include "GamePlayer/Gamestate.h"

#include <nlohmann/json.hpp>

#include <cassert>

using json = nlohmann::json;

namespace GamePlayer
{
TranspositionTable::TranspositionTable()
{
    // Invalidate all entries in the table
    for (auto & e : table_)
    {
        e.clear();
    }
}

//! This function returns the value of a state if the value is stored in the table. Otherwise, false is returned and the return
//! values are not modified.
//!
//! @param  state               State to be checked for
//! @param  pReturnedValue      Where to return the state's value if not NULL (default: NULL)
//! @param  pReturnedQuality    Where to return the value's quality if not NULL (default: NULL)
//!
//! @return true, if the state's value was found

bool TranspositionTable::check(GameState const & state,
                               float *           pReturnedValue /* = NULL*/,
                               int *             pReturnedQuality /* = NULL*/) const
{
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.checkCount;
#endif

    uint64_t hash = state.fingerprint();
    assert(hash != Entry::UNUSED_ENTRY);
    Entry const & entry = find(hash);

    // The state is found if the fingerprints are the same.

    if (entry.fingerprint_ == hash)
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        ++analysisData_.hitCount;
#endif

        if (pReturnedValue)
            *pReturnedValue = entry.value_;
        if (pReturnedQuality)
            *pReturnedQuality = entry.q_;
        entry.age_ = 0;  // Reset age
        return true;
    }
    else
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        if (entry.fingerprint_ != Entry::UNUSED_ENTRY)
            ++analysisData_.collisionCount;
#endif
        return false;
    }
}

//! This function returns the value of a state if the value is stored in the table, but only if the stored value's quality is above
//! the specified minimum. Otherwise, false is returned and the return values are not modified.
//!
//! @param  state               State to be checked for
//! @param  minQ                Minimum quality
//! @param  pReturnedValue      Where to return the state's value if not NULL (default: NULL)
//! @param  pReturnedQuality    Where to return the value's quality if not NULL (default: NULL)
//!
//! @return true, if the state was found and its value's quality is higher than the minimum

bool TranspositionTable::check(GameState const & state,
                               int               minQ,
                               float *           pReturnedValue /* = NULL*/,
                               int *             pReturnedQuality /* = NULL*/) const
{
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.checkCount;
#endif

    uint64_t hash = state.fingerprint();
    assert(hash != Entry::UNUSED_ENTRY);
    Entry const & entry = find(hash);

    // A hit occurs if the states are the same, and the minimum quality is <= the quality of the stored state-> The
    // reason for the quality check is that if the stored quality is less, then we aren't going to want the value
    // of the stored state anyway.

    bool hit = false;

    if (entry.fingerprint_ == hash)
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        ++analysisData_.hitCount;
#endif

        if (entry.q_ >= minQ)
        {
            hit = true;
            if (pReturnedValue)
                *pReturnedValue = entry.value_;
            if (pReturnedQuality)
                *pReturnedQuality = entry.q_;
        }
        entry.age_ = 0;         // Reset age
    }
    else
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        if (entry.fingerprint_ != Entry::UNUSED_ENTRY)
            ++analysisData_.collisionCount;
#endif
    }

    return hit;
}

//! @param  state   State whose value is to be stored
//! @param  value   Value to be stored
//! @param  quality Quality of the value

void TranspositionTable::update(GameState const & state, float value, int quality)
{
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.updateCount;
#endif

    uint64_t hash = state.fingerprint();
    assert(hash != Entry::UNUSED_ENTRY);
    Entry & entry = find(hash);

    bool isUnused = (entry.fingerprint_ == Entry::UNUSED_ENTRY);

    // If the entry is unused or if the new quality >= the stored quality, then store the new value. Note: It is assumed to be
    // better to replace values of equal quality in order to dispose of old entries that may no longer be relevant.

    if (isUnused || (quality >= entry.q_))
    {
        entry.fingerprint_ = hash;
        entry.value_       = value;
        entry.q_           = quality;
        entry.age_         = 0; // Reset age

#if defined(ANALYSIS_TRANSPOSITION_TABLE)

        // For tracking the number of used entries

        if (isUnused)
            ++analysisData_.usage;
        else if (entry.fingerprint_ == hash)
            ++analysisData_.refreshed;
        else
            ++analysisData_.overwritten;

#endif      // defined(ANALYSIS_TRANSPOSITION_TABLE)
    }
    else
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        ++analysisData_.rejected;
#endif
    }
}

//! @param  state   State whose value is to be stored
//! @param  value   Value to be stored
//! @param  quality Quality of the value

void TranspositionTable::set(GameState const & state, float value, int quality)
{
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.updateCount;
#endif

    uint64_t hash = state.fingerprint();
    assert(hash != Entry::UNUSED_ENTRY);
    Entry & entry = find(hash);

    // Store the state, value and quality
    entry.fingerprint_ = hash;
    entry.value_       = value;
    entry.q_           = quality;
    entry.age_         = 0; // Reset age

#if defined(ANALYSIS_TRANSPOSITION_TABLE)

    // For tracking the number of used entries

    if (entry.fingerprint_ == Entry::UNUSED_ENTRY)
        ++analysisData_.usage;
    else if (entry.fingerprint_ == hash)
        ++analysisData_.refreshed;
    else
        ++analysisData_.overwritten;

#endif  // defined(ANALYSIS_TRANSPOSITION_TABLE)
}

//! The T-table is persistent. So in order to gradually dispose of entries that are no longer relevant, entries that have not been
//! referenced for a while are removed.

void TranspositionTable::age()
{
    for (auto & entry : table_)
    {
        if (entry.fingerprint_ != Entry::UNUSED_ENTRY)
        {
            ++entry.age_;
            if (entry.age_ > MAX_AGE)
            {
                entry.fingerprint_ = Entry::UNUSED_ENTRY;
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
                --analysisData_.usage;
#endif
            }
        }
    }
}

#if defined(ANALYSIS_TRANSPOSITION_TABLE)

TranspositionTable::AnalysisData::AnalysisData()
    : usage(0)
{
    reset();
}

void TranspositionTable::AnalysisData::reset()
{
    checkCount     = 0;
    updateCount    = 0;
    hitCount       = 0;
    collisionCount = 0;
    rejected       = 0;
    overwritten    = 0;
    refreshed      = 0;
//    usage          = 0;    // never reset
}

json TranspositionTable::AnalysisData::toJson() const
{
    json out =
    {
        { "checkCount", checkCount },
        { "updateCount", updateCount },
        { "hitCount", hitCount },
        { "collisionCount", collisionCount },
        { "rejected", rejected },
        { "overwritten", overwritten },
        { "refreshed", refreshed },
        { "usage", usage },
    };

    return out;
}

#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)
} // namespace GamePlayer
