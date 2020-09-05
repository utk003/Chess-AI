// piece.h header guard
#ifndef CHESS_AI_CHESS_PIECE_H_
#define CHESS_AI_CHESS_PIECE_H_

#include "piece.fwd.h"

#include <iostream>
#include <string>

#include "game.fwd.h"

// The "piece" namespace: See piece.fwd.h
namespace piece {

// The Piece class: See piece.fwd.h
// This class contains:
//   - Methods to access the piece's color, type, and image file path through color(), type(), and image_file_path(), respectively
//   - A vitual method verifyMove(...) which each extending class changes to accurately evaluate whether the move is valid
//   - A virtual clone() method which returns a deep copy of the current piece
//   - iostream compatibility w/ << and >>
class Piece {
  public:
    Piece(const Piece &p) = delete;
    Piece &operator=(const Piece &p) = delete;

    Piece();
    virtual ~Piece() = default;

    [[nodiscard]] inline PieceColor color() const { return _color; }
    [[nodiscard]] inline PieceType type() const { return _type; }
    [[nodiscard]] inline std::string image_file_path() const { return _image_file_path; }

    virtual bool verifyMove(const game::Move &move, game::Board *board);

    [[nodiscard]] virtual Piece *clone() const;
    [[nodiscard]] virtual int code() const;

    friend std::istream &operator>>(std::istream &input, Piece *&p);
    friend std::ostream &operator<<(std::ostream &output, Piece *&p);

  protected:
    Piece(PieceColor c, PieceType t);

    PieceColor _color;
    PieceType _type;
    std::string _image_file_path;

    static bool checkClearMovePath(game::Board *board, int r1, int c1, int r2, int c2);
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

    explicit King(PieceColor c);
    ~King() override = default;

    bool verifyMove(const game::Move &move, game::Board *board) override;
    [[nodiscard]] inline bool moved() const { return _moved; }

    [[nodiscard]] Piece *clone() const override;
    [[nodiscard]] int code() const override;

    friend std::ostream &operator<<(std::ostream &output, King *&k);

  private:
    bool _moved;
};

// The Queen class: See piece.fwd.h && piece::Piece class
class Queen : public Piece {
  public:
    Queen() = delete;
    Queen(const Queen &p) = delete;
    Queen &operator=(const Queen &p) = delete;

    explicit Queen(PieceColor c);
    ~Queen() override = default;

    bool verifyMove(const game::Move &move, game::Board *board) override;

    [[nodiscard]] Piece *clone() const override;

    friend std::ostream &operator<<(std::ostream &output, Queen *&q);
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

    explicit Rook(PieceColor c);
    ~Rook() override = default;

    // DEFAULT DESTRUCTOR
    bool verifyMove(const game::Move &move, game::Board *board) override;
    [[nodiscard]] inline bool moved() const { return _moved; }

    [[nodiscard]] Piece *clone() const override;
    [[nodiscard]] int code() const override;

    friend std::ostream &operator<<(std::ostream &output, Rook *&r);

  private:
    bool _moved;
};

// The Knight class: See piece.fwd.h && piece::Piece class
class Knight : public Piece {
  public:
    Knight() = delete;
    Knight(const Knight &p) = delete;
    Knight &operator=(const Knight &p) = delete;

    explicit Knight(PieceColor c);
    ~Knight() override = default;

    bool verifyMove(const game::Move &move, game::Board *board) override;

    [[nodiscard]] Piece *clone() const override;

    friend std::ostream &operator<<(std::ostream &output, Knight *&k);
};

// The Bishop class: See piece.fwd.h && piece::Piece class
class Bishop : public Piece {
  public:
    Bishop() = delete;
    Bishop(const Bishop &p) = delete;
    Bishop &operator=(const Bishop &p) = delete;

    explicit Bishop(PieceColor c);
    ~Bishop() override = default;

    bool verifyMove(const game::Move &move, game::Board *board) override;

    [[nodiscard]] Piece *clone() const override;

    friend std::ostream &operator<<(std::ostream &output, Bishop *&b);
};

// The Pawn class: See piece.fwd.h && piece::Piece class
// This class also contains: The method moved2x(), which is used internally for en passant requirements
// This class has the friend PieceManager class, through which its moved2x flag can be updated
class Pawn : public Piece {
    friend class PieceManager;

  public:
    Pawn() = delete;
    Pawn(const Pawn &p) = delete;
    Pawn &operator=(const Pawn &p) = delete;

    explicit Pawn(PieceColor c);
    ~Pawn() override = default;

    bool verifyMove(const game::Move &move, game::Board *board) override;
    [[nodiscard]] inline bool moved2x() const { return _moved2x; }

    [[nodiscard]] Piece *clone() const override;
    [[nodiscard]] int code() const override;

    friend std::ostream &operator<<(std::ostream &output, Pawn *&p);

  private:
    bool _moved2x;
};

// The PieceManager class: See piece.fwd.h && piece::King, piece::Rook, and piece::Pawn classes
class PieceManager {
  public:
    PieceManager(const PieceManager &p) = delete;
    PieceManager &operator=(const PieceManager &p) = delete;

    static Piece *getPieceOfTypeAndColor(PieceType t, PieceColor c);
    static Piece *getPieceOfTypeAndColor(PieceType t, PieceColor c, bool mod);

  protected:
    PieceManager() = default;
    ~PieceManager() = default;

    inline static void update_flag(King *king, bool newValue) { king->_moved = newValue; }
    inline static void update_flag(Rook *rook, bool newValue) { rook->_moved = newValue; }
    inline static void update_flag(Pawn *pawn, bool newValue) { pawn->_moved2x = newValue; }
};

}

// end piece.h header guard
#endif // CHESS_AI_CHESS_PIECE_H_