#include "game.h"

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>

#include "piece.h"
#include "../util/thread_util.h"

// Board Class
game::Board::Board(int l, int w) {
  _length = l;
  _width = w;

  _pawn_upgrade_type = piece::PieceType::NONE;

  int i, total = _length * _width;
  for (i = 0; i < total; ++i)
    _pieces.push_back(nullptr);
}

game::Board::~Board() {
  piece::Piece *piece;
  for (int i = 0; i < _length * _width; ++i) {
    piece = _pieces[i];
    _pieces[i] = nullptr;
    delete piece;
  }
  _pieces.clear();

  game::Move *move;
  while (!_move_stack.empty()) {
    move = _move_stack.top();
    _move_stack.pop();
    delete move;
  }
}

int game::Board::getPositionThreats(int r, int c, piece::PieceColor kingColor) const {
  if (!isValidPosition(r, c)) {
    debug_assert();
    return 0;
  }

  piece::PieceColor enemyColor = !kingColor;
  int x, y, dangerCounter = 0;

  piece::Piece *checkPiece;

  // Check axis attacks (queen/rook)
  std::vector<std::vector<int>> axisCheck = {{1,  0},
                                             {-1, 0},
                                             {0,  1},
                                             {0,  -1}};
  for (std::vector<int> axis: axisCheck) {
    x = r + axis[0];
    y = c + axis[1];
    while (isValidPosition(x, y)) {
      checkPiece = getPiece(x, y);
      if (checkPiece->color() == enemyColor) {
        if (checkPiece->type().isQueen() || checkPiece->type().isRook())
          ++dangerCounter;
        break;
      }

      if (checkPiece->color() == kingColor) // Long range attacks can be blocked
        break;

      x += axis[0];
      y += axis[1];
    }
  }

  // Check diagonal attacks (queen/bishop)
  std::vector<std::vector<int>> diagCheck = {{1,  1},
                                             {1,  -1},
                                             {-1, 1},
                                             {-1, -1}};
  for (std::vector<int> diag: diagCheck) {
    x = r + diag[0];
    y = c + diag[1];
    while (isValidPosition(x, y)) {
      checkPiece = getPiece(x, y);
      if (checkPiece->color() == enemyColor) {
        if (checkPiece->type().isQueen() || checkPiece->type().isBishop())
          ++dangerCounter;
        break;
      }

      if (checkPiece->color() == kingColor) // Long range attacks can be blocked
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

      checkPiece = getPiece(x, y);
      if (checkPiece->color() == enemyColor && checkPiece->type().isKing())
        dangerCounter++;
    }

  std::vector<int> pm1 = {1, -1};

  // Check pawn attacks
  x = r + (enemyColor.isWhite() ? -1: 1); // if enemy is white, pawn attacks from below; otherwise from above
  for (int dc: pm1) {
    y = c + dc;

    if (!isValidPosition(x, y))
      continue;

    checkPiece = getPiece(x, y);
    if (checkPiece->color() == enemyColor && checkPiece->type().isPawn())
      dangerCounter++;
  }

  // Check knight attacks
  std::vector<std::vector<int>> knightMoves = {{1, 2},
                                               {2, 1}};
  for (std::vector<int> move: knightMoves)
    for (int mr: pm1)
      for (int mc: pm1) {
        x = r + mr * move[0];
        y = c + mc * move[1];

        if (!isValidPosition(x, y))
          continue;

        checkPiece = getPiece(x, y);
        if (checkPiece->color() == enemyColor && checkPiece->type().isKnight())
          dangerCounter++;
      }

  return dangerCounter; // Return total danger count
}

std::pair<int, int> game::Board::getKingPosition(piece::PieceColor color) const {
  piece::Piece *checkPiece;
  int xKing = -1, yKing = -1, r, c;

  for (r = 0; r < _length; ++r) {
    for (c = 0; c < _width; ++c) {
      checkPiece = getPiece(r, c);
      if (checkPiece->type().isKing() && checkPiece->color() == color) {
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
  // get piece indices
  int from = locMap(r, c), to = locMap(toR, toC);
  if (from < 0 || to < 0) {
    debug_assert();
    return false;
  }

  // get pieces
  piece::PieceColor pieceColor = _pieces[from]->color();
  if (!pieceColor.isColored()) {
    debug_assert();
    return false;
  }

  // save replaced piece (b/c to is overwritten by from)
  piece::Piece *copy = _pieces[to];
  auto *newPiece = new piece::Piece();

  // simulate move
  _pieces[to] = _pieces[from];
  _pieces[from] = newPiece;

  // score threats
  bool isSafe = isKingSafe(pieceColor);

  // undo move
  _pieces[from] = _pieces[to];
  _pieces[to] = copy;

  // free memory
  delete newPiece;

  // return result
  return isSafe; // move allowed iff king is safe post-move
}

void game::Board::getMovesFromSquare(int r, int c, std::vector<game::Move> *moves) {
  if (moves == nullptr)
    return;

  if (isValidPosition(r, c)) {
    int r2, c2;
    for (r2 = 0; r2 < 8; ++r2)
      for (c2 = 0; c2 < 8; ++c2) {
        if (r == r2 && c == c2)
          continue;

        std::vector<Move> poss_moves = Move::getMoves(r, c, r2, c2, this);
        for (const Move &move: poss_moves)
          if (move.verify(this))
            moves->push_back(move);
      }
  } else debug_assert();
}

void game::Board::getPossibleMoves(std::vector<game::Move> *white, std::vector<game::Move> *black) {
  int r1, c1;
  for (r1 = 0; r1 < 8; ++r1)
    for (c1 = 0; c1 < 8; ++c1)
      switch (getPiece(r1, c1)->color()) {
        case piece::PieceColor::WHITE:
          getMovesFromSquare(r1, c1, white);
          break;

        case piece::PieceColor::BLACK:
          getMovesFromSquare(r1, c1, black);
          break;

        default:
          continue;
      }
}

bool game::Board::doMove(Move *move, Game *game) {
  if (move == nullptr) {
    debug_assert();
    return false;
  }
  _move_stack.push(move);
  _move_count++;

  bool isCaptureMove = move->doMove(this);

  if (game != nullptr)
    game->_current_player_color = _move_count % 2 == 0 ? piece::PieceColor::WHITE: piece::PieceColor::BLACK;

  return isCaptureMove;
}

void game::Board::undoMove(Game *game) {
  if (_move_stack.empty()) {
    debug_assert();
    return;
  }
  _move_count++;

  Move *move = _move_stack.top();
  move->undoMove(this);
  _move_stack.pop();
  delete move;

  if (game != nullptr) {
    game->_current_player_color = _move_count % 2 == 0 ? piece::PieceColor::WHITE: piece::PieceColor::BLACK;
    game->resetSelection();
    game->updateGameState();
  }
}

game::Move *game::Board::getLastMove() const {
  return _move_stack.empty() ? nullptr: new Move(*_move_stack.top());
}

game::Board *game::Board::clone() const {
  auto *newBoard = new Board(_length, _width);

  int i, total_count = _length * _width;
  for (i = 0; i < total_count; ++i)
    newBoard->_pieces[i] = _pieces[i]->clone();

  newBoard->_pawn_upgrade_type = piece::PieceType::NONE;
  newBoard->_move_count.store(_move_count.operator int());

  return newBoard;
}

double game::Board::score(const std::function<double(piece::Piece *)> &piece_scorer) const {
  double score = 0;
  for (const auto &piece: _pieces)
    score += piece_scorer(piece);
  return score;
}

namespace game {

std::istream &operator>>(std::istream &input, Board *&b) {
  // memory management -> clear old board completely
  while (!b->_move_stack.empty()) {
    delete b->_move_stack.top();
    b->_move_stack.pop();
  }
  b->_pawn_upgrade_type = piece::PieceType::NONE;
  for (auto &p: b->_pieces)
    delete p;
  b->_pieces.clear();

  // Load new board in
  input >> b->_length >> b->_width;
  int max_index = b->_length * b->_width;
  b->_pieces.resize(max_index);

  int ind;
  std::string spacer;
  for (int i = 0; i < max_index; ++i) {
    input >> ind >> spacer;
    if (ind == i)
      input >> b->_pieces[i];
    else debug_assert(); // -> Malformed input file!!
    getline(input, spacer); // skip to end of line
  }

  return input;
}

std::ostream &operator<<(std::ostream &output, Board *&b) {
  output << b->_length << " " << b->_width << std::endl;

  for (int i = 0; i < b->_length * b->_width; ++i)
    output << i << " - " << b->_pieces[i] << std::endl;

  return output;
}

}

std::ofstream game::Board::saveToFile(const std::string &fileName) {
  std::ofstream out_stream("game_state/" + fileName + ".txt");
  if (out_stream.is_open())
    out_stream << this;
  else debug_assert();

  return out_stream;
}

std::ifstream game::Board::loadFromFile(const std::string &fileName) {
  std::ifstream in_stream(fileName);
  if (in_stream.is_open()) {
    Board *b = this;
    in_stream >> b;
  } else debug_assert();

  return in_stream;
}

// Move Class
std::vector<game::Move> game::Move::getMoves(int r1, int c1, int r2, int c2, game::Board *b) {
  std::vector<game::Move> moves;
  piece::Piece *piece = b->getPiece(r1, c1);

  if (piece != nullptr) {
    if (piece->type().isPawn() && (r2 == 0 || r2 == 7)) {
      moves.emplace_back(r1, c1, r2, c2, piece::PieceType::QUEEN);
      moves.emplace_back(r1, c1, r2, c2, piece::PieceType::ROOK);
      moves.emplace_back(r1, c1, r2, c2, piece::PieceType::KNIGHT);
      moves.emplace_back(r1, c1, r2, c2, piece::PieceType::BISHOP);
    } else
      moves.emplace_back(r1, c1, r2, c2, piece::PieceType::NONE);
  } else debug_assert();

  return moves;
}

game::Move::Move(int r1, int c1, int r2, int c2, piece::PieceType promotionType) {
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
  piece::Piece *piece;
  for (auto &_other_replaced_piece : _other_replaced_pieces) {
    piece = _other_replaced_pieces[_other_replaced_piece.first];
    _other_replaced_pieces[_other_replaced_piece.first] = nullptr;
    delete piece;
  }
  _other_replaced_pieces.clear();

  _piece_setting_changes.clear();
}

game::Move &game::Move::operator=(const Move &m) {
  _start_row = m._start_row;
  _start_col = m._start_col;

  _end_row = m._end_row;
  _end_col = m._end_col;

  _pawn_promotion_type = m._pawn_promotion_type;

  piece::Piece *piece;
  for (auto &_other_replaced_piece : _other_replaced_pieces) {
    piece = _other_replaced_pieces[_other_replaced_piece.first];
    _other_replaced_pieces[_other_replaced_piece.first] = nullptr;
    delete piece;
  }
  _other_replaced_pieces.clear();

  _piece_setting_changes.clear();

  return *this;
}

bool game::Move::verify(Board *board) const {
  // Implicit calls to:
  piece::Piece *p1 = board->getPiece(_start_row, _start_col);
  if (p1 == nullptr) {
    debug_assert();
    return false;
  }
  piece::Piece *p2 = board->getPiece(_end_row, _end_col);
  if (p2 == nullptr) {
    debug_assert();
    return false;
  }

  if (p1->type().isEmpty()) // fail if not moving a piece
    return false;

  if (p1->color() == p2->color()) // fail if moving to same color piece
    return false;

  if (!p1->verifyMove(*this, board)) // fail if invalid move pattern
    return false;

  // succeed iff moving piece does NOT put king in check/checkmate
  return board->canPieceMove(_start_row, _start_col, _end_row, _end_col);
}

bool game::Move::isAttack(Board *board) const {
  if (!board->getPiece(_end_row, _end_col)->type().isEmpty())
    return true; // target in attacked square

  if (!board->getPiece(_start_row, _start_col)->type().isPawn())
    return false; // only pawns have "janky" attack (don't move onto captured piece cell)

  return _start_col != _end_col; // is pawn attack iff pawn moved sideways
}

void game::Move::addReplacedPiece(int r, int c, piece::Piece *p) {
  if (p != nullptr)
    _other_replaced_pieces[std::pair<int, int>(r, c)] = p;
}

void game::Move::addSettingChange(int r, int c, bool oldSetting) {
  _piece_setting_changes[std::pair<int, int>(r, c)] = oldSetting;
}

bool game::Move::doMove(Board *board) {
  std::vector<piece::Piece *> &pieces = getBoard(board);

  piece::Piece *p;
  int r, c;
  for (r = 0; r < 8; ++r)
    for (c = 0; c < 8; ++c) {
      p = pieces[locMap(board, r, c)];
      if (p->type().isPawn())
        if (((piece::Pawn *) p)->moved2x()) {
          addSettingChange(r, c, true);
          update_flag((piece::Pawn *) p, false);
        }
    }

  int r1 = _start_row, c1 = _start_col, r2 = _end_row, c2 = _end_col;

  int from = locMap(board, r1, c1), to = locMap(board, r2, c2);

  piece::Piece *removedPiece = pieces[to];
  pieces[to] = pieces[from];
  pieces[from] = new piece::Piece();

  addReplacedPiece(r2, c2, removedPiece);
  switch (removedPiece->type()) {
    case piece::PieceType::ROOK:
      addSettingChange(r2, c2, ((piece::Rook *) removedPiece)->moved());
      break;

    case piece::PieceType::KING:
      addSettingChange(r2, c2, ((piece::King *) removedPiece)->moved());
      break;

    case piece::PieceType::PAWN:
      addSettingChange(r2, c2, ((piece::Pawn *) removedPiece)->moved2x());
      break;

    default:
      break;
  }

  piece::Piece *piece = board->getPiece(r2, c2);
  piece::PieceType type = piece->type();

  // Piece Settings
  switch (type) {
    case piece::PieceType::ROOK:
      if (!((piece::Rook *) piece)->moved()) {
        addSettingChange(r1, c1, false);
        update_flag((piece::Rook *) piece, true);
      }
      break;

    case piece::PieceType::KING:
      if (!((piece::King *) piece)->moved()) {
        addSettingChange(r1, c1, false);
        update_flag((piece::King *) piece, true);
      }
      break;

    case piece::PieceType::PAWN:
      if (abs(r1 - r2) == 2) {
        addSettingChange(r1, c1, false);
        update_flag((piece::Pawn *) piece, true);
      }
      break;

    default:
      break;
  }

  // Other changed pieces
  switch (type) {
    case piece::PieceType::KING:
      if (abs(c1 - c2) == 2) {
        int rookCol = (c2 > c1) * 7; // if c2 > c1, then king moved right, so rookCol = 7; else, rookCol = 0
        int newC = (c1 + c2) / 2;

        int oldPos = locMap(board, r1, rookCol);
        int newPos = locMap(board, r2, newC);

        // clone rook b/c it gets deleted in undoMove() while restoring pieces -> segfault
        addReplacedPiece(r1, rookCol, pieces[oldPos]->clone());
        addReplacedPiece(r2, newC, pieces[newPos]);

        pieces[newPos] = pieces[oldPos];
        pieces[oldPos] = new piece::Piece();
      }
      break;

    case piece::PieceType::PAWN:
      if (r2 == 7 || r2 == 0) {
        piece::Piece *newPiece = nullptr;
        int timer = 0;
        piece::PieceType upgrade_type = _pawn_promotion_type;
        while (newPiece == nullptr && timer++ < 600) { // Time out after 600 seconds = 10 minutes
          switch (upgrade_type) {
            case piece::PieceType::QUEEN:
            case piece::PieceType::ROOK:
            case piece::PieceType::KNIGHT:
            case piece::PieceType::BISHOP:
              newPiece = upgrade_type.getPieceOfType(piece->color());
              break;

            default:
              thread::sleep(1); // Sleep for 1 second
              upgrade_type = board->pawn_upgrade_type();
              break;
          }
        }

        // timer hit 600 <--> Program time out
        if (newPiece == nullptr) {
          std::cout << "Program timed out" << std::endl;
          fatal_assert();
        }

        addReplacedPiece(r1, c1, piece);
        pieces[locMap(board, r2, c2)] = newPiece;
      }
      if (abs(c1 - c2) == 1 && removedPiece->type().isEmpty()) {
        int captured = locMap(board, r1, c2);
        removedPiece = pieces[captured];
        pieces[captured] = new piece::Piece();

        addReplacedPiece(r1, c2, removedPiece);
      }
      break;

    default:
      break;
  }

  return !removedPiece->type().isEmpty();
}

void game::Move::updateSetting(Board *board, int r, int c, bool setting) {
  piece::Piece *piece = getBoard(board)[locMap(board, r, c)];

  switch (piece->type()) {
    case piece::PieceType::ROOK:
      update_flag((piece::Rook *) piece, setting);
      break;

    case piece::PieceType::KING:
      update_flag((piece::King *) piece, setting);
      break;

    case piece::PieceType::PAWN:
      update_flag((piece::Pawn *) piece, setting);
      break;

    default:
      break;
  }
}

void game::Move::undoMove(Board *board) const {
  std::vector<piece::Piece *> &pieces = getBoard(board);
  int r1 = _start_row, c1 = _start_col, r2 = _end_row, c2 = _end_col;

  int i1 = locMap(board, r1, c1), i2 = locMap(board, r2, c2);
  piece::Piece *oldPos = pieces[i1];
  pieces[i1] = pieces[i2]; // Move back main piece
  pieces[i2] = nullptr; // Get rid of copy of moved piece so we don't nuke the program (end up segfaulting)
  delete oldPos; // Delete old replacement piece!!!

  // Put back all other pieces
  piece::Piece *copy;
  int index;
  for (auto &_other_replaced_piece : _other_replaced_pieces) {
    std::pair<int, int> coords = _other_replaced_piece.first;
    index = locMap(board, coords.first, coords.second);

    copy = pieces[index];
    pieces[index] = _other_replaced_piece.second->clone();
    delete copy; // free memory to avoid memory leaks
  }

  // Fix move states
  for (auto &_piece_setting_change : _piece_setting_changes) {
    std::pair<int, int> coords = _piece_setting_change.first;
    updateSetting(board, coords.first, coords.second, _piece_setting_change.second);
  }
}

std::string game::Move::toString() const {
  std::stringstream ss;
  ss << "(" << _start_row << ", " << _start_col << ") to (" << _end_row << ", " << _end_col << ") -> "
     << _pawn_promotion_type;
  return ss.str();
}

// Game Class
game::Game::Game(int length, int width) : Game(new Board(length, width)) {}

game::Game::Game(Board *b) {
  _board = b;

  _started = false;
  _over = false;

  _selected_x = -1;
  _selected_y = -1;

  _white_player = nullptr;
  _black_player = nullptr;

  _is_move_complete = false;
  _is_ready_to_delete = false;

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

void game::Game::setPlayer(piece::PieceColor color, player::PlayerType type) {
  if (!color.isColored()) {
    debug_assert();
    return;
  }

  player::Player *player = type.getPlayerOfType(this, color);

  if (color.isWhite())
    _white_player = player;
  else // if (color.isBlack())
    _black_player = player;
}

void game::Game::startGame() {
  if (_white_player == nullptr) {
    debug_assert();
    return;
  }
  if (_black_player == nullptr) {
    debug_assert();
    return;
  }

  _started = true;

  generatePossibleMoveVectors();

  thread::create([&] {
    while (!_over) {
      _is_move_complete = false;
      if (_current_player_color.isWhite())
        _white_player->playNextMove();
      else
        _black_player->playNextMove();
    }
    _is_ready_to_delete = true;
  });
}

void game::Game::endGame() {
  _over = true;
  _is_move_complete = true;
}

void game::Game::waitForDelete() const {
  thread::wait_for(_is_ready_to_delete);
}

game::Game *game::Game::clone() const {
  Game *copy = new Game(_board->clone());

  copy->_current_player_color = _current_player_color;

  // selected x,y are already -1
  // players are already nullptr

  copy->_is_move_complete = false;

  copy->_started = _started;
  copy->_over = _over;
  copy->_moves_since_last_capture = _moves_since_last_capture;
  copy->_result = _result;

  copy->generatePossibleMoveVectors();

  return copy;
}

void game::Game::selectSquare(int x, int y) {
  if (!_started) {
    debug_assert();
    return;
  }
  if (!_board->isValidPosition(x, y)) {
    debug_assert();
    return;
  }

  // Reset pawn upgrade type for move
  _board->set_pawn_upgrade_type(piece::PieceType::NONE);

  if (_board->getPiece(x, y)->color() == _current_player_color) {
    if (_selected_x == x && _selected_y == y)
      resetSelection();
    else {
      _selected_x = x;
      _selected_y = y;
    }
  } else if (_selected_x != -1 && _selected_y != -1) {
    bool moveSucceeded = tryMove(Move(_selected_x, _selected_y, x, y, _board->pawn_upgrade_type()));
    if (moveSucceeded)
      resetSelection();
  }
}

bool game::Game::tryMove(const Move &move) {
  // check if move is valid
  if (!move.verify(_board))
    return false;

  // do move
  bool isCapture = _board->doMove(new Move(move), this);

  // check for 50 move no-capture stalemate
  if (!isCapture)
    _moves_since_last_capture++;
  else
    _moves_since_last_capture = 0;

  if (_moves_since_last_capture >= 50) { // 50 non-capture moves = Stalemate
    _over = true;
    _result = game::GameResult::STALEMATE;
  } else
    updateGameState();

  // move complete
  _is_move_complete = true;
  return true;
}

void game::Game::updateGameState() {
  // check for checkmate/stalemate
  generatePossibleMoveVectors();
  int movesSize = _current_player_color.isWhite() ? _white_moves.size(): _black_moves.size();
  if (movesSize == 0) {
    _over = true;

    if (_board->isKingSafe(_current_player_color))
      _result = game::GameResult::STALEMATE;
    else
      _result = _current_player_color.isWhite() ? game::GameResult::BLACK: game::GameResult::WHITE;
  } else {
    _over = false;
    _result = game::GameResult::NONE;
  }
}

void game::Game::generatePossibleMoveVectors() {
  _white_moves.clear();
  _black_moves.clear();

  _board->getPossibleMoves(&_white_moves, &_black_moves);
}

std::vector<game::Move> game::Game::possibleMoves() const { return possibleMoves(piece::PieceColor::NONE); }
std::vector<game::Move> game::Game::possibleMoves(piece::PieceColor color) const {
  std::vector<Move> moves;

  if (!color.isBlack()) // Load white moves
    for (auto &_white_move : _white_moves)
      moves.push_back(_white_move);

  if (!color.isWhite()) // Load black moves
    for (auto &_black_move : _black_moves)
      moves.push_back(_black_move);

  return moves;
}