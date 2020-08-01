#ifndef CHESS_AI_CHESS_GAME_H_
#define CHESS_AI_CHESS_GAME_H_

#include "game.fwd.h"

#include <execinfo.h>
#include <cstdio>

#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <stack>

#include "piece.h"
#include "../player/player.h"

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

    [[nodiscard]] inline int length() const { return _length; }
    [[nodiscard]] inline int width() const { return _width; }

    [[nodiscard]] inline std::vector<piece::Piece *> pieces() const { return _pieces; }

    [[nodiscard]] inline piece::Piece *getPiece(int r, int c) const {
      assert(isValidPosition(r, c));
      return _pieces[locMap(r, c)];
    }

    [[nodiscard]] constexpr bool isValidPosition(int r, int c) const {
      return 0 <= r && r < _length && 0 <= c && c < _width;
    }

    friend std::istream &operator>>(std::istream &input, Board *&b);
    friend std::ostream &operator<<(std::ostream &output, Board *&b);

    [[nodiscard]] int getPositionThreats(int r, int c, piece::PieceColor kingColor) const; // returns # of threats
    [[nodiscard]] inline bool isPositionSafe(int r, int c, piece::PieceColor kingColor) const {
      return getPositionThreats(r, c, kingColor) == 0;
    }

    [[nodiscard]] std::pair<int, int> getKingPosition(piece::PieceColor color) const;
    [[nodiscard]] inline bool isKingSafe(piece::PieceColor color) const {
      std::pair<int, int> coords = getKingPosition(color);
      return isPositionSafe(coords.first, coords.second, color);
    }

    bool canPieceMove(int r, int c, int toR, int toC);

    void getMovesFromSquare(int r, int c, std::vector<game::Move> *moves);
    void getPossibleMoves(std::vector<game::Move> *white, std::vector<game::Move> *black);

    bool doMove(Move *move, Game *game); // See game::Move::doMove()
    void undoMove(Game *game);

    [[nodiscard]] Move *getLastMove() const;

    [[nodiscard]] inline piece::PieceType pawn_upgrade_type() const { return _pawn_upgrade_type; }
    inline void set_pawn_upgrade_type(piece::PieceType type) { _pawn_upgrade_type = type; }

    [[nodiscard]] Board *clone() const;

    void loadFromFile(const std::string &fileName);

  private:
    int _length;
    int _width;
    std::vector<piece::Piece *> _pieces;

    std::stack<Move *> _move_stack;
    piece::PieceType _pawn_upgrade_type{};

    [[nodiscard]] constexpr int locMap(int r, int c) const { return r * _width + c; };
};

class BoardController {
  public:
    BoardController(const BoardController &bc) = delete;
    BoardController &operator=(const BoardController &bc) = delete;

  protected:
    BoardController() = default;
    ~BoardController() = default;

    inline static std::vector<piece::Piece *> &getBoard(Board *board) { return board->_pieces; }
    inline static int locMap(Board *board, int r, int c) { return board->locMap(r, c); }
};

class Move : BoardController, piece::PieceManager {
  public:
    static std::vector<game::Move> getMoves(int r1, int c1, int r2, int c2, Board *b); // Array of Move Pointers
    Move(int r1, int c1, int r2, int c2, piece::PieceType promotionType);

    Move(const Move &m);
    ~Move();

    Move &operator=(const Move &m);

    bool operator==(const Move &m) const {
      return _start_row == m._start_row && _start_col == m._start_col &&
             _end_row == m._end_row && _end_col == m._end_col &&
             _pawn_promotion_type == m._pawn_promotion_type;
    }
    bool operator!=(const Move &m) const { return !(*this == m); }
    bool operator<(const Move &m) const {
      return _start_row < m._start_row ||
             (_start_row == m._start_row && _start_col < m._start_col) ||
             (_start_row == m._start_row && _start_col == m._start_col && _end_row < m._end_row) ||
             (_start_row == m._start_row && _start_col == m._start_col && _end_row == m._end_row &&
              _end_col < m._end_col) ||
             (_start_row == m._start_row && _start_col == m._start_col && _end_row == m._end_row &&
              _end_col < m._end_col && _pawn_promotion_type.typeCode() < m._pawn_promotion_type.typeCode());
    }
    bool operator<=(const Move &m) const { return !(m < *this); }
    bool operator>(const Move &m) const { return m < *this; }
    bool operator>=(const Move &m) const { return !(*this < m); }

    [[nodiscard]] inline int startingRow() const { return _start_row; }
    [[nodiscard]] inline int startingColumn() const { return _start_col; }

    [[nodiscard]] inline int endingRow() const { return _end_row; }
    [[nodiscard]] inline int endingColumn() const { return _end_col; }

    [[nodiscard]] inline piece::PieceType pawn_promotion_type() const { return _pawn_promotion_type; }

    bool verify(Board *board) const;
    bool isAttack(Board *board) const;

    bool doMove(Board *board); // true iff piece is captured
    void undoMove(Board *board) const;

    [[nodiscard]] std::string toString() const;

  private:
    int _start_row, _start_col;
    int _end_row, _end_col;

    piece::PieceType _pawn_promotion_type{};

    std::map<std::pair<int, int>, piece::Piece *> _other_replaced_pieces;
    std::map<std::pair<int, int>, bool> _piece_setting_changes;

    void addReplacedPiece(int r, int c, piece::Piece *p);

    void addSettingChange(int r, int c, bool oldSetting);

    static void updateSetting(Board *board, int r, int c, bool setting);
};

class Game : BoardController, piece::PieceManager {
    friend class Board;

  public:
    Game() = delete;
    Game(const Game &g) = delete;
    Game &operator=(const Game &g) = delete;

    Game(int length, int width);
    explicit Game(Board *b);

    ~Game();

    [[nodiscard]] inline int length() const { return _board->length(); }
    [[nodiscard]] inline int width() const { return _board->width(); }

    [[nodiscard]] inline Board *board() const { return _board; }
    [[nodiscard]] inline piece::Piece *getPiece(int r, int c) const { return _board->getPiece(r, c); }

    [[nodiscard]] inline piece::PieceColor getCurrentColor() const { return _current_player_color; }

    [[nodiscard]] inline int selected_x() const { return _selected_x; }
    [[nodiscard]] inline int selected_y() const { return _selected_y; }

    [[nodiscard]] inline bool isMoveOver() const { return _is_move_complete; }
    [[nodiscard]] inline bool isOver() const { return _over; }

    [[nodiscard]] inline GameResult getResult() const { return _result; }

    [[nodiscard]] inline player::Player *white_player() const { return _white_player; }
    [[nodiscard]] inline player::Player *black_player() const { return _black_player; }

    void setPlayer(piece::PieceColor color, player::PlayerType type);

    void startGame();
    void endGame();
    void waitForDelete() const;

    [[nodiscard]] Game *clone() const;

    [[nodiscard]] std::vector<Move> possibleMoves() const;
    [[nodiscard]] std::vector<Move> possibleMoves(piece::PieceColor color) const;



    void selectSquare(int x, int y);
    bool tryMove(const Move &move);

  private:
    Board *_board;
    int _selected_x, _selected_y;
    piece::PieceColor _current_player_color{};

    player::Player *_white_player;
    player::Player *_black_player;

    bool _is_move_complete, _is_ready_to_delete;

    void playGame() {
      while (!_over) {
        _is_move_complete = false;
        if (_current_player_color.isWhite())
          _white_player->playNextMove();
        else
          _black_player->playNextMove();
      }
      _is_ready_to_delete = true;
    }

    static inline void play_game(Game *g) { g->playGame(); }

    bool _started, _over;

    std::vector<Move> _white_moves, _black_moves;

    void generatePossibleMoveVectors();

    int _moves_since_last_capture;

    GameResult _result{};
};

}

#endif // CHESS_AI_CHESS_GAME_H_