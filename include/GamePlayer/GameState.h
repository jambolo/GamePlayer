#pragma once

#if !defined(GAMEPLAYER_GAMESTATE_H)
#define GAMEPLAYER_GAMESTATE_H

#include <memory>
#include <vector>
#if defined(ANALYSIS_GAME_STATE)
#include <nlohmann/json.hpp>
#endif

namespace GamePlayer
{
class GameState
{
public:

    GameState()          = default;
    virtual ~GameState() = default;

    //!< IDs of the two players
    enum class PlayerId
    {
        FIRST  = 0,
        SECOND = 1
    };

    //! Returns a fingerprint for this state.
    //!
    //! @note   The fingerprint is assumed to be at least statistically unique.
    //! @note   This function must be overridden.
    virtual uint64_t fingerprint() const = 0;

    //! Generates a list of responses to this state by the next player.
    //!
    //! @param  depth       The depth of the current ply
    //! @param  responses   The list of returned responses
    //!
    //! @note   The caller gains ownership of the states.
    //! @note   This function must be overridden.
    virtual void generateResponses(int depth, std::vector<GamePlayer::GameState *> & responses) const = 0;

    //! Returns the ID of the player that responds to this state
    //!
    //! @note   This function must be overridden.
    virtual PlayerId whoseTurn() const = 0;

    //!< The expected response to this state, or nullptr if not determined
    std::shared_ptr<GameState> response_;
};
} // namespace GamePlayer

#endif // !defined(GAMEPLAYER_GAMESTATE_H)
