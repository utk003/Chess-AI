#ifndef CHESSAI_CHESS_GAME_H_
#define CHESSAI_CHESS_GAME_H_

#include "game.fwd.h"

#include <execinfo.h>
#include <stdio.h>

#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <stack>
#include <thread>

#include "piece.h"
#include "player/player.h"

// The "game" namespace: See game.fwd.h
namespace game {

class Board : piece::PieceManager {
  friend class BoardController;

  public:
    Board() = delete;
    Board(const Board &b) = delete;
    Board &operator=(const Board &b) = delete;

    Board(int board_length, int board_width);
    ~Board();

    inline int length() { return _length; }
    inline int width() { return _width; }

    //      0  1  2  3  4
    //  / \ c ------------>
    // 5 | 25 26 27 28 29
    // 4 | 20 21 22 23 24
    // 3 | 15 16 17 18 19
    // 2 | 10 11 12 13 14
    // 1 |  5  6  7  8  9
    // 0 r  0  1  2  3  4
    inline piece::Piece* getPiece(int r, int c) {
      assert(isValidPosition(r, c));
      int index = locMap(r, c);
      return _pieces[index];
    }
    constexpr bool isValidPosition(int r, int c) { return 0 <= r && r < _length && 0 <= c && c < _width; }

    std::string toString();

    inline bool isPositionSafe(int r, int c, piece::PieceColor kingColor) { return getPositionThreats(r, c, kingColor) == 0; }
    int getPositionThreats(int r, int c, piece::PieceColor kingColor); // returns # of threats
    bool canPieceMove(int r, int c, int toR, int toC);

    std::pair<int, int> getKingPosition(piece::PieceColor color);
    inline bool isKingSafe(piece::PieceColor color) {
      std::pair<int, int> coords = getKingPosition(color);
      return isPositionSafe(coords.first, coords.second, color);
    }

    void getPossibleMoves(std::vector<game::Move>* white, std::vector<game::Move>* black);

    bool doMove(Move* move, Game* game); // See game::Move::doMove()
    void undoMove(Game* game);

    inline void setPawnUpgradeType(piece::PieceType type) { _pawn_upgrade_type = type; }
    inline piece::PieceType pawnUpgradeType() { return _pawn_upgrade_type; }

    Board* clone();

    void loadFromFile(std::string fileName);
  
  private:
    int _length;
    int _width;
    std::vector<piece::Piece*> _pieces;

    std::stack<Move*> _move_stack;

    piece::PieceType _pawn_upgrade_type;

    constexpr int locMap(int r, int c) { return r * _width + c; };
};

class BoardController {
  public:
    BoardController(const BoardController &bc) = delete;
    BoardController &operator=(const BoardController &bc) = delete;
  
  protected:
    BoardController() {}
    inline std::vector<piece::Piece*> &getBoard(Board* board) const { return board -> _pieces; }
    inline int locMap(Board* board, int r, int c) const { return board -> locMap(r, c); }
};

class Move : BoardController, piece::PieceManager {
  public:
    static std::vector<game::Move> getMoves(int r1, int c1, int r2, int c2, Board* b); // Array of Move Pointers
    Move(int r1, int c1, int r2, int c2, piece::PieceType promotionType);
    Move(const Move &m);
    ~Move();

    Move &operator=(const Move &m);

    bool operator==(Move m) const {
      return _start_row == m._start_row && _start_col == m._start_col && _end_row == m._end_row && _end_col == m._end_col;
    }
    bool operator!=(Move m) const {
      return !(*this == m);
    }
    bool operator<(Move m) const {
      return _start_row < m._start_row ||
        (_start_row == m._start_row && _start_col < m._start_col) ||
        (_start_row == m._start_row && _start_col == m._start_col && _end_row < m._end_row) ||
        (_start_row == m._start_row && _start_col == m._start_col && _end_row == m._end_row && _end_col < m._end_col);
    }
    bool operator<=(Move m) const {
      return !(m < *this);
    }
    bool operator>(Move m) const {
      return m < *this;
    }
    bool operator>=(Move m) const {
      return !(*this < m);
    }

    inline int startingRow() { return _start_row; }
    inline int startingColumn() { return _start_col; }

    inline int endingRow() { return _end_row; }
    inline int endingColumn() { return _end_col; }

    inline piece::PieceType pawn_promotion_type() { return _pawn_promotion_type; }

    virtual bool verify(Board* board);

    bool doMove(Board* board); // true iff piece is captured
    void undoMove(Board* board);

    inline std::string toString() const { return "(" + std::to_string(_start_row) + ", " + std::to_string(_start_col) + ") to (" + std::to_string(_end_row) + ", " + std::to_string(_end_col) + ")" + " + " + _pawn_promotion_type.toString(); }
    
  private:
    Move(int r1, int c1, int r2, int c2);

    int _start_row, _start_col;
    int _end_row, _end_col;

    piece::PieceType _pawn_promotion_type;

    std::map<std::pair<int,int>, piece::Piece*> _other_replaced_pieces;
    std::map<std::pair<int,int>, bool> _piece_setting_changes;

    void addReplacedPiece(int r, int c, piece::Piece* p);
    void addSettingChange(int r, int c, bool oldSetting);

    void updateSetting(Board* board, int r, int c, bool setting);
};

// Game Result "enum"
class GameResult {
  public:
    enum Result { BLACK, WHITE, STALEMATE, NONE };

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
        case BLACK: return "0-1";
        case WHITE: return "1-0";

        case STALEMATE: return "0.5-0.5";

        case NONE: return "none";
      }
    }

  private:
    Result value;
};

class Game : BoardController, piece::PieceManager {
  friend class Board;

  public:
    Game() = delete;
    Game(const Game &g) = delete;
    Game &operator=(const Game &g) = delete;

    Game(int length, int width);
    Game(Board* b);
    ~Game();

    inline void setPlayer(piece::PieceColor color, player::PlayerType type) {
      assert(color.isColored());

      player::Player* player = type.getPlayerOfType(this, color);

      if (color.isWhite())
        _white_player = player;
      else // if (color.isBlack())
        _black_player = player;
    }

    inline int length() { return _board -> length(); }
    inline int width() { return _board -> width(); }

    inline Board* board() { return _board; }

    bool processMove(Move m);

    inline std::string toString() { return _board -> toString(); }
    inline piece::Piece* getPiece(int r, int c) { return _board -> getPiece(r, c); }

    void selectSquare(int x, int y);
    bool tryMove(Move move);

    inline int selected_x() { return _selected_x; }
    inline int selected_y() { return _selected_y; }

    inline piece::PieceColor getCurrentColor() { return _current_player_color; }

    void setPawnUpgradeType(piece::PieceType type) { _board -> setPawnUpgradeType(type); }

    std::vector<Move> possibleMoves();
    std::vector<Move> possibleMoves(piece::PieceColor color);

    inline player::Player* white_player() { return _white_player; }
    inline player::Player* black_player() { return _black_player; }

    const void startGame() {
      assert(_white_player != nullptr);
      assert(_black_player != nullptr);

      _started = true;

      generatePossibleMoveVectors();

      std::thread (play_game, this).detach();
    }

    const void endGame() {
      _over = true;
      _is_move_complete = true;
    }

    const void waitForDelete() {
      while (!_is_ready_to_delete)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    inline bool isMoveOver() { return _is_move_complete; }

    inline bool isOver() { return _over; }
    inline GameResult getResult() { return _result; }

    Game* clone();
  
  private:
    Board* _board;
    int _selected_x, _selected_y;
    piece::PieceColor _current_player_color;

    player::Player* _white_player;
    player::Player* _black_player;

    bool _is_move_complete, _is_ready_to_delete;

    const void playGame() {
      while (!_over) {
        _is_move_complete = false;
        if (_current_player_color.isWhite())
          _white_player -> playNextMove();
        else
          _black_player -> playNextMove();
      }
      _is_ready_to_delete = true;
    }

    static inline void play_game(Game* g) { g -> playGame(); }

    bool _started, _over;

    std::vector<Move> _white_moves, _black_moves;
    void generatePossibleMoveVectors();

    int _moves_since_last_capture;

    GameResult _result;
};

}

#endif // CHESSAI_CHESS_GAME_H_