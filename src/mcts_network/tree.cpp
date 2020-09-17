#include "tree.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <vector>
#include <utility>
#include <map>
#include <cmath>
#include <functional>
#include <atomic>
#include <iostream>
#include <climits>

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
  for (auto &it : _children)
    delete it.second;
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
  _value_sum += std::abs(inc);
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
  double max_score = -1.0, score;

  for (auto &it : _children) {
    score = ranker(this, it.second);
    if (score > max_score) {
      max_score = score;
      optimal = {it.first, it.second};
    }
  }

  if (optimal.second == nullptr) DEBUG_ASSERT
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
int tree::MCTS::SIMULATION_SEARCH_DEPTH = 8;
int tree::MCTS::NUM_SIMULATIONS_PER_THREAD = 125;
int tree::MCTS::DEFAULT_NUM_THREADS = 4;

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
    DEBUG_ASSERT
    num_threads = roots.size();
  }

  std::atomic_int iteration_counter{NUM_SIMULATIONS_PER_THREAD * num_threads};
  std::atomic_int thread_counter{0};

  std::vector<game::Game *> clones(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    clones[i] = game->clone();

    expand_node(roots[i], clones[i], move_ranker);
    add_dirichlet_noise(roots[i]);

    thread::create(mcts, clones[i], move_ranker, roots[i], std::ref(iteration_counter), std::ref(thread_counter));
  }

  thread::wait_for([&] { return thread_counter >= num_threads; });

  for (auto &clone: clones)
    delete clone;
  clones.clear();

  auto *master_node = Node::combineNodes(roots, num_threads);
  return {select_optimal_move(master_node).first, master_node};
}

std::pair<game::Move, tree::Node *> tree::MCTS::run_mcts(game::Game *game, decider::Decider *move_ranker) {
  return run_mcts_multithreaded(game, 1, move_ranker);
}

void tree::MCTS::mcts(game::Game *game, decider::Decider *move_ranker, Node *root,
                      std::atomic_int &search_iteration_count, std::atomic_int &thread_finished_count) {
  Node *node;
  game::Game *clone;
  std::vector<Node *> searchPath;

  while (search_iteration_count-- > 0) {
    node = root;
    clone = game->clone();
    searchPath = {root};

    for (int i = 0; i < SIMULATION_SEARCH_DEPTH; ++i) {
      if (clone->isOver())
        break;
      if (!node->expanded())
        expand_node(node, clone, move_ranker);

      std::pair<game::Move, Node *> optimal = select_optimal_move(node);
      node = optimal.second;
      bool success = clone->tryMove(optimal.first);
      if (!success) {
        DEBUG_ASSERT
        delete clone;
        goto TERMINATE_LOOP; // jump to end of mcts
      }

      searchPath.push_back(node);
    }

    double curr_color_code = clone->getCurrentColor().value();
    double value;
    // If game is over
    if (clone->isOver()) {
      // If black has won, that means current color is white
      // Thus, value is ((1 * -1) + 1) / 2 = 0, which is fine b/c current node (white) lost

      // If white has won, that means current color is black
      // Thus, value is ((-1 * 1) + 1) / 2 = 0, which is fine b/c current node (black) lost

      // If game is drawn:
      // value is ((±1 * 0) + 1) / 2 = 0.5, which is correct b/c it is a draw
      value = (curr_color_code * clone->getResult().evaluate() + 1.0) / 2.0;
    } else { // game is not over
      // unknown outcome... using minimax board scoring -> TODO find better default result - finished??
      value = clone->board()->score(
        [&](piece::Piece *piece) -> double {
          return curr_color_code * piece->color().value() * piece->type().minimaxValue();
        }
      );
      // now apply sigmoid to value -> (-∞, ∞) maps to (0.0, 1.0)
      value = 1.0 / (1.0 + exp(-value / 27.18));
    }

    propagate_result(searchPath, value, clone->getCurrentColor());

    delete clone;
  }

  TERMINATE_LOOP:
  thread_finished_count++;
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
  // Straight from Google/DeepMind's AlphaZero paper
  const double BASE_MULTIPLIER_BASE = 19652.0;    // original
  const double BASE_MULTIPLIER_INIT = 1.25;       // original

  double base = log((parent->visit_count() + BASE_MULTIPLIER_BASE + 1.0) / BASE_MULTIPLIER_BASE) + BASE_MULTIPLIER_INIT;
  base *= sqrt(parent->visit_count()) / (child->visit_count() + 1);
  return base * child->priority() + child->value();

//  // Modified exploration scoring
//  const double BASE_MULTIPLIER_BASE = 1965.0;  // modified
//  const double BASE_MULTIPLIER_INIT = 2.75;    // modified
//
//  double base = log((parent->visit_count() + BASE_MULTIPLIER_BASE + 1.0) / BASE_MULTIPLIER_BASE) + BASE_MULTIPLIER_INIT;
//  base *= sqrt(log(parent->visit_count() + 1) / (child->visit_count() + 1)); // exploration term
//  return base * child->priority() + child->value();
}

double tree::MCTS::expand_node(tree::Node *node, game::Game *game, decider::Decider *move_ranker) {
  std::pair<double, std::map<game::Move, double>> prediction = move_ranker->prediction(game);

  double color_multiplier = node->color_to_play().value();

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
  // set up rng
  gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set(rng, (long) ((double) LONG_MAX * math::random()));

  // Straight from Google/DeepMind's AlphaZero paper
  const double DIRICHLET_NOISE_ALPHA_VALUE = 0.2;
  const double EXPLORATION_FRACTION_RATIO = 0.25;

//  // Modified exploration values
//  const double DIRICHLET_NOISE_ALPHA_VALUE = 0.15;
//  const double EXPLORATION_FRACTION_RATIO = 0.35;

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