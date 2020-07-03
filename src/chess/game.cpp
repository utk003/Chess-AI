#include "game.h"

#include <string>
#include <cassert>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <utility>

#include "piece.h"
#include "player/player.h"

#define sgn(x) (x > 0) - (x < 0)

// Board Class
game::Board::Board(int l, int w) {
  _length = l;
  _width = w;

  _pieces = new piece::Piece*[l * w];
}

game::Board::~Board() {
  delete[] _pieces;

  while (!_move_stack.empty())
    _move_stack.pop();
}

std::string game::Board::toString() {
  std::string _toString = "";
  for (int i = 0; i < _length * _width; ++i)
    _toString += std::to_string(i) + ": " + _pieces[i] -> toString() + "\n";
  return _toString;
}

int game::Board::getPositionThreats(int r, int c, piece::PieceColor kingColor) {
  assert(isValidPosition(r, c));

  piece::PieceColor enemyColor = !kingColor;
  int x, y, dangerCounter = 0;

  piece::Piece* checkPiece = nullptr;

  // Check axis attacks (queen/rook)
  std::vector<std::vector<int>> axisCheck = { {1,0}, {-1,0}, {0,1}, {0,-1} };
  for (std::vector<int> axis: axisCheck) {
    x = r + axis[0];
    y = c + axis[1];
    while (isValidPosition(x,y)) {
      checkPiece = getPiece(x,y);
      if (checkPiece -> color() == enemyColor) {
        if (checkPiece -> type().isQueen() || checkPiece -> type().isRook())
          ++dangerCounter;
        break;
      }

      if (checkPiece -> color() == kingColor) // Long range attacks can be blocked
        break;
      
      x += axis[0];
      y += axis[1];
    }
  }

  // Check diagonal attacks (queen/bishop)
  std::vector<std::vector<int>> diagCheck = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };
  for (std::vector<int> diag: diagCheck) {
    x = r + diag[0];
    y = c + diag[1];
    while (isValidPosition(x,y)) {
      checkPiece = getPiece(x,y);
      if (checkPiece -> color() == enemyColor) {
        if (checkPiece -> type().isQueen() || checkPiece -> type().isBishop())
          ++dangerCounter;
        break;
      }

      if (checkPiece -> color() == kingColor) // Long range attacks can be blocked
        break;
      
      x += diag[0];
      y += diag[1];
    }
  }

  // Check king attacks
  for (x = r - 1; x <= r + 1; ++x)
    for (y = c - 1; y <= c + 1; ++y) {
      if (!isValidPosition(x, y))
        continue;
      
      checkPiece = getPiece(x,y);
      if (checkPiece -> color() == enemyColor && checkPiece -> type().isKing())
        dangerCounter++;
    }
  
  std::vector<int> pm1 = {1, -1};

  // Check pawn attacks
  x = r + (enemyColor.isWhite() ? -1: 1); // if enemy is white, pawn attacks from below; otherwise from above
  for (int dc: pm1) {
    y = c + dc;

    if (!isValidPosition(x, y))
      continue;
    
    checkPiece = getPiece(x,y);
    if (checkPiece -> color() == enemyColor && checkPiece -> type().isPawn())
      dangerCounter++;
  }

  // Check knight attacks
  std::vector<std::vector<int>> knightMoves = { {1,2}, {2,1} };
  for (std::vector<int> move: knightMoves)
    for (int mr: pm1)
      for (int mc: pm1) {
        x = r + mr * move[0];
        y = c + mc * move[1];

        if (!isValidPosition(x, y))
          continue;
        
        checkPiece = getPiece(x,y);
        if (checkPiece -> color() == enemyColor && checkPiece -> type().isKnight())
          dangerCounter++;
      }
  
  return dangerCounter; // Return total danger count
}

std::pair<int, int> game::Board::getKingPosition(piece::PieceColor color) {
  piece::Piece* checkPiece;
  int xKing = -1, yKing = -1, r, c;

  for (r = 0; r < _length; ++r) {
    for (c = 0; c < _width; ++c) {
      checkPiece = getPiece(r, c);
      if (checkPiece -> type().isKing() && checkPiece -> color() == color) {
        xKing = r;
        yKing = c;
        break;
      }
    }
    if (xKing != -1)
      break;
  }

  return {xKing, yKing};
}

bool game::Board::canPieceMove(int r, int c, int toR, int toC) {
  piece::PieceColor pieceColor = _pieces[locMap(r, c)] -> color(), enemyColor = !pieceColor;
  assert(pieceColor.isColored());

  // get pieces in question
  piece::Piece*& piece1 = _pieces[locMap(r, c)];
  piece::Piece*& piece2 = _pieces[locMap(toR, toC)];

  // save piece 2 (piece 1 is saved into piece 2)
  piece::Piece* copy = piece2;
  piece::Piece* newPiece = new piece::Piece();

  // simulate move
  piece2 = piece1;
  piece1 = newPiece;

  // score threats
  bool isSafe = isKingSafe(pieceColor);

  // undo move
  piece1 = piece2;
  piece2 = copy;

  // free memory
  delete newPiece;
  
  // return result
  return isSafe; // move allowed iff king is safe post-move
}

void game::Board::getPossibleMoves(std::vector<game::Move>* white, std::vector<game::Move>* black) {
  int r1, c1, r2, c2;
  piece::PieceType type;
  for (r1 = 0; r1 < 8; ++r1)
    for (c1 = 0; c1 < 8; ++c1) {
      piece::Piece* piece = getPiece(r1, c1);
      if (!piece -> color().isColored())
        continue;
      
      for (r2 = 0; r2 < 8; ++r2)
        for (c2 = 0; c2 < 8; ++c2) {
          if (r1 == r2 && c1 == c2)
            continue;
          
          std::vector<Move*> moves = Move::getMoves(r1, c1, r2, c2, this);
          for (Move* move: moves) {
            type = move -> pawn_promotion_type();
            if (move -> verify(this)) {
              if (piece -> color().isWhite() && white != nullptr)
                white -> emplace_back(r1, c1, r2, c2, type);
              else if (piece -> color().isBlack() && black != nullptr)
                black -> emplace_back(r1, c1, r2, c2, type);
            }
          }
        }
    }
}

bool game::Board::doMove(Move* move, Game* game) {
  _move_stack.push(move);

  bool isCaptureMove = move -> doMove(this);

  if (game != nullptr)
    game -> _current_player_color = !game -> _current_player_color;
  
  return isCaptureMove;
}
void game::Board::undoMove(Game* game) {
  if (_move_stack.empty())
    return;

  _move_stack.top() -> undoMove(this);
  _move_stack.pop();

  if (game != nullptr)
    game -> _current_player_color = !game -> _current_player_color;
}

game::Board* game::Board::clone() {
  Board* newBoard = new Board(_length, _width);

  int i, total_count = _length * _width;
  for (i = 0; i < total_count; ++i)
    newBoard -> _pieces[i] = _pieces[i] -> clone();

  newBoard -> _pawn_upgrade_type = piece::PieceType::NONE;

  return newBoard;
}

void game::Board::loadFromFile(std::string fileName) {
  std::ifstream in(fileName);
  assert(in.is_open());

  std::string line, t, c;
  piece::PieceType type;
  piece::PieceColor color;
  piece::Piece* piece;

  delete[] _pieces;
  int total_count = _length * _width;
  _pieces = new piece::Piece*[total_count];

  int i, ind1, ind2;
  for (i = 0; i < total_count; ++i) {
    std::getline(in, line);

    ind1 = line.find_first_of(" :");
    assert(i == std::stoi(line.substr(0, ind1)));

    ind1 += 2;
    ind2 = line.find_first_of(" :", ind1);
    t = line.substr(ind1, ind2 - ind1);
    type = piece::PieceType::fromString(t);

    ind1 = ind2 + 1;
    ind2 = line.find_first_of(" :", ind1);
    c = line.substr(ind1, ind2 - ind1);
    color = piece::PieceColor::fromString(c);

    piece = type.getPieceOfType(color);

    if (type.isPawn() || type.isRook() || type.isKing()) {
      bool mod = line[line.find_first_of("(") + 1] == 1; // single character after only "(" in string is either 1 or 0

      if (type.isPawn())
        update_moved2x_flag((piece::Pawn*) piece, mod);
      else if (type.isRook())
        update_moved_flag((piece::Rook*) piece, mod);
      else if (type.isKing())
        update_moved_flag((piece::King*) piece, mod);
    }

    _pieces[i] = piece;
  }
}

// Move Class
std::vector<game::Move*> game::Move::getMoves(int r1, int c1, int r2, int c2, game::Board* b) {
  piece::Piece* piece = b -> getPiece(r1, c1);

  std::vector<game::Move*> moves;

  if (piece -> type().isPawn() && (r2 == 0 || r2 == 7)) {
    moves.push_back(new game::Move(r1, c1, r2, c2, piece::PieceType::QUEEN));
    moves.push_back(new game::Move(r1, c1, r2, c2, piece::PieceType::ROOK));
    moves.push_back(new game::Move(r1, c1, r2, c2, piece::PieceType::KNIGHT));
    moves.push_back(new game::Move(r1, c1, r2, c2, piece::PieceType::BISHOP));
  }
  else
    moves.push_back(new game::Move(r1, c1, r2, c2));

  return moves;
}

game::Move::Move(int r1, int c1, int r2, int c2) : Move(r1, c1, r2, c2, piece::PieceType::NONE) {}
game::Move::Move(int r1, int c1, int r2, int c2, piece::PieceType promotionType) {
  assert(!(r1 == r2 && c1 == c2));

  _start_row = r1;
  _start_col = c1;
  _end_row = r2;
  _end_col = c2;

  _pawn_promotion_type = promotionType;
}

game::Move::Move(const Move &m) {
  _start_row = m._start_row;
  _start_col = m._start_col;
  _end_row = m._end_row;
  _end_col = m._end_col;

  _pawn_promotion_type = m._pawn_promotion_type;
}

game::Move::~Move() {
  _other_replaced_pieces.clear();
  _piece_setting_changes.clear();
}

bool game::Move::verify(Board* board) {
  // Implicit calls to:
  // assert(_board -> isValidPosition(_start_row, _start_col));
  // assert(_board -> isValidPosition(_end_row, _end_col));
  piece::Piece* p1 = board -> getPiece(_start_row, _start_col);
  piece::Piece* p2 = board -> getPiece(_end_row, _end_col);
  
  if (p1 -> type().isEmpty()) // fail if not moving a piece
    return false;
  
  if (p1 -> color() == p2 -> color()) // fail if moving to same color piece
    return false;

  if (!p1 -> verifyMove(*this, board)) // fail if invalid move pattern
    return false;
  
  // succeed iff moving piece does NOT put king in check/checkmate
  return board -> canPieceMove(_start_row, _start_col, _end_row, _end_col);
}

void game::Move::addReplacedPiece(int r, int c, piece::Piece* p) {
  if (p != nullptr)
    _other_replaced_pieces[std::pair<int,int>(r,c)] = p;
}
void game::Move::addSettingChange(int r, int c, bool oldSetting) {
  _piece_setting_changes[std::pair<int,int>(r,c)] = oldSetting;
}

bool game::Move::doMove(Board* board) {
  piece::Piece** pieces = getBoard(board);

  piece::Piece* p;
  int r, c;
  for (r = 0; r < 8; ++r)
    for (c = 0; c < 8; ++c) {
      p = pieces[locMap(board, r, c)];
      if (p -> type().isPawn())
        if (((piece::Pawn*) p) -> moved2x()) {
          addSettingChange(r, c, true);
          update_moved2x_flag((piece::Pawn*) p, false);
        }
    }
  
  int r1 = _start_row, c1 = _start_col, r2 = _end_row, c2 = _end_col;

  piece::Piece*& piece1 = pieces[locMap(board, r1, c1)];
  piece::Piece*& piece2 = pieces[locMap(board, r2, c2)];

  piece::Piece* removedPiece = piece2;
  piece2 = piece1;
  piece1 = new piece::Piece();

  addReplacedPiece(r2, c2, removedPiece);
  switch (removedPiece -> type()) {
    case piece::PieceType::ROOK:
      addSettingChange(r2, c2, ((piece::Rook*) removedPiece) -> moved());
      break;
    
    case piece::PieceType::KING:
      addSettingChange(r2, c2, ((piece::King*) removedPiece) -> moved());
      break;
    
    case piece::PieceType::PAWN:
      addSettingChange(r2, c2, ((piece::Pawn*) removedPiece) -> moved2x());
      break;
    
    default: break;
  }

  piece::Piece* piece = board -> getPiece(r2, c2);
  piece::PieceType type = piece -> type();

  // Piece Settings
  switch (type) {
    case piece::PieceType::ROOK:
      if (!((piece::Rook*) piece) -> moved()) {
        addSettingChange(r1, c1, false);
        update_moved_flag((piece::Rook*) piece, true);
      }
      break;
    
    case piece::PieceType::KING:
      if (!((piece::King*) piece) -> moved()) {
        addSettingChange(r1, c1, false);
        update_moved_flag((piece::King*) piece, true);
      }
      break;
    
    case piece::PieceType::PAWN:
      if (abs(r1 - r2) == 2) {
        addSettingChange(r1, c1, false);
        update_moved2x_flag((piece::Pawn*) piece, true);
      }
      break;
    
    default: break;
  }

  // Other changed pieces
  switch (type) {
    case piece::PieceType::KING:
      if (abs(c1 - c2) == 2) {
        int rookCol = (c2 > c1) * 7; // if c2 > c1, then king moved right, so rookCol = 7; else, rookCol = 0
        int newC = (c1 + c2) / 2;

        piece::Piece*& p1 = pieces[locMap(board, r1, rookCol)];
        addReplacedPiece(r1, rookCol, p1 -> clone()); // clone rook b/c it gets deleted in undoMove() while restoring pieces -> segfault

        piece::Piece*& p2 = pieces[locMap(board, r2, newC)];
        addReplacedPiece(r2, newC, p2);

        p2 = p1;
        p1 = new piece::Piece();
      }
      break;
    
    case piece::PieceType::PAWN:
      if (r2 == 7 || r2 == 0) {
        piece::Piece* newPiece = nullptr;
        int timer = 0;
        piece::PieceType upgrade_type = _pawn_promotion_type;
        while (newPiece == nullptr && timer++ < 600) { // Time out after 600 seconds = 10 minutes
          switch (upgrade_type) {
            case piece::PieceType::QUEEN:
            case piece::PieceType::ROOK:
            case piece::PieceType::KNIGHT:
            case piece::PieceType::BISHOP:
              newPiece = upgrade_type.getPieceOfType(piece -> color());
              break;
            
            default:
              std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for 1 second
              upgrade_type = board -> pawnUpgradeType();
              break;
          }
        }

        assert(newPiece != nullptr);
        
        addReplacedPiece(r1, c1, piece);
        pieces[locMap(board, r2, c2)] = newPiece;
      }
      if (abs(c1 - c2) == 1 && removedPiece -> type().isEmpty()) {
        piece::Piece*& p = pieces[locMap(board, r1, c2)];
        removedPiece = p;
        p = new piece::Piece();

        addReplacedPiece(r1, c2, removedPiece);
      }
      break;
    
    default: break;
  }

  return !removedPiece -> type().isEmpty();
}

void game::Move::updateSetting(Board* board, int r, int c, bool setting) {
  piece::Piece* piece = getBoard(board)[locMap(board, r, c)];

  switch (piece -> type()) {
    case piece::PieceType::ROOK:
      update_moved_flag((piece::Rook*) piece, setting);
      break;
    
    case piece::PieceType::KING:
      update_moved_flag((piece::King*) piece, setting);
      break;
    
    case piece::PieceType::PAWN:
      update_moved2x_flag((piece::Pawn*) piece, setting);
      break;
    
    default: break;
  }
}

void game::Move::undoMove(Board* board) {
  piece::Piece** pieces = getBoard(board);
  int r1 = _start_row, c1 = _start_col, r2 = _end_row, c2 = _end_col;

  int i1 = locMap(board, r1, c1), i2 = locMap(board, r2, c2);
  pieces[i1] = pieces[i2]; // Move back main piece
  pieces[i2] = nullptr; // Get rid of copy of moved piece so we don't nuke the program (end up segfaulting)

  // Put back all other pieces
  piece::Piece* copy;
  int index;
  for (std::map<std::pair<int,int>, piece::Piece*>::iterator it = _other_replaced_pieces.begin(); it != _other_replaced_pieces.end(); ++it) {
    std::pair<int,int> coords = it -> first;
    index = locMap(board, coords.first, coords.second);

    copy = pieces[index];
    pieces[index] = it -> second;
    delete copy; // free memory to avoid memory leaks
  }

  // Fix move states
  for (std::map<std::pair<int,int>, bool>::iterator it = _piece_setting_changes.begin(); it != _piece_setting_changes.end(); ++it) {
    std::pair<int,int> coords = it -> first;
    updateSetting(board, coords.first, coords.second, it -> second);
  }
}

// Game Class
game::Game::Game(int length, int width) : Game(new Board(length, width)) {}
game::Game::Game(Board* b) {
  _board = b;

  _started = false;
  _over = false;

  _selected_x = -1;
  _selected_y = -1;

  _white_player = nullptr;
  _black_player = nullptr;

  _is_move_complete = false;

  _current_player_color = piece::PieceColor::WHITE;
  _result = game::GameResult::NONE;

  _moves_since_last_capture = 0;
}

game::Game::~Game() {
  delete _board;

  delete _white_player;
  delete _black_player;

  _white_moves.clear();
  _black_moves.clear();
}

bool game::Game::processMove(Move m) {
  if (!m.verify(_board))
    return false;

  bool isCapture = _board -> doMove(new Move(m), this);
  if (!isCapture)
    _moves_since_last_capture++;
  else
    _moves_since_last_capture = 0;

  // Here to satisfy requirements for MCTS/AI players 🤡 (:clown:)
  // SHOULD have no impact on runtime
  generatePossibleMoveVectors();
  return true;
}

void game::Game::selectSquare(int x, int y) {
  assert(_started);
  assert(_board -> isValidPosition(x, y));

  // Reset pawn upgrade type for move
  setPawnUpgradeType(piece::PieceType::NONE);

  if (_board -> getPiece(x, y) -> color() == _current_player_color) {
    if (_selected_x == x && _selected_y == y) {
      _selected_x = -1;
      _selected_y = -1;
    }
    else {
      _selected_x = x;
      _selected_y = y;
    }
  }
  else if (_selected_x != -1 && _selected_y != -1) {
    bool moveSucceeded = tryMove(Move(_selected_x, _selected_y, x, y, _board -> pawnUpgradeType()));
    if (moveSucceeded) {
      _selected_x = -1;
      _selected_y = -1;
    }
  }
}

bool game::Game::tryMove(Move move) {
  bool moveSuccess = processMove(move);

  if (moveSuccess) {
    if (_moves_since_last_capture >= 50) { // 50 non-capture moves = Stalemate
      _over = true;
      _result = game::GameResult::STALEMATE;
      return true;
    }

    int movesSize = _current_player_color.isWhite() ? _white_moves.size(): _black_moves.size();
    if (movesSize == 0) {
      _over = true;

      if (_board -> isKingSafe( _current_player_color))
        _result = game::GameResult::STALEMATE;
      else
        _result = _current_player_color.isWhite() ? game::GameResult::BLACK: game::GameResult::WHITE;
    }

    _is_move_complete = true;
  }

  return moveSuccess;
}

void game::Game::generatePossibleMoveVectors() {
  _white_moves.clear();
  _black_moves.clear();

  _board -> getPossibleMoves(&_white_moves, &_black_moves);
}

std::vector<game::Move> game::Game::possibleMoves() { return possibleMoves(piece::PieceColor::NONE); }

std::vector<game::Move> game::Game::possibleMoves(piece::PieceColor color) {
  std::vector<Move> moves;

  if (!color.isBlack()) // Load white moves
    for (std::vector<Move>::iterator it = _white_moves.begin() ; it != _white_moves.end(); ++it)
      moves.push_back(*it);
  if (!color.isWhite()) // Load black moves
    for (std::vector<Move>::iterator it = _black_moves.begin() ; it != _black_moves.end(); ++it)
      moves.push_back(*it);
  
  return moves;
}

game::Game* game::Game::clone() {
  Game* copy = new Game(_board -> clone());

  copy -> _current_player_color = _current_player_color;

  // selected x,y are already -1
  // players are already nullptr

  copy -> _is_move_complete = false;

  copy -> _started = _started;
  copy -> _over = _over;
  copy -> _moves_since_last_capture = _moves_since_last_capture;
  copy -> _result = _result;

  copy -> generatePossibleMoveVectors();

  return copy;
}