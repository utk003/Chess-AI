// piece.fwd.h header guard
#ifndef CHESSAI_CHESS_PIECE_FWD_H_
#define CHESSAI_CHESS_PIECE_FWD_H_

#include <string>

// The "piece" namespace is for all piece related classes/methods:
//   - PieceColor and PieceType "enums"
//   - Piece, King, Queen, Rook, Knight, Bishop, and Pawn classes for the actual pieces
//   - PieceManager manager class
namespace piece {

// The generic Piece class (extended by other 6 pieces)
// This class should only be instantiated for the empty cells of the chess board:
// Any of the 6 specific pieces should be instantiated through their own specific constructors
class Piece;

// The 6 piece specific classes: All 6 extend Piece
class King;
class Queen;
class Rook;
class Knight;
class Bishop;
class Pawn;

// This PieceManager class allows external classes to change internal settings in Pieces
// Simply extend the PieceManager class
class PieceManager;

// The PieceColor "enum":
//   - Black
//   - White
//   - None
//
// Comes with utilities for working with:
//   - The == and != operators
//   - Switch case statements
//   - The ! operator (White <--> Black, None <--> None)
//   - The methods isWhite(), isBlack(), and isColored()
//   - The methods toString() and fromString(...) for converting to and from std::string
class PieceColor {
  public:
    enum Color { BLACK, WHITE, NONE };

    PieceColor() = default;
    PieceColor(Color c) { value = c; }

    constexpr operator Color() const { return value; }
    explicit operator bool() = delete;

    constexpr bool operator==(PieceColor c) const { return value == c.value; }
    constexpr bool operator!=(PieceColor c) const { return value != c.value; }

    // none -> none, black -> white, white -> black
    inline PieceColor operator!() const { return value == NONE ? NONE: value == BLACK ? WHITE: BLACK; }

    constexpr bool isWhite() const { return value == WHITE; }
    constexpr bool isBlack() const { return value == BLACK; }
    constexpr bool isColored() const { return value != NONE; }

    std::string toString() const {
      switch (value) {
        case BLACK: return "black";
        case WHITE: return "white";

        case NONE: return "none";
      }
    }

    static PieceColor fromString(std::string s) {
      if (s.compare("black") == 0)
        return BLACK;
      if (s.compare("white") == 0)
        return WHITE;
      return NONE;
    }

    constexpr int colorCode() const {
      switch (value) {
        case BLACK: return -1;
        case WHITE: return 1;
        case NONE: return 0;
      }
    }

  private:
    Color value;
};

// The PieceType "enum":
//   - King
//   - Queen
//   - Rook
//   - Knight
//   - Bishop
//   - Pawn
//   - None
//
// Comes with utilities for working with:
//   - The == and != operators
//   - Switch case statements
//   - The methods isKing(), isQueen(), isRook(), isKnight(), isBishop(), isPawn(), isEmpty()
//   - The methods toString() and fromString(...) for converting to and from std::string
//   - getPieceOfType(...) methods for constructing Pieces from given parameters
//   - minimaxValue(...) methods for getting the Minimax value of a piece (for the Minimax algorithm and derivatives)
class PieceType {
  public:
    enum Type { KING, QUEEN, ROOK, KNIGHT, BISHOP, PAWN, NONE };

    PieceType() = default;
    PieceType(Type t) { value = t; }

    constexpr operator Type() const { return value; }
    explicit operator bool() = delete;

    constexpr bool operator==(PieceType c) const { return value == c.value; }
    constexpr bool operator!=(PieceType c) const { return value != c.value; }

    constexpr bool isKing() const { return value == KING; }
    constexpr bool isQueen() const { return value == QUEEN; }
    constexpr bool isRook() const { return value == ROOK; }
    constexpr bool isKnight() const { return value == KNIGHT; }
    constexpr bool isBishop() const { return value == BISHOP; }
    constexpr bool isPawn() const { return value == PAWN; }
    constexpr bool isEmpty() const { return value == NONE; }

    std::string toString() const {
      switch (value) {
        case KING: return "king";
        case QUEEN: return "queen";
        case ROOK: return "rook";
        case KNIGHT: return "knight";
        case BISHOP: return "bishop";
        case PAWN: return "pawn";
        
        case NONE: return "none";
      }
    }

    inline Piece* getPieceOfType(PieceColor c) { return getPieceOfType(*this, c); }
    static Piece* getPieceOfType(PieceType t, PieceColor c);

    static PieceType fromString(std::string s) {
      if (s.compare("king") == 0)
        return KING;
      if (s.compare("queen") == 0)
        return QUEEN;
      if (s.compare("rook") == 0)
        return ROOK;
      if (s.compare("knight") == 0)
        return KNIGHT;
      if (s.compare("bishop") == 0)
        return BISHOP;
      if (s.compare("pawn") == 0)
        return PAWN;
      return NONE;
    }

    constexpr int minimaxValue() const {
      switch (value) {
        case KING:
          return 100;
        
        case QUEEN:
          return 9;
        
        case ROOK:
          return 5;
        
        case KNIGHT:
        case BISHOP:
          return 3;
        
        case PAWN:
          return 1;
        
        case NONE:
          return 0;
      }
    }

    constexpr int minimaxValue(int r, int c, int homeRow) const {
      switch (value) {
        case KING: // +10 if in home row
          return 100 + 10 * (r == homeRow);
        
        case QUEEN: // +1 for in center 4x4
          return 9 + (2 <= r && r < 6 && 2 <= c && c < 6);
        
        case ROOK: // +1 for not in center 4x4, +2 for enemy end row
          return 5 + !(2 <= r && r < 6 && 2 <= c && c < 6) + (abs(7 - homeRow - r) <= 1);
        
        case KNIGHT: // +1 for 4x4 center, +1 for 5th or 6th row position center
          return 3 + (2 <= r && r < 6 && 2 <= c && c < 6) + (homeRow == 0 ? (4 <= r && r < 6): (2 <= r && r < 4)) * (!(2 <= c && c < 6));
        
        case BISHOP: // +1 for near each main diagonal
          return 3 + (6 <= r + c && r + c <= 8) + (-1 <= r - c && r - c <= 1);
        
        case PAWN: // +1 if in 4x4 center or 2x8 middle rows, +2 if in 2x4 center, +1 if in last square before promotion
          return 1 + (2 <= r && r < 6 && 2 <= c && c < 6) + (3 <= r && r < 5) + (abs(6 - homeRow) == r);
        
        case NONE:
          return 0;
      }
    }

    constexpr int typeCode() const {
      const int TWO_MILLION = 2000000;
      switch (value) {
        case KING: return 1 * TWO_MILLION;
        case QUEEN: return 2 * TWO_MILLION;
        case ROOK: return 3 * TWO_MILLION;
        case KNIGHT: return 4 * TWO_MILLION;
        case BISHOP: return 5 * TWO_MILLION;
        case PAWN: return 6 * TWO_MILLION;

        case NONE: return 0;
      }
    }

  private:
    Type value;
};

}

// end piece.fwd.h header guard
#endif // CHESSAI_CHESS_PIECE_FWD_H_