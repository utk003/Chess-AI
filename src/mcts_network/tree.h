#ifndef CHESSAI_MCTS_NETWORK_TREE_H_
#define CHESSAI_MCTS_NETWORK_TREE_H_

#include <string>
#include <vector>
#include <utility>
#include <map>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "network.fwd.h"

namespace tree {

class Node {
  friend class MCTS;

  public:
    Node(const Node &n) = delete;
    Node &operator=(const Node &n) = delete;

    Node(); // Default priority 0.0f -> for root node only
    Node(float prior);

    ~Node();

    inline float priority() { return _priority; }
    
    inline int visit_count() { return _visit_count; }
    inline float value() { return _visit_count > 0 ? _value_sum / _visit_count: 0.0f; }

    inline piece::PieceColor color_to_play() { return _color_to_play; }

    inline bool expanded() { return !_children.empty(); }
    inline std::map<game::Move, Node*> &children() { return _children; }

  private:
    float _priority;

    int _visit_count;
    float _value_sum;

    piece::PieceColor _color_to_play; 
    std::map<game::Move, Node*> _children;
};

class MCTS {
  public:
    MCTS() = delete;
    MCTS(const MCTS &mcts) = delete;
    MCTS &operator=(const MCTS &mcts) = delete;

    static std::pair<game::Move, Node*> run_mcts(game::Game* game, network::Decider* move_ranker);

  private:
    static const int NUM_SIMULATIONS = 100;

    static float expand_node(Node* node, game::Game* game, network::Decider* move_ranker);
    static std::pair<game::Move, Node*> select_optimal_child(Node* parent);
    static float score_move(Node* parent, Node* child);
    static void propagate_result(std::vector<Node*> searchPath, float value, piece::PieceColor color);
    static void add_dirichlet_noise(Node* node);
    static game::Move select_optimal_move(Node* root);
};

}

#endif // CHESSAI_MCTS_NETWORK_TREE_H_