#include "piece.h"

#include <string>
#include <sstream>

#include "game.h"
#include "../util/string_util.h"
#include "../util/math_util.h"

// PieceType class
piece::Piece *piece::PieceType::getPieceOfType(PieceType t, PieceColor c) {
  switch (t) {
    case KING:
      return new King(c);
    case QUEEN:
      return new Queen(c);
    case ROOK:
      return new Rook(c);
    case KNIGHT:
      return new Knight(c);
    case BISHOP:
      return new Bishop(c);
    case PAWN:
      return new Pawn(c);

    case NONE:
      return new Piece();

    default: fatal_assert();
  }
}

// Piece Class
piece::Piece::Piece() {
  _color = piece::PieceColor::NONE;
  _type = piece::PieceType::NONE;

  _image_file_path = "transparent.png";
}

piece::Piece::Piece(piece::PieceColor c, piece::PieceType t) {
  // Must be a valid color -AND- Must be a non-empty piece
  if (!c.isColored() || t.isEmpty()) fatal_assert();

  // Create a new Piece with the given color and type
  _color = c;
  _type = t;

  std::stringstream ss;
  ss << c << "_" << t << ".png";
  _image_file_path = ss.str();
}

bool piece::Piece::verifyMove(const game::Move &move, game::Board *board) { return false; }

bool piece::Piece::checkClearMovePath(game::Board *board, int r1, int c1, int r2, int c2) {
  int stepr = math::sgn(r2 - r1), stepc = math::sgn(c2 - c1);
  int r = r1 + stepr, c = c1 + stepc;
  while (r != r2 || c != c2) {
    if (!board->getPiece(r, c)->type().isEmpty())
      return false;

    r += stepr;
    c += stepc;
  }
  return true;
}

piece::Piece *piece::Piece::clone() const {
  auto *newPiece = new Piece();

  newPiece->_color = _color;
  newPiece->_type = _type;
  newPiece->_image_file_path = _image_file_path;

  return newPiece;
}

int piece::Piece::code() const {
  return _type.typeCode() * _color.colorCode();
}

// King Class
piece::King::King(piece::PieceColor c) : piece::Piece::Piece(c, piece::PieceType::KING) { _moved = false; }

bool piece::King::verifyMove(const game::Move &move, game::Board *board) {
  int r1 = move.startingRow(), r2 = move.endingRow(), c1 = move.startingColumn(), c2 = move.endingColumn();
  if (abs(r1 - r2) <= 1 && abs(c1 - c2) <= 1) // normal 1-step king move (check/checkmate checks are done elsewhere)
    return true;

  // Otherwise, check for sideways 2 step by king <--> castling
  if (abs(r1 - r2) != 0 || abs(c1 - c2) != 2) // To castle, king moves sideways 2 steps
    return false;

  if (moved())
    return false; // king must not have moved yet

  int rookCol = (c2 > c1) * 7; // if c2 > c1, then king moved right, so rookCol = 7; else, rookCol = 0
  piece::Piece *piece = board->getPiece(r1, rookCol);

  if (!piece->type().isRook() || piece->color() != _color)
    return false;

  if (((piece::Rook *) piece)->moved())
    return false;

  if (!board->isPositionSafe(r1, c1, _color) || !board->isPositionSafe(r2, c2, _color))
    return false; // old and new squares must be safe/check-free

  int c = (c1 + c2) / 2;
  // middle square must be empty and safe/check free
  return board->getPiece(r1, c)->type().isEmpty() && board->isPositionSafe(r1, c, _color);
}

piece::Piece *piece::King::clone() const {
  King *newPiece = new King(_color);
  newPiece->_moved = _moved;
  return newPiece;
}

int piece::King::code() const {
  return _color.colorCode() * (_type.typeCode() + _moved * piece::PieceType::TYPE_CODE_DIFFERENCE / 2);
}

// Queen Class
piece::Queen::Queen(piece::PieceColor c) : piece::Piece::Piece(c, piece::PieceType::QUEEN) {}

bool piece::Queen::verifyMove(const game::Move &move, game::Board *board) {
  int r1 = move.startingRow(), r2 = move.endingRow(), c1 = move.startingColumn(), c2 = move.endingColumn();
  int dr = abs(r1 - r2), dc = abs(c1 - c2);

  return (dr == 0 || dc == 0 || dr == dc) &&
         checkClearMovePath(board, r1, c1, r2, c2); // queen moved diagonally or along axis && path is clear
}

piece::Piece *piece::Queen::clone() const {
  return new Queen(_color);
}

// Rook Class
piece::Rook::Rook(piece::PieceColor c) : piece::Piece::Piece(c, piece::PieceType::ROOK) { _moved = false; }

bool piece::Rook::verifyMove(const game::Move &move, game::Board *board) {
  int r1 = move.startingRow(), r2 = move.endingRow(), c1 = move.startingColumn(), c2 = move.endingColumn();
  return (abs(r1 - r2) == 0 || abs(c1 - c2) == 0) &&
         checkClearMovePath(board, r1, c1, r2, c2); // rook moved along axis && path is clear
}

piece::Piece *piece::Rook::clone() const {
  Rook *newPiece = new Rook(_color);
  newPiece->_moved = _moved;
  return newPiece;
}

int piece::Rook::code() const {
  return _color.colorCode() * (_type.typeCode() + _moved * piece::PieceType::TYPE_CODE_DIFFERENCE / 2);
}

// Knight Class
piece::Knight::Knight(piece::PieceColor c) : piece::Piece::Piece(c, piece::PieceType::KNIGHT) {}

bool piece::Knight::verifyMove(const game::Move &move, game::Board *board) {
  int r1 = move.startingRow(), r2 = move.endingRow(), c1 = move.startingColumn(), c2 = move.endingColumn();
  int dr = abs(r1 - r2), dc = abs(c1 - c2);
  return (dr == 2 && dc == 1) || (dr == 1 && dc == 2); // knight moved in a (2,1) or (1,2) jump pattern
}

piece::Piece *piece::Knight::clone() const {
  return new Knight(_color);
}

// Bishop Class
piece::Bishop::Bishop(piece::PieceColor c) : piece::Piece::Piece(c, piece::PieceType::BISHOP) {}

bool piece::Bishop::verifyMove(const game::Move &move, game::Board *board) {
  int r1 = move.startingRow(), r2 = move.endingRow(), c1 = move.startingColumn(), c2 = move.endingColumn();
  return (abs(r1 - r2) == abs(c1 - c2)) &&
         checkClearMovePath(board, r1, c1, r2, c2); // bishop moved diagonally && path is clear
}

piece::Piece *piece::Bishop::clone() const {
  return new Bishop(_color);
}

// Pawn Class
piece::Pawn::Pawn(piece::PieceColor c) : piece::Piece::Piece(c, piece::PieceType::PAWN) { _moved2x = false; }

bool piece::Pawn::verifyMove(const game::Move &move, game::Board *board) {
  int r1 = move.startingRow(), r2 = move.endingRow(), c1 = move.startingColumn(), c2 = move.endingColumn();

  int homeRow = _color.isWhite() ? 1: 6;
  int moveDir = _color.isWhite() ? 1: -1;

  if (math::sgn(r2 - r1) != moveDir) // pawn must have moved forward
    return false;

  if (c1 == c2) {
    if (!board->getPiece(r2, c2)->type().isEmpty()) // make sure pawn isn't capturing anything
      return false;

    switch (abs(r1 - r2)) {
      case 2:
        return r1 == homeRow && board->getPiece(r1 + moveDir,
                                                c1)->type().isEmpty(); // make sure pawn hasn't moved yet && middle square isn't occupied

      case 1:
        return true;

      default:
        return false; // pawn cannot move more than 2 steps forward
    }
  } else {
    if (abs(c1 - c2) != 1 || abs(r1 - r2) != 1) // if pawn is not moving diagonally with step dist 1, then illegal move
      return false;

    if (board->getPiece(r2, c2)->type().isEmpty()) {
      if (r1 != homeRow + 3 * moveDir) // Must be in fifth row to en passante
        return false;

      piece::Piece *jumpPiece = board->getPiece(r1, c2);
      if (!jumpPiece->type().isPawn()) // Must en passante pawn
        return false;

      return ((piece::Pawn *) jumpPiece)->moved2x(); // En passante valid iff pawn just moved 2x - game::Game must update constantly
    }
    return true; // Otherwise, its good -> no en passante, just classic capture with pawn
  }
}

piece::Piece *piece::Pawn::clone() const {
  Pawn *newPiece = new Pawn(_color);
  newPiece->_moved2x = _moved2x;
  return newPiece;
}

int piece::Pawn::code() const {
  return _color.colorCode() * (_type.typeCode() + _moved2x * piece::PieceType::TYPE_CODE_DIFFERENCE / 2);
}

// PieceManager class
piece::Piece *piece::PieceManager::getPieceOfTypeAndColor(PieceType t, PieceColor c) {
  return t.getPieceOfType(c);
}
piece::Piece *piece::PieceManager::getPieceOfTypeAndColor(PieceType t, PieceColor c, bool mod) {
  Piece *p = t.getPieceOfType(c);
  switch (t) {
    case PieceType::KING:
      update_flag((King *) p, mod);

    case PieceType::ROOK:
      update_flag((Rook *) p, mod);

    case PieceType::PAWN:
      update_flag((Pawn *) p, mod);

    default:
      return p;
  }
}

// Piece to/from iostream
namespace piece {

std::istream &operator>>(std::istream &input, Piece *&p) {
  delete p;

  PieceType t = PieceType::NONE;
  PieceColor c = PieceColor::NONE;
  std::string mod;

  input >> t >> c >> mod;
  p = PieceManager::getPieceOfTypeAndColor(t, c, string::to_bool(mod));

  return input;
}
std::ostream &operator<<(std::ostream &output, Piece *&p) {
  switch (p->_type) {
    case PieceType::KING: {
      auto *k = (King *) p;
      output << k;
      break;
    }

    case PieceType::QUEEN: {
      auto *q = (Queen *) p;
      output << q;
      break;
    }

    case PieceType::ROOK: {
      auto *r = (Rook *) p;
      output << r;
      break;
    }

    case PieceType::KNIGHT: {
      auto *k = (Knight *) p;
      output << k;
      break;
    }

    case PieceType::BISHOP: {
      auto *b = (Bishop *) p;
      output << b;
      break;
    }

    case PieceType::PAWN: {
      auto *pawn = (Pawn *) p;
      output << pawn;
      break;
    }

    case piece::PieceType::NONE:
      output << p->_type << " " << p->_color << " " << ".";
      break;

    default: fatal_assert();
  }
  return output;
}

std::ostream &operator<<(std::ostream &output, King *&k) {
  output << k->_type << " " << k->_color << " " << string::from_bool(k->_moved);
  return output;
}

std::ostream &operator<<(std::ostream &output, Queen *&q) {
  output << q->_type << " " << q->_color << " " << ".";
  return output;
}

std::ostream &operator<<(std::ostream &output, Rook *&r) {
  output << r->_type << " " << r->_color << " " << string::from_bool(r->_moved);
  return output;
}

std::ostream &operator<<(std::ostream &output, Knight *&k) {
  output << k->_type << " " << k->_color << " " << ".";
  return output;
}

std::ostream &operator<<(std::ostream &output, Bishop *&b) {
  output << b->_type << " " << b->_color << " " << ".";
  return output;
}

std::ostream &operator<<(std::ostream &output, Pawn *&p) {
  output << p->_type << " " << p->_color << " " << string::from_bool(p->_moved2x);
  return output;
}

}