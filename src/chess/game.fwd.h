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