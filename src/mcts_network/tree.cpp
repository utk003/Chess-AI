#include "tree.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <vector>
#include <utility>
#include <map>
#include <cmath>
#include <functional>
#include <atomic>

#include "../chess/game.h"
#include "decider.h"
#include "../util/thread_util.h"

// Node class
tree::Node::Node(piece::PieceColor col) : Node(0.0, col) {}
tree::Node::Node(double prior, piece::PieceColor col) {
  _priority = prior;

  _expanded = false;

  _visit_count = 0;
  _value_sum = 0.0;

  _color_to_play = col;
}

tree::Node::~Node() {
  Node *node;
  for (auto &it : _children) {
    node = _children[it.first];
    _children[it.first] = nullptr;
    delete node;
  }
  _children.clear();
}

double tree::Node::priority() const {
  return _priority;
}

void tree::Node::increment_visit_count() {
  _visit_count++;
}
int tree::Node::visit_count() const {
  return _visit_count;
}

void tree::Node::increase_value(double inc) {
  _value_sum += inc;
}
double tree::Node::value() const {
  return _visit_count > 0 ? _value_sum / _visit_count: 0.0;
}

piece::PieceColor tree::Node::color_to_play() {
  return _color_to_play;
}
int tree::Node::countChildren() {
  return _children.size();
}

bool tree::Node::expanded() const {
  return _expanded;
}
bool tree::Node::expand(const std::map<game::Move, double> &weights, double sum_weights) {
  if (!_expanded) {
    for (auto &weight : weights)
      _children[weight.first] = new Node(weight.second / sum_weights, !_color_to_play);

    _expanded = true;
  }
  return !_children.empty();
}

void tree::Node::addNoise(double frac, const double *noise) {
  int i = 0;
  for (auto &it : _children)
    it.second->_priority = it.second->_priority * (1.0 - frac) + noise[i++] * frac;
}

std::pair<game::Move, tree::Node *> tree::Node::selectOptimalMove(const std::function<double(Node *, Node *)> &ranker) {
  std::pair<game::Move, Node *> optimal{game::Move(-1, -1, -1, -1, piece::PieceType::NONE), nullptr};
  double max_score = -1000000.0, score;

  for (auto &it : _children) {
    score = ranker(this, it.second);
    if (score > max_score) {
      max_score = score;
      optimal = {it.first, it.second};
    }
  }

  if (optimal.second == nullptr)
    debug_assert();
  return optimal;
}

tree::Node *tree::Node::combineNodes(const std::vector<Node *> &nodes, int num_nodes) {
  piece::PieceColor root_color = nodes[0]->color_to_play();
  auto *new_node = new Node(root_color);

  Node *child;
  for (int i = 0; i < num_nodes; ++i) {
    for (auto &it: nodes[i]->_children) {
      if (new_node->_children[it.first] == nullptr)
        new_node->_children[it.first] = new Node(!root_color);

      child = new_node->_children[it.first];
      child->_priority += it.second->_priority / num_nodes;
      child->_visit_count += it.second->_visit_count;
      child->_value_sum += it.second->_value_sum;
    }
    new_node->_value_sum += nodes[i]->_value_sum;
    new_node->_visit_count += nodes[i]->_visit_count;
  }

  return new_node;
}

// Monte Carlo Tree Search class
std::pair<game::Move, tree::Node *>
tree::MCTS::run_mcts_multithreaded(game::Game *game, decider::Decider *move_ranker) {
  return run_mcts_multithreaded(game, DEFAULT_NUM_THREADS, move_ranker);
}
std::pair<game::Move, tree::Node *>
tree::MCTS::run_mcts_multithreaded(game::Game *game, int num_threads, decider::Decider *move_ranker) {
  std::vector<Node *> roots(num_threads);
  for (int i = 0; i < num_threads; ++i)
    roots[i] = new Node(game->getCurrentColor());

  std::pair<game::Move, tree::Node *> return_val = run_mcts_multithreaded(game, num_threads, move_ranker, roots);

  for (auto &it: roots)
    delete it;
  roots.clear();

  return return_val;
}
std::pair<game::Move, tree::Node *>
tree::MCTS::run_mcts_multithreaded(game::Game *game, int num_threads, decider::Decider *move_ranker,
                                   const std::vector<Node *> &roots) {
  if (roots.size() < num_threads) {
    debug_assert();
    num_threads = roots.size();
  }

  std::atomic_int counter{NUM_SIMULATIONS};
  std::vector<game::Game *> clones(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    clones[i] = game->clone();

    expand_node(roots[i], clones[i], move_ranker);
    add_dirichlet_noise(roots[i]);

    thread::create(mcts, clones[i], move_ranker, roots[i], std::ref(counter));
  }

  thread::wait_for([&] { return counter <= -num_threads; });

  for (auto &clone: clones)
    delete clone;
  clones.clear();

  auto *master_node = Node::combineNodes(roots, num_threads);
  return {select_optimal_move(master_node).first, master_node};
}

std::pair<game::Move, tree::Node *> tree::MCTS::run_mcts(game::Game *game, decider::Decider *move_ranker) {
  return run_mcts(game, move_ranker, new Node(game->getCurrentColor()));
}
std::pair<game::Move, tree::Node *> tree::MCTS::run_mcts(game::Game *game, decider::Decider *move_ranker, Node *root) {
  game::Game *clone = game->clone();
  expand_node(root, clone, move_ranker);
  add_dirichlet_noise(root);
  delete clone;

  std::atomic_int counter{NUM_SIMULATIONS};
  mcts(game, move_ranker, root, std::ref(counter));

  return {select_optimal_move(root).first, root};
}

void tree::MCTS::mcts(game::Game *game, decider::Decider *move_ranker, Node *root, std::atomic_int &count) {
  Node *node;
  game::Game *clone;
  std::vector<Node *> searchPath;

  double value;

  int i;
  while (count-- > 0) {
    node = root;
    clone = game->clone();
    searchPath = {root};

    i = 0;
    while (!clone->isOver() && i < 1) { // TODO decide 1 or 20 or unlimited depth search
      if (!node->expanded())
        expand_node(node, clone, move_ranker);

      std::pair<game::Move, Node *> optimal = select_optimal_move(node);
      node = optimal.second;
      bool success = clone->tryMove(optimal.first);
      if (!success) {
        debug_assert();

        delete clone;
        return;
      }

      searchPath.push_back(node);
      ++i;
    }

    // If game is over
    if (clone->isOver())
      // If black has won, that means current color is white
      // Thus, value is ((1 * -1) + 1) / 2 = 0, which is fine b/c current node (white) lost

      // If white has won, that means current color is black
      // Thus, value is ((-1 * 1) + 1) / 2 = 0, which is fine b/c current node (black) lost

      // If game is drawn:
      // value is ((±1 * 0) + 1) / 2 = 0.5, which is correct b/c it is a draw
      value = (clone->getCurrentColor().colorCode() * clone->getResult().evaluate() + 1.0) / 2.0;
    else // game is not over
      value = 0.5; // unknown outcome... assume 0.5 -> TODO find better default result

    propagate_result(searchPath, value, clone->getCurrentColor());

    delete clone;
  }
}

void tree::MCTS::propagate_result(std::vector<Node *> &searchPath, double value, piece::PieceColor color) {
  for (auto &i : searchPath) {
    i->increment_visit_count();
    i->increase_value(color == i->color_to_play() ? value: 1.0 - value);
  }
}

std::pair<game::Move, tree::Node *> tree::MCTS::select_optimal_move(Node *parent) {
  return parent->selectOptimalMove(score_move);
}

// UCB algorithm
double tree::MCTS::score_move(Node *parent, Node *child) {
  const double BASE_MULTIPLIER_BASE = 1965.0; // Straight from Google/DeepMind's AlphaZero paper - 19652.0 original
  const double BASE_MULTIPLIER_INIT = 2.75; // Straight from Google/DeepMind's AlphaZero paper - 1.25 original

  double base = log((parent->visit_count() + BASE_MULTIPLIER_BASE + 1.0) / BASE_MULTIPLIER_BASE) + BASE_MULTIPLIER_INIT;
  base *= sqrt(log(parent->visit_count() + 1) / (child->visit_count() + 1)); // exploration term
  // original -> base *= sqrt(parent -> visit_count()) / (child -> visit_count() + 1);

  return base * child->priority() + child->value();
}

double tree::MCTS::expand_node(tree::Node *node, game::Game *game, decider::Decider *move_ranker) {
  std::pair<double, std::map<game::Move, double>> prediction = move_ranker->prediction(game);

  int color_multiplier = node->color_to_play().colorCode();

  double sum_weights = 0.0;
  std::map<game::Move, double> weights, log_weights = prediction.second;
  for (auto &log_weight : log_weights) {
    weights[log_weight.first] = exp(color_multiplier * log_weight.second);
    sum_weights += weights[log_weight.first];
  }

  node->expand(weights, sum_weights);

  const double DRAW_THRESHOLD = 0.1;
  double eval = color_multiplier * prediction.first;
  // Map decider output to 0 for loss, 1 for win, and 0.5 for tie
  if (eval > DRAW_THRESHOLD)
    return 1.0;
  if (eval < -DRAW_THRESHOLD)
    return 0.0;
  return 0.5;
}

void tree::MCTS::add_dirichlet_noise(Node *node) {
  gsl_rng *rng = gsl_rng_alloc(gsl_rng_default);
  const double DIRICHLET_NOISE_ALPHA_VALUE = 0.15; // Straight from Google/DeepMind's AlphaZero paper - 0.2 original
  const double EXPLORATION_FRACTION_RATIO = 0.35; // Straight from Google/DeepMind's AlphaZero paper - 0.25 original

  int size = node->countChildren();

  auto *alphas = new double[size];
  auto *thetas = new double[size];
  int i;
  for (i = 0; i < size; ++i)
    alphas[i] = DIRICHLET_NOISE_ALPHA_VALUE;

  gsl_ran_dirichlet(rng, size, alphas, thetas);

  node->addNoise(EXPLORATION_FRACTION_RATIO, thetas);

  delete[] alphas;
  delete[] thetas;

  gsl_rng_free(rng);
}