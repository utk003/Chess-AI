#include "player.h"

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <functional>
#include <utility>
#include <algorithm>

#include "../chess/piece.h"
#include "../chess/game.h"
#include "../mcts_network/network.h"
#include "../mcts_network/tree.h"
#include "../util/util.h"

// PlayerType class
player::Player *player::PlayerType::getPlayerOfType(PlayerType type, game::Game *game, piece::PieceColor color) {
  switch (type) {
    case HUMAN:
      return new HumanPlayer(game, color);
    case RANDOM:
      return new RandomPlayer(game, color);
    case MINIMAX:
      return new MinimaxPlayer(game, color);
    case AB_PRUNING:
      return new AlphaBetaPlayer(game, color);
    case MCTS:
      return new MonteCarloPlayer(game, color);
    case AI:
      return new NetworkAIPlayer(game, color);

    default:
      assert(false);
  }
}

// Player class
player::Player::Player(game::Game *g, piece::PieceColor c, PlayerType t) {
  _game = g;
  _board = g->board();
  _color = c;
  _type = t;
}
// DO NOT DELETE BOARD OR GAME
// nothing else to delete
player::Player::~Player() = default;

void player::Player::playMove(const game::Move &m) {
  _game->board()->set_pawn_upgrade_type(piece::PieceType::QUEEN); // Default in case move "forgot" it???
  assert(_game->tryMove(m)); // Move must succeed
}

void player::Player::playRandomMove() {
  thread::sleep(1); // Pause AI for 1 second

  std::vector<game::Move> moves = _game->possibleMoves(_color);
  playMove(moves[math::random((int) moves.size())]);
}

// HumanPlayer class
player::HumanPlayer::HumanPlayer(game::Game *g, piece::PieceColor c) : Player(g, c, PlayerType::HUMAN) {
  _r = -1;
  _c = -1;
}
player::HumanPlayer::~HumanPlayer() = default;

void player::HumanPlayer::playNextMove() {
  thread::do_while_waiting_for(
    [&] {
      if (_r != -1) {
        _game->selectSquare(_r, _c);
        _r = -1;
        _c = -1;
      }
    }, [&] { return _game->isMoveOver(); }
  );
}

void player::HumanPlayer::clickSquare(int r, int c) {
  if (_game->getCurrentColor() == _color) {
    _r = r;
    _c = c;
  }
}
void player::HumanPlayer::setPawnUpgradeType(piece::PieceType type) {
  if (_game->getCurrentColor() == _color)
    _game->board()->set_pawn_upgrade_type(type);
}

// RandomPlayer class
player::RandomPlayer::RandomPlayer(game::Game *g, piece::PieceColor c) : Player(g, c, PlayerType::RANDOM) {}
player::RandomPlayer::~RandomPlayer() = default;

void player::RandomPlayer::playNextMove() {
  playRandomMove();
}

// MinimaxPlayer class
player::MinimaxPlayer::MinimaxPlayer(game::Game *g, piece::PieceColor c) : MinimaxPlayer(g, c, PlayerType::MINIMAX) {}
player::MinimaxPlayer::MinimaxPlayer(game::Game *g, piece::PieceColor c, player::PlayerType t) : Player(g, c, t) {
  _search_depth = DEFAULT_SEARCH_DEPTH;
  _simulation_board = nullptr;

  _is_time_up = true;
  _move_counter = 0;
}
player::MinimaxPlayer::~MinimaxPlayer() {
  delete _simulation_board;
}

int player::MinimaxPlayer::currentBoardScore() {
  int score = 0;
  piece::Piece *piece;
  int r, c, temp;
  for (r = 0; r < 8; ++r)
    for (c = 0; c < 8; ++c) {
      piece = _simulation_board->getPiece(r, c);
      temp = piece->type().minimaxValue();
      if (piece->color() == _color)
        score += temp;
      else
        score -= temp;
    }
  return score;
}

std::vector<game::Move> player::MinimaxPlayer::allMoves(piece::PieceColor c) {
  std::vector<game::Move> moves;

  if (c.isWhite())
    _simulation_board->getPossibleMoves(&moves, nullptr);
  else if (c.isBlack())
    _simulation_board->getPossibleMoves(nullptr, &moves);

  return moves;
}

void player::MinimaxPlayer::timeKeeper(MinimaxPlayer *player, int moveCount, int time_in_seconds) {
  thread::sleep(time_in_seconds);
  if (moveCount == player->_move_counter)
    player->_is_time_up = true;
}

void player::MinimaxPlayer::playNextMove() {
  if (_search_depth <= 0) {
    playRandomMove();
    return;
  }

  thread::create(timeKeeper, this, _move_counter, 60); // set up 60 second thinking timer

  playMove(bestMove());
  ++_move_counter;
  _is_time_up = true;
}

game::Move player::MinimaxPlayer::bestMove() {
  int depth = _search_depth;
  _is_time_up = false;

  _simulation_board = _board->clone();
  _simulation_board->set_pawn_upgrade_type(piece::PieceType::QUEEN);

  std::vector<game::Move> moves = allMoves(_color);
  std::set<std::pair<int, game::Move>, std::greater<>> moves_sortedByEndScore;

  for (auto &move : moves) {
    _simulation_board->doMove(new game::Move(move), nullptr);
    int score = currentBoardScore();
    _simulation_board->undoMove(nullptr);

    moves_sortedByEndScore.insert(std::pair<int, game::Move>(score, move));
  }

  game::Move selectedMove = moves[0];
  int maxScore = -500, newScore;
  for (const auto &it : moves_sortedByEndScore) {
    _simulation_board->doMove(new game::Move(it.second), nullptr);

    newScore = meanestResponse(depth - 1);
    if (maxScore < newScore) {
      maxScore = newScore;
      selectedMove = it.second;
    }

    _simulation_board->undoMove(nullptr);

    if (_is_time_up)
      break;
  }

  delete _simulation_board;

  return selectedMove;
}

int player::MinimaxPlayer::bestMove(int depth) {
  if (depth <= 0)
    return currentBoardScore();

  std::vector<game::Move> moves = allMoves(_color);
  if (moves.empty())
    return _simulation_board->isKingSafe(_color) ? 0: -1000; // If safe, stalemate; otherwise, opponent won

  int maxScore = -500, newScore;
  for (auto &move : moves) {
    _simulation_board->doMove(new game::Move(move), nullptr);

    newScore = meanestResponse(depth - 1);
    if (maxScore < newScore)
      maxScore = newScore;

    _simulation_board->undoMove(nullptr);

    if (_is_time_up)
      break;
  }

  return maxScore;
}

int player::MinimaxPlayer::meanestResponse(int depth) {
  if (depth <= 0)
    return currentBoardScore();

  std::vector<game::Move> moves = allMoves(!_color);
  if (moves.empty())
    return _simulation_board->isKingSafe(!_color) ? 0: 1000; // If safe, stalemate; otherwise, we won

  int minScore = 500, newScore;
  for (auto &move : moves) {
    _simulation_board->doMove(new game::Move(move), nullptr);

    newScore = bestMove(depth - 1);
    if (minScore > newScore)
      minScore = newScore;

    _simulation_board->undoMove(nullptr);

    if (_is_time_up)
      break;
  }

  return minScore;
}

// AlphaBetaPlayer Class
player::AlphaBetaPlayer::AlphaBetaPlayer(game::Game *g, piece::PieceColor c) : player::MinimaxPlayer::MinimaxPlayer(g,
                                                                                                                    c,
                                                                                                                    player::PlayerType::AB_PRUNING) {
  _search_depth = DEFAULT_SEARCH_DEPTH;
}
player::AlphaBetaPlayer::~AlphaBetaPlayer() = default;

int player::AlphaBetaPlayer::currentBoardScore() {
  int score = 0;
  piece::Piece *piece;

  int r, c, temp;
  for (r = 0; r < 8; ++r)
    for (c = 0; c < 8; ++c) {
      piece = _simulation_board->getPiece(r, c);
      temp = piece->type().minimaxValue(r, c, 7 * piece->color().isBlack());
      if (piece->color() == _color)
        score += temp;
      else
        score -= temp;
    }

  return score;
}

void player::AlphaBetaPlayer::playNextMove() {
  if (_search_depth <= 0) {
    playRandomMove();
    return;
  }

  std::cout << "AI: " << _type.toString() << " (" << _color << ")" << std::endl;

  thread::create(timeKeeper, this, _move_counter, 120); // set up 120 second thinking timer

  playMove(bestMove());
  ++_move_counter;
  _is_time_up = true;
}

game::Move player::AlphaBetaPlayer::bestMove() {
  int depth = _search_depth;
  _is_time_up = false;

  _simulation_board = _board->clone();
  _simulation_board->set_pawn_upgrade_type(piece::PieceType::QUEEN);

  std::vector<game::Move> moves = allMoves(_color);
  if (moves.size() == 1)
    return moves[0];

  std::set<std::pair<int, game::Move>, std::greater<>> moves_sortedByEndScore;

  for (auto &move : moves) {
    _simulation_board->doMove(new game::Move(move), nullptr);
    int score = currentBoardScore();
    _simulation_board->undoMove(nullptr);

    moves_sortedByEndScore.insert(std::pair<int, game::Move>(score, move));
  }

  game::Move selectedMove = moves[0];
  int value = -10000, alpha = -10000, beta = 10000, newScore, c = 0, totMoves = moves.size();

  for (const auto &it : moves_sortedByEndScore) {
    std::cout << "    Processing Move: " << std::to_string(c + 1) << "/" << std::to_string(totMoves) << std::endl;

    _simulation_board->doMove(new game::Move(it.second), nullptr);
    newScore = alphaBetaSearch(depth - 1, alpha, beta, false);
    if (value < newScore) {
      value = newScore;
      selectedMove = it.second;
    }
    _simulation_board->undoMove(nullptr);

    alpha = std::max(alpha, value);
    if (_is_time_up || alpha >= beta)
      break;

    ++c;
  }

  delete _simulation_board;

  std::cout << "  Processed " << c << "/" << moves.size() << " moves";
  std::cout << " (" << (_is_time_up ? "timed out": "search complete") << ")" << std::endl << std::endl;

  return selectedMove;
}

int player::AlphaBetaPlayer::alphaBetaSearch(int depth, int alpha, int beta, bool maximizing) {
  if (depth <= 0)
    return currentBoardScore();

  piece::PieceColor col = maximizing ? _color: !_color;

  std::vector<game::Move> moves = allMoves(col);

  if (moves.empty())
    // If safe, stalemate; otherwise, checkmate -> if maxing, we lost; else, we won
    return _simulation_board->isKingSafe(col) ? 0: (maximizing ? -100000: 100000);

  if (maximizing) {
    std::set<std::pair<int, game::Move>, std::greater<>> moves_sortedByEndScore;

    for (auto &move : moves) {
      _simulation_board->doMove(new game::Move(move), nullptr);
      int score = currentBoardScore();
      _simulation_board->undoMove(nullptr);

      moves_sortedByEndScore.insert(std::pair<int, game::Move>(score, move));
    }

    int value = -10000;
    for (const auto &it : moves_sortedByEndScore) {
      _simulation_board->doMove(new game::Move(it.second), nullptr);
      value = std::max(value, alphaBetaSearch(depth - 1, alpha, beta, false));
      _simulation_board->undoMove(nullptr);

      alpha = std::max(alpha, value);
      if (_is_time_up || alpha >= beta)
        break;
    }
    return value;
  } else {
    std::set<std::pair<int, game::Move>, std::less<>> moves_sortedByEndScore;

    for (auto &move : moves) {
      _simulation_board->doMove(new game::Move(move), nullptr);
      int score = currentBoardScore();
      _simulation_board->undoMove(nullptr);

      moves_sortedByEndScore.insert(std::pair<int, game::Move>(score, move));
    }

    int value = 10000;
    for (const auto &it : moves_sortedByEndScore) {
      _simulation_board->doMove(new game::Move(it.second), nullptr);
      value = std::min(value, alphaBetaSearch(depth - 1, alpha, beta, true));
      _simulation_board->undoMove(nullptr);

      beta = std::min(beta, value);
      if (_is_time_up || beta <= alpha)
        break;
    }
    return value;
  }
}

// MonteCarloPlayer Class
player::MonteCarloPlayer::MonteCarloPlayer(game::Game *g, piece::PieceColor c) : Player(g, c,
                                                                                        player::PlayerType::MCTS) {
  _move_ranker = new decider::Randomizer();
}
player::MonteCarloPlayer::MonteCarloPlayer(game::Game *g, piece::PieceColor c, player::PlayerType t) : Player(g, c,
                                                                                                              t) {}
player::MonteCarloPlayer::~MonteCarloPlayer() {
  delete _move_ranker;
}

void player::MonteCarloPlayer::playNextMove() {
  std::pair<game::Move, tree::Node *> move_node_pair = tree::MCTS::run_mcts(_game, _move_ranker);
  playMove(move_node_pair.first);

  delete move_node_pair.second; // free memory to prevent memory leaks
}

// NetworkAIPlayer Class
player::NetworkAIPlayer::NetworkAIPlayer(game::Game *g, piece::PieceColor c) : MonteCarloPlayer(g, c,
                                                                                                player::PlayerType::AI) {
  _move_ranker = network::NetworkStorage::current_network();
}
player::NetworkAIPlayer::~NetworkAIPlayer() {
  _move_ranker = nullptr;
}

void player::NetworkAIPlayer::playNextMove() {
  std::pair<game::Move, tree::Node *> move_node_pair = tree::MCTS::run_mcts_multithreaded(_game, _move_ranker);
  playMove(move_node_pair.first);

  network::NetworkStorage::saveBoard(_game->board());

  delete move_node_pair.second; // free memory to prevent memory leaks
}