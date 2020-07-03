#ifndef CHESSAI_PLAYER_PLAYER_H_
#define CHESSAI_PLAYER_PLAYER_H_

#include "player.fwd.h"

#include <string>
#include <vector>

#include "chess/piece.h"
#include "chess/game.fwd.h"
#include "mcts_network/network.fwd.h"

namespace player {

class Player {
  public:
    Player(game::Game* g, piece::PieceColor c, player::PlayerType t);
    virtual ~Player();

    inline game::Game* game() { return _game; }
    inline game::Board* board() {return _board; }
    inline piece::PieceColor color() { return _color; }
    inline player::PlayerType type() { return _type; }

    virtual void playNextMove() = 0; // called by game when its this player's turn to move

  protected:
    game::Game* _game;
    game::Board* _board;
    piece::PieceColor _color;
    player::PlayerType _type;

    void playMove(game::Move m);
    void playRandomMove();
};

class HumanPlayer : public Player {
  public:
    HumanPlayer(game::Game* g, piece::PieceColor c);
    virtual ~HumanPlayer();
    virtual void playNextMove();

    void clickSquare(int r, int c);
    void setPawnUpgradeType(piece::PieceType type);
  
  private:
    int _r, _c;
};

class RandomPlayer : public Player {
  public:
    RandomPlayer(game::Game* g, piece::PieceColor c);
    virtual ~RandomPlayer();
    virtual void playNextMove();
};

class MinimaxPlayer : public Player {
  public:
    MinimaxPlayer(game::Game* g, piece::PieceColor c);
    virtual ~MinimaxPlayer();
    virtual void playNextMove();
  
  protected:
    game::Board* _simulation_board;

    MinimaxPlayer(game::Game* g, piece::PieceColor c, player::PlayerType t);
    int _search_depth;

    virtual int currentBoardScore();
    std::vector<game::Move> allMoves(piece::PieceColor c);

    bool _is_time_up;
    int _move_counter;

    static void timeKeeper(MinimaxPlayer* p, int moveCount, int time_in_seconds);

  private:
    game::Move bestMove();
    int bestMove(int depth);
    int meanestResponse(int depth);

    static const int DEFAULT_SEARCH_DEPTH = 4; // in half-moves -> each move by black OR white (white move followed by black move == 2 half-moves)
};

class AlphaBetaPlayer : public MinimaxPlayer {
  public:
    AlphaBetaPlayer(game::Game* g, piece::PieceColor c);
    virtual ~AlphaBetaPlayer();
    virtual void playNextMove();
  
  protected:
    virtual int currentBoardScore();
    
  private:
    game::Move bestMove();
    int alphaBetaSearch(int depth, int alpha, int beta, bool maximizing);

    static void timeKeeper(MinimaxPlayer* p, int moveCount, int time_in_seconds) { MinimaxPlayer::timeKeeper(p, moveCount, time_in_seconds); }

    static const int DEFAULT_SEARCH_DEPTH = 6; // in half-moves -> each move by black OR white (white move followed by black move == 2 half-moves)
};

class MonteCarloPlayer : public Player {
  public:
    MonteCarloPlayer(game::Game* g, piece::PieceColor c);
    virtual ~MonteCarloPlayer();
    virtual void playNextMove();
  
  private:
    network::Decider* _move_ranker;
};

}
#endif // CHESSAI_PLAYER_PLAYER_H_