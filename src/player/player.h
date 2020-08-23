#ifndef CHESS_AI_PLAYER_PLAYER_H_
#define CHESS_AI_PLAYER_PLAYER_H_

#include "player.fwd.h"

#include <vector>

#include "../mcts_network/decider.fwd.h"
#include "../mcts_network/network.fwd.h"
#include "../mcts_network/tree.fwd.h"

namespace player {

class Player {
  public:
    Player() = delete;
    Player(const Player &p) = delete;
    Player &operator=(const Player &p) = delete;

    Player(game::Game *g, piece::PieceColor c, player::PlayerType t);
    virtual ~Player();

    inline game::Game *game() { return _game; }
    inline game::Board *board() { return _board; }
    inline piece::PieceColor color() { return _color; }
    inline player::PlayerType type() { return _type; }

    void playNextMove(); // called by game when its this player's turn to move

  protected:
    game::Game *_game;
    game::Board *_board;
    piece::PieceColor _color{};
    player::PlayerType _type{};

    int _move_count_at_start;

    void playMove(const game::Move &m);
    void playRandomMove();

    virtual void findAndPlayMove() = 0;
    [[nodiscard]] bool moveOverByUndo() const;
    [[nodiscard]] bool moveOver() const;
};

class HumanPlayer : public Player {
  public:
    HumanPlayer() = delete;
    HumanPlayer(const HumanPlayer &p) = delete;
    HumanPlayer &operator=(const HumanPlayer &p) = delete;

    HumanPlayer(game::Game *g, piece::PieceColor c);
    ~HumanPlayer() override;
    void findAndPlayMove() override;

    void clickSquare(int r, int c);
    void setPawnUpgradeType(piece::PieceType type);

  private:
    int _r, _c;
};

class RandomPlayer : public Player {
  public:
    RandomPlayer() = delete;
    RandomPlayer(const RandomPlayer &p) = delete;
    RandomPlayer &operator=(const RandomPlayer &p) = delete;

    RandomPlayer(game::Game *g, piece::PieceColor c);
    ~RandomPlayer() override;
    void findAndPlayMove() override;
};

class MinimaxPlayer : public Player {
  public:
    MinimaxPlayer() = delete;
    MinimaxPlayer(const MinimaxPlayer &p) = delete;
    MinimaxPlayer &operator=(const MinimaxPlayer &p) = delete;

    MinimaxPlayer(game::Game *g, piece::PieceColor c);
    ~MinimaxPlayer() override;
    void findAndPlayMove() override;

  protected:
    game::Board *_simulation_board;

    MinimaxPlayer(game::Game *g, piece::PieceColor c, player::PlayerType t);
    int _search_depth;

    virtual int currentBoardScore();
    std::vector<game::Move> allMoves(piece::PieceColor c);

    bool _is_time_up;
    int _move_counter;

    static void timeKeeper(MinimaxPlayer *p, int moveCount, int time_in_seconds);

  private:
    game::Move bestMove();
    int bestMove(int depth);
    int meanestResponse(int depth);

    static const int DEFAULT_SEARCH_DEPTH = 4; // in half-moves -> each move by black OR white (white move followed by black move == 2 half-moves)
};

class AlphaBetaPlayer : public MinimaxPlayer {
  public:
    AlphaBetaPlayer() = delete;
    AlphaBetaPlayer(const AlphaBetaPlayer &p) = delete;
    AlphaBetaPlayer &operator=(const AlphaBetaPlayer &p) = delete;

    AlphaBetaPlayer(game::Game *g, piece::PieceColor c);;
    ~AlphaBetaPlayer() override;
    void findAndPlayMove() override;

  protected:
    int currentBoardScore() override;

  private:
    game::Move bestMove();
    int alphaBetaSearch(int depth, int alpha, int beta, bool maximizing);

    static void timeKeeper(MinimaxPlayer *p, int moveCount, int time_in_seconds) {
      MinimaxPlayer::timeKeeper(p, moveCount, time_in_seconds);
    }

    static const int DEFAULT_SEARCH_DEPTH = 6; // in half-moves -> each move by black OR white (white move followed by black move == 2 half-moves)
};

class MonteCarloPlayer : public Player {
  public:
    MonteCarloPlayer() = delete;
    MonteCarloPlayer(const MonteCarloPlayer &p) = delete;
    MonteCarloPlayer &operator=(const MonteCarloPlayer &p) = delete;

    MonteCarloPlayer(game::Game *g, piece::PieceColor c);
    ~MonteCarloPlayer() override;
    void findAndPlayMove() override;

  protected:
    MonteCarloPlayer(game::Game *g, piece::PieceColor c, player::PlayerType t);
    decider::Decider *_move_ranker;
};

class NetworkAIPlayer : public MonteCarloPlayer {
  public:
    NetworkAIPlayer() = delete;
    NetworkAIPlayer(const NetworkAIPlayer &p) = delete;
    NetworkAIPlayer &operator=(const NetworkAIPlayer &p) = delete;

    NetworkAIPlayer(game::Game *g, piece::PieceColor c);
    ~NetworkAIPlayer() override;
    void findAndPlayMove() override;

  private:

};

}
#endif // CHESS_AI_PLAYER_PLAYER_H_