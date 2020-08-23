#ifndef CHESS_AI_MCTS_NETWORK_DECIDER_H_
#define CHESS_AI_MCTS_NETWORK_DECIDER_H_

#include "decider.fwd.h"

#include <map>
#include <utility>

#include "../chess/piece.fwd.h"
#include "../chess/game.fwd.h"
#include "../util/math_util.h"

namespace decider {

class Decider {
  public:
    Decider(const Decider &d) = delete;
    Decider &operator=(const Decider &d) = delete;

    virtual std::pair<double, std::map<game::Move, double>> prediction(game::Game *game);
    virtual ~Decider() = default; // Do nothing

  protected:
    Decider() = default;

    // Random numbers from [a,b)
    static inline double random(double a, double b) { return math::random(a, b); }

    virtual double predictPosition(game::Board *b) = 0;
};

class Randomizer : public Decider {
  public:
    Randomizer() = default;
    Randomizer(const Randomizer &d) = delete;
    Randomizer &operator=(const Randomizer &d) = delete;

    ~Randomizer() override = default; // Do nothing

  protected:
    double predictPosition(game::Board *b) override;
};

class Minimaxer : public Decider {
  public:
    Minimaxer() = default;
    Minimaxer(const Randomizer &d) = delete;
    Minimaxer &operator=(const Randomizer &d) = delete;

    ~Minimaxer() override = default; // Do nothing

  protected:
    double predictPosition(game::Board *b) override;
};

}

#endif // CHESS_AI_MCTS_NETWORK_DECIDER_H_