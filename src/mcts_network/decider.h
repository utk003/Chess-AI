// ------------------------------------------------------------------------------ //
// MIT License                                                                    //
//                                                                                //
// Copyright (c) 2020 Utkarsh Priyam                                              //
//                                                                                //
// Permission is hereby granted, free of charge, to any person obtaining a copy   //
// of this software and associated documentation files (the "Software"), to deal  //
// in the Software without restriction, including without limitation the rights   //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      //
// copies of the Software, and to permit persons to whom the Software is          //
// furnished to do so, subject to the following conditions:                       //
//                                                                                //
// The above copyright notice and this permission notice shall be included in all //
// copies or substantial portions of the Software.                                //
//                                                                                //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  //
// SOFTWARE.                                                                      //
// ------------------------------------------------------------------------------ //

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