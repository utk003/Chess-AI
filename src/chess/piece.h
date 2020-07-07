// piece.h header guard
#ifndef CHESSAI_CHESS_PIECE_H_
#define CHESSAI_CHESS_PIECE_H_

#include "piece.fwd.h"

#include <string>

#include "game.fwd.h"

// The "piece" namespace: See piece.fwd.h
namespace piece {

// The Piece class: See piece.fwd.h
// This class contains:
//   - Methods to access the piece's color, type, and image file path through color(), type(), and image_file_path(), respectively
//   - A vitual method verifyMove(...) which each extending class changes to accurately evaluate whether the move is valid
//   - A virtual clone() method which returns a deep copy of the current piece
//   - A virtual toString() method that returns an std::string that contains
//     all of the essential information needed to create a deep reconstruction of the Piece
class Piece {
  public:
    Piece(const Piece &p) = delete;
    Piece &operator=(const Piece &p) = delete;

    Piece();
    // DEFAULT DESTRUCTOR

    inline PieceColor color() { return _color; }
    inline PieceType type() { return _type; }
    inline std::string image_file_path() { return _image_file_path; }

    virtual bool verifyMove(game::Move move, game::Board* board);

    virtual Piece* clone();

    virtual std::string toString() { return _type.toString() + " " + _color.toString(); }

    virtual int code();

  protected:
    Piece(PieceColor c, PieceType t);

    const int ONE_MILLION = 1000000;

    PieceColor _color;
    PieceType _type;
    std::string _image_file_path;

    bool checkClearMovePath(game::Board* board, int r1, int c1, int r2, int c2);
};

// The King class: See piece.fwd.h && piece::Piece class
// This class also contains: The method moved(), which is used internally for castling requirements
// This class has the friend PieceManager class, through which its moved flag can be updated
class King : public Piece {
  friend class PieceManager;
  
  public:
    King() = delete;
    King(const King &p) = delete;
    King &operator=(const King &p) = delete;

    King(PieceColor c);
    // DEFAULT DESTRUCTOR
    virtual bool verifyMove(game::Move move, game::Board* board);

    inline bool moved() { return _moved; }

    virtual Piece* clone();

    virtual std::string toString() { return _type.toString() + " " + _color.toString() + " (" + (_moved ? "1": "0") + ")"; }

    virtual int code();

  private:
    bool _moved;
};

// The Queen class: See piece.fwd.h && piece::Piece class
class Queen : public Piece {
  public:
    Queen() = delete;
    Queen(const Queen &p) = delete;
    Queen &operator=(const Queen &p) = delete;

    Queen(PieceColor c);
    // DEFAULT DESTRUCTOR
    virtual bool verifyMove(game::Move move, game::Board* board);

    virtual Piece* clone();
};

// The Rook class: See piece.fwd.h && piece::Piece class
// This class also contains: The method moved(), which is used internally for castling requirements
// This class has the friend PieceManager class, through which its moved flag can be updated
class Rook : public Piece {
  friend class PieceManager;
  
  public:
    Rook() = delete;
    Rook(const Rook &p) = delete;
    Rook &operator=(const Rook &p) = delete;
  
    Rook(PieceColor c);
    // DEFAULT DESTRUCTOR
    virtual bool verifyMove(game::Move move, game::Board* board);

    inline bool moved() { return _moved; }

    virtual Piece* clone();

    virtual std::string toString() { return _type.toString() + " " + _color.toString() + " (" + (_moved ? "1": "0") + ")"; }

    virtual int code();

  private:
    bool _moved;
};

// The Knight class: See piece.fwd.h && piece::Piece class
class Knight : public Piece {
  public:
    Knight() = delete;
    Knight(const Knight &p) = delete;
    Knight &operator=(const Knight &p) = delete;

    Knight(PieceColor c);
    // DEFAULT DESTRUCTOR
    virtual bool verifyMove(game::Move move, game::Board* board);

    virtual Piece* clone();
};

// The Bishop class: See piece.fwd.h && piece::Piece class
class Bishop : public Piece {
  public:
    Bishop() = delete;
    Bishop(const Bishop &p) = delete;
    Bishop &operator=(const Bishop &p) = delete;
  
    Bishop(PieceColor c);
    // DEFAULT DESTRUCTOR
    virtual bool verifyMove(game::Move move, game::Board* board);

    virtual Piece* clone();
};

// The Pawn class: See piece.fwd.h && piece::Piece class
// This class also contains: The method moved2x(), which is used internally for en passante requirements
// This class has the friend PieceManager class, through which its moved2x flag can be updated
class Pawn : public Piece {
  friend class PieceManager;

  public:
    Pawn() = delete;
    Pawn(const Pawn &p) = delete;
    Pawn &operator=(const Pawn &p) = delete;
  
    Pawn(PieceColor c);
    // DEFAULT DESTRUCTOR
    virtual bool verifyMove(game::Move move, game::Board* board);

    inline bool moved2x() { return _moved2x; }

    virtual Piece* clone();

    virtual std::string toString() { return _type.toString() + " " + _color.toString() + " (" + (_moved2x ? "1": "0") + ")"; }

    virtual int code();

  private:
    bool _moved2x;
};

// The PieceManager class: See piece.fwd.h && piece::King, piece::Rook, and piece::Pawn classes
class PieceManager {
  public:
    PieceManager(const PieceManager &p) = delete;
    PieceManager &operator=(const PieceManager &p) = delete;
  
  protected:
    PieceManager() {}

    inline void update_moved_flag(King* king, bool newValue) const { king -> _moved = newValue; }
    inline void update_moved_flag(Rook* rook, bool newValue) const { rook -> _moved = newValue; }
    inline void update_moved2x_flag(Pawn* pawn, bool newValue) const { pawn -> _moved2x = newValue; }
};

}

// end piece.h header guard
#endif // CHESSAI_CHESS_PIECE_H_