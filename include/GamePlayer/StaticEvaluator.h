#pragma once

#if !defined(GAMEPLAYER_STATICEVALUATOR_H)
#define GAMEPLAYER_STATICEVALUATOR_H

namespace GamePlayer
{
class GameState;

class StaticEvaluator
{
public:

    //! Returns a value for the given state.
    //!
    //! A higher value must be better for the first player and a lower value must be better for the second player. The returned
    //! value must be greater than or equal to secondPlayerWins() and less than or equal to firstPlayerWins(). Returning
    //! firstPlayerWins() or secondPlayerWins() indicates an expected win for that player.
    //!
    //! @param  state   State to be evaluated
    //!
    //! @note   This function must be overridden
    virtual float evaluate(GameState const & state) const = 0;

    //! Returns the value of a winning state for the first player.
    //!
    //! The returned value must be invariant. It must be higher than any other non-winning value returned by evaluate(), but it
    //! must be comparably less than std::numeric_limits<float>::max().
    //!
    //! @note   This function must be overridden
    virtual float firstPlayerWins() const = 0;

    //! Returns the value of a winning state for the second player.
    //!
    //! The returned value must be invariant. It must be lower than any other non-wining value returned by evaluate(), but it
    //! must be comparably greater than -std::numeric_limits<float>::max().
    //!
    //! @note   This function must be overridden
    virtual float secondPlayerWins() const = 0;
};
} // namespace GamePlayer

#endif // !defined(GAMEPLAYER_STATICEVALUATOR_H)
