#pragma once

namespace GamePlayer
{
class GameState;

//! An abstract static evaluation function.
//!
//! StaticEvaluator is used by GameTree to compute a preliminary estimated value for a state.

class StaticEvaluator
{
public:
    virtual ~StaticEvaluator() = default;

    //! Returns a value for the given state.
    //!
    //! A higher value must be better for the first player and a lower value must be better for the second player. The returned
    //! value must be between secondPlayerWinsValue() and firstPlayerWinsValue(), unless the state is a win/loss. Returning
    //! firstPlayerWinsValue() or secondPlayerWinsValue() indicates that the state is a win/loss.
    //!
    //! @param  state   State to be evaluated
    //!
    //! @return The value of the state
    //!
    //! @note   This function must be overridden
    virtual float evaluate(GameState const & state) const = 0;

    //! Returns the value of a winning state for the first player.
    //!
    //! Any value greater than or equal to this value indicates a win for the first player. The returned value must be invariant.
    //! It must be higher than any non-winning value returned by evaluate() for the first player, but it must be less than
    //! std::numeric_limits<float>::max().
    //!
    //! @return The value of a state in which the first player has won
    //!
    //! @note   This function must be overridden
    virtual float firstPlayerWinsValue() const = 0;

    //! Returns the value of a winning state for the second player.
    //!
    //! Any value less than or equal to this value indicates a win for the second player. The returned value must be invariant.
    //! It must be lower than any non-winning value returned by evaluate() for the second player, but it must be greater than
    //! std::numeric_limits<float>::max().
    //!
    //! @return The value of a state in which the second player has won
    //!
    //! @note   This function must be overridden
    virtual float secondPlayerWinsValue() const = 0;
};
} // namespace GamePlayer
