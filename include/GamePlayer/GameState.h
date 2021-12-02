#if !defined(GAMEPLAYER_GAMESTATE_H)
#define GAMEPLAYER_GAMESTATE_H

#pragma once

#include <memory>
#include <vector>
#if defined(ANALYSIS_GAME_STATE)
#include <nlohmann/json_fwd.hpp>
#endif

namespace GamePlayer
{
//! An abstract game state.

class GameState
{
public:

    virtual ~GameState() = default;

    //! IDs of the players.
    enum class PlayerId
    {
        FIRST  = 0,
        SECOND = 1
    };

    //! Returns a fingerprint for this state.
    //!
    //! @note   The fingerprint is assumed to be statistically unique.
    //! @note   This function must be overridden.
    virtual uint64_t fingerprint() const = 0;

    //! Returns the ID of the player that will respond to this state
    //!
    //! @return The ID of the player that responds to this state
    //!
    //! @note   This function must be overridden.
    virtual PlayerId whoseTurn() const = 0;

    //! The expected response to this state, or nullptr if not yet determined
    std::shared_ptr<GameState> response_;
};
} // namespace GamePlayer

#endif // !defined(GAMEPLAYER_GAMESTATE_H)
