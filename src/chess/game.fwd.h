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

// game.fwd.h header guard
#ifndef CHESS_AI_CHESS_GAME_FWD_H_
#define CHESS_AI_CHESS_GAME_FWD_H_

#include <string>

#include "../util/assert_util.h"

// The "game" namespace is for all game related classes:
//   - Move, Board, Game classes
//   - BoardController manager class
namespace game {

class Board;
class BoardController;

class Move;
class Game;

// Game Result "enum"
class GameResult {
  public:
    enum Result {
      BLACK, WHITE, STALEMATE, NONE
    };

    GameResult() = default;
    GameResult(Result c) { value = c; }

    constexpr operator Result() const { return value; }
    explicit operator bool() = delete;

    constexpr bool operator==(GameResult c) const { return value == c.value; }
    constexpr bool operator!=(GameResult c) const { return value != c.value; }

    constexpr bool isWhiteWin() const { return value == WHITE; }
    constexpr bool isBlackWin() const { return value == BLACK; }
    constexpr bool isStalemate() const { return value == STALEMATE; }
    constexpr bool isGameUndecided() const { return value == NONE; }

    std::string toString() const {
      switch (value) {
        case BLACK:
          return "0-1";
        case WHITE:
          return "1-0";

        case STALEMATE:
          return "0.5-0.5";

        case NONE:
          return "none";

        default: FATAL_ASSERT
      }
    }

    constexpr int evaluate() const {
      switch (value) {
        case BLACK:
          return -1;
        case WHITE:
          return 1;

        case STALEMATE:
        case NONE:
          return 0;

        default: FATAL_ASSERT
      }
    }

  private:
    Result value;
};

}

// end game.fwd.h header guard
#endif // CHESS_AI_CHESS_GAME_FWD_H_