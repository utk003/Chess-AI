#ifndef CHESS_AI_MCTS_NETWORK_TREE_H_
#define CHESS_AI_MCTS_NETWORK_TREE_H_

#include "tree.fwd.h"

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <thread>
#include <atomic>

#include "../chess/piece.h"
#include "../chess/game.fwd.h"
#include "decider.fwd.h"

namespace tree {

class Node {
  public:
    Node() = delete;
    Node(const Node &n) = delete;
    Node &operator=(const Node &n) = delete;

    explicit Node(piece::PieceColor col); // Default priority 0.0 -> for root node only
    ~Node();

    [[nodiscard]] double priority() const;

    void increment_visit_count();
    [[nodiscard]] int visit_count() const;

    void increase_value(double inc);
    [[nodiscard]] double value() const;

    piece::PieceColor color_to_play();
    int countChildren();

    [[nodiscard]] bool expanded() const;
    bool expand(const std::map<game::Move, double> &weights, double sum_weights);

    void addNoise(double frac, const double *noise);

    std::pair<game::Move, tree::Node *> selectOptimalMove(const std::function<double(Node *, Node *)> &ranker);

    static Node *combineNodes(const std::vector<Node *> &nodes, int num_nodes);

  private:
    Node(double prior, piece::PieceColor col);

    double _priority;
    bool _expanded;

    int _visit_count;
    double _value_sum;

    piece::PieceColor _color_to_play;
    std::map<game::Move, Node *> _children;
};

class MCTS {
  public:
    MCTS() = delete;
    MCTS(const MCTS &mcts) = delete;
    MCTS &operator=(const MCTS &mcts) = delete;

    static std::pair<game::Move, Node *> run_mcts_multithreaded(game::Game *game, decider::Decider *move_ranker);
    static std::pair<game::Move, Node *>
    run_mcts_multithreaded(game::Game *game, int num_threads, decider::Decider *move_ranker);
    static std::pair<game::Move, Node *>
    run_mcts_multithreaded(game::Game *game, int num_threads, decider::Decider *move_ranker,
                           const std::vector<Node *> &roots);

    static std::pair<game::Move, Node *> run_mcts(game::Game *game, decider::Decider *move_ranker);

    static int game_move_count;

    static int SIMULATION_SEARCH_DEPTH;
    static int NUM_SIMULATIONS_PER_THREAD;
    static int DEFAULT_NUM_THREADS;

  private:
    static void mcts(game::Game *game, decider::Decider *move_ranker, Node *root, std::atomic_int &count);

    static double expand_node(Node *node, game::Game *game, decider::Decider *move_ranker);
    static std::pair<game::Move, Node *> select_optimal_move(Node *parent);
    static double score_move(Node *parent, Node *child);
    static void propagate_result(std::vector<Node *> &searchPath, double value, piece::PieceColor color);
    static void add_dirichlet_noise(Node *node);
};

}

#endif // CHESS_AI_MCTS_NETWORK_TREE_H_