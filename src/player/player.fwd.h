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

#ifndef CHESS_AI_PLAYER_PLAYER_FWD_H_
#define CHESS_AI_PLAYER_PLAYER_FWD_H_

#include <string>

#include "../chess/piece.fwd.h"
#include "../chess/game.fwd.h"
#include "../util/assert_util.h"

namespace player {

class Player;
class HumanPlayer;
class RandomPlayer;
class MinimaxPlayer;
class AlphaBetaPlayer;
class MonteCarloPlayer;

// Player Type "enum"
class PlayerType {
  public:
    enum Type {
      HUMAN, RANDOM, MINIMAX, AB_PRUNING, MCTS, AI
    };

    PlayerType() = default;
    PlayerType(Type t) { value = t; }

    constexpr operator Type() const { return value; }
    explicit operator bool() = delete;

    constexpr bool operator==(PlayerType c) const { return value == c.value; }
    constexpr bool operator!=(PlayerType c) const { return value != c.value; }

    constexpr bool isHumanPlayer() const { return value == HUMAN; }
    constexpr bool isRandomPlayer() const { return value == RANDOM; }
    constexpr bool isMinimaxPlayer() const { return value == MINIMAX; }
    constexpr bool isAlphaBetaPruningPlayer() const { return value == AB_PRUNING; }
    constexpr bool isMonteCarloTreeSearchPlayer() const { return value == MCTS; }
    constexpr bool isAIPlayer() const { return value == AI; }

    inline Player *getPlayerOfType(game::Game *game, piece::PieceColor color) const {
      return getPlayerOfType(*this, game, color);
    }
    static Player *getPlayerOfType(PlayerType type, game::Game *game, piece::PieceColor color);

    std::string toString() const {
      switch (value) {
        case HUMAN:
          return "Human Player";
        case RANDOM:
          return "Random Player";
        case MINIMAX:
          return "Minimax Player";
        case AB_PRUNING:
          return "Alpha-Beta Pruning Player";
        case MCTS:
          return "Monte Carlo Tree Search Player";
        case AI:
          return "AI Network Player";

        default: FATAL_ASSERT
      }
    }

  private:
    Type value;
};

}

#endif // CHESS_AI_PLAYER_PLAYER_FWD_H_