#include "GamePlayer/TranspositionTable.h"

#include <nlohmann/json.hpp>

#include <cassert>

using json = nlohmann::json;

namespace GamePlayer
{

//! @param  size    Number of entries in the table
//! @param  maxAge  Maximum age of entries allowed in the table

TranspositionTable::TranspositionTable(size_t size, int maxAge)
    : table_(size)
    , maxAge_(maxAge)
{
    assert(size > 0);
    assert(0 < maxAge && maxAge < std::numeric_limits<decltype(Entry::age_)>::max());

    // Invalidate all entries in the table
    for (auto & entry : table_)
    {
        entry.clear();
    }
}

//! Returns the value and quality of a state if they are stored in the table and its quality is above the specified minimum (if
//! specified). Otherwise, nullopt is returned.
//!
//! @param  fingerprint     Fingerprint of state to be checked for
//! @param  minQ            Minimum quality. If less 0, it is not used. (default: -1, i.e., no minimum)
//!
//! @return optional CheckResult

std::optional<TranspositionTable::CheckResult> TranspositionTable::check(uint64_t fingerprint, int minQ /* = -1*/) const
{
    assert(fingerprint != Entry::UNUSED);
    assert(minQ < std::numeric_limits<decltype(Entry::q_)>::max());

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.checkCount;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

    // Find the entry
    Entry const & entry = find(fingerprint);
    if (entry.fingerprint_ != fingerprint)
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        if (entry.fingerprint_ != Entry::UNUSED)
            ++analysisData_.collisionCount;
#endif                       // defined(ANALYSIS_TRANSPOSITION_TABLE)
        return std::nullopt; // Not found
    }

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.hitCount;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

    // Check the quality
    if (minQ >= 0 && entry.q_ < minQ)
        return std::nullopt; // Insufficient quality

    entry.age_ = 0; // The entry was accessed so reset its age

    return CheckResult{entry.value_, entry.q_};
}

//! @param  fingerprint     Fingerprint of state to be stored
//! @param  value           Value to be stored
//! @param  quality         Quality of the value

void TranspositionTable::update(uint64_t fingerprint, float value, int quality)
{
    assert(fingerprint != Entry::UNUSED);
    assert(0 <= quality && quality < std::numeric_limits<decltype(Entry::q_)>::max());

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.updateCount;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

    // Find the entry for the fingerprint
    Entry & entry    = find(fingerprint);
    bool    isUnused = (entry.fingerprint_ == Entry::UNUSED);

    // If the entry is unused or if the new quality >= the stored quality, then store the new value. Note: It is assumed to be
    // better to replace values of equal quality in order to dispose of old entries that may no longer be relevant.

    if (isUnused || (quality >= entry.q_))
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        if (isUnused)
            ++analysisData_.usage;
        else if (entry.fingerprint_ == fingerprint)
            ++analysisData_.refreshed;
        else
            ++analysisData_.overwritten;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)
        entry = Entry{fingerprint, value, static_cast<decltype(Entry::q_)>(quality), 0};
    }
    else
    {
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
        ++analysisData_.rejected;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)
    }
}

//! @param  fingerprint     Fingerprint of state to be stored
//! @param  value           Value to be stored
//! @param  quality         Quality of the value

void TranspositionTable::set(uint64_t fingerprint, float value, int quality)
{
    assert(fingerprint != Entry::UNUSED);
    assert(0 <= quality && quality < std::numeric_limits<decltype(Entry::q_)>::max());

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    ++analysisData_.updateCount;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

    // Find the entry for the fingerprint
    Entry & entry = find(fingerprint);

#if defined(ANALYSIS_TRANSPOSITION_TABLE)
    if (entry.fingerprint_ == Entry::UNUSED)
        ++analysisData_.usage;
    else if (entry.fingerprint_ == fingerprint)
        ++analysisData_.refreshed;
    else
        ++analysisData_.overwritten;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

    // Store the state, value and quality
    entry = Entry{fingerprint, value, static_cast<decltype(Entry::q_)>(quality), 0};
}

//! The T-table is persistent. So in order to gradually dispose of entries that are no longer relevant, entries that have not been
//! referenced for a while are removed.

void TranspositionTable::age()
{
    for (auto & entry : table_)
    {
        if (entry.fingerprint_ != Entry::UNUSED)
        {
            ++entry.age_;
            if (entry.age_ > maxAge_)
            {
                entry.fingerprint_ = Entry::UNUSED;
#if defined(ANALYSIS_TRANSPOSITION_TABLE)
                --analysisData_.usage;
#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)
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
    // Note: usage is intentionally not reset
}

json TranspositionTable::AnalysisData::toJson() const
{
    return json{{"checkCount", checkCount},
                {"updateCount", updateCount},
                {"hitCount", hitCount},
                {"collisionCount", collisionCount},
                {"rejected", rejected},
                {"overwritten", overwritten},
                {"refreshed", refreshed},
                {"usage", usage}};
}

#endif // defined(ANALYSIS_TRANSPOSITION_TABLE)

} // namespace GamePlayer
