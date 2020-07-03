#include "tree.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <cmath>
#include <iostream>

#include "chess/game.h"
#include "network.h"

// Node class
tree::Node::Node() : Node(0.0f) {}
tree::Node::Node(float prior) {
  _priority = prior;

  _visit_count = 0;
  _value_sum = 0.0f;

  _color_to_play = piece::PieceColor::NONE;
}

tree::Node::~Node() {
  _children.clear();
}

// Monte Carlo Tree Search class
std::pair<game::Move, tree::Node*> tree::MCTS::run_mcts(game::Game* game, network::Decider* move_ranker) {
  Node* root = new Node();
  Node* node = nullptr;
  game::Game* clone = game -> clone();
  std::vector<Node*> searchPath;
  expand_node(root, clone, move_ranker);
  add_dirichlet_noise(root);
  
  float value;

  int i;
  for (i = 0; i < NUM_SIMULATIONS; ++i) {
    node = root;
    delete clone;
    clone = game -> clone();
    searchPath = {root};

    int c = 0;

    while (node -> expanded()) {
      std::pair<game::Move, Node*> optimal = select_optimal_child(node);
      node = optimal.second;
      assert(clone -> processMove(optimal.first)); // must complete processing -> success
      searchPath.push_back(node);
      ++c;
    }

    value = expand_node(node, clone, move_ranker);

    propagate_result(searchPath, value, clone -> getCurrentColor());
  }

  delete clone;

  return {select_optimal_move(root), root};
}

game::Move tree::MCTS::select_optimal_move(Node* root) {
  std::map<game::Move, Node*> children = root -> children();

  std::map<game::Move, Node*>::iterator it = children.begin();
  game::Move optimal = it -> first;
  float max_visits = it -> second -> visit_count(), visits;

  ++it; // go to next element;

  for (/* Nothing here */; it != children.end(); ++it) {
    visits = it -> second -> visit_count();
    if (visits > max_visits) {
      max_visits = visits;
      optimal = it -> first;
    }
  }

  return optimal;
}

void tree::MCTS::propagate_result(std::vector<Node*> searchPath, float value, piece::PieceColor color) {
  for (int i = 0; i < searchPath.size(); ++i) {
    searchPath[i] -> _visit_count += 1;
    searchPath[i] -> _value_sum += color == searchPath[i] -> color_to_play() ? value: 1.0f - value;
  }
}

std::pair<game::Move, tree::Node*> tree::MCTS::select_optimal_child(Node* parent) {
  std::map<game::Move, Node*> children = parent -> children();

  std::map<game::Move, Node*>::iterator it = children.begin();
  std::pair<game::Move, Node*> optimal {it -> first, it -> second};
  float max_score = score_move(parent, it -> second), score;

  ++it; // go to next element;

  for (/* Nothing here */; it != children.end(); ++it) {
    score = score_move(parent, it -> second);
    if (score > max_score) {
      max_score = score;
      optimal = {it -> first, it -> second};
    }
  }

  return optimal;
}

// UCB algorithm
float tree::MCTS::score_move(Node* parent, Node* child) {
  const float BASE_MULTIPLIER_BASE = 19652.0f;
  const float BASE_MULTIPLIER_INIT = 1.25f;
  float base = log((parent -> visit_count() + BASE_MULTIPLIER_BASE + 1.0f) / BASE_MULTIPLIER_BASE) + BASE_MULTIPLIER_INIT;
  base *= sqrt(parent -> visit_count()) / (child -> visit_count() + 1);

  return base * child -> priority() + child -> value();
}

float tree::MCTS::expand_node(tree::Node* node, game::Game* game, network::Decider* move_ranker) {
  std::pair<float, std::map<game::Move, float>> prediction = move_ranker -> prediction(game);

  node -> _color_to_play = game -> getCurrentColor();

  float sum_weights = 0.0f;
  std::map<game::Move, float> weights, log_weights = prediction.second;
  for (std::map<game::Move, float>::iterator it = log_weights.begin(); it != log_weights.end(); ++it) {
    weights[it -> first] = exp(it -> second);
    sum_weights += weights[it -> first];
  }

  for (std::map<game::Move, float>::iterator it = weights.begin(); it != weights.end(); ++it)
    node -> _children[it -> first] = new Node(it -> second / sum_weights);

  const float DRAW_THRESHOLD = 0.1f;
  float eval = prediction.first;
  // Map network output to 0 for loss, 1 for win, and 0.5 for tie
  if (eval > DRAW_THRESHOLD)
    return 1.0f;
  if (eval < -DRAW_THRESHOLD)
    return 0.0f;
  return 0.5f;
}

void tree::MCTS::add_dirichlet_noise(Node* node) {
  gsl_rng* rng = gsl_rng_alloc(gsl_rng_default);
  const float DIRICHLET_NOISE_ALPHA_VALUE = 0.3f; // Straight from Google/DeepMind's AlphaZero paper
  const float EXPLORATION_FRACTION_RATIO = 0.25f; // Straight from Google/DeepMind's AlphaZero paper

  std::map<game::Move, Node*> children = node -> children();
  int size = children.size();

  double* alphas = new double[size];
  double* thetas = new double[size];
  int i;
  for (i = 0; i < size; ++i)
    alphas[i] = DIRICHLET_NOISE_ALPHA_VALUE;
  
  gsl_ran_dirichlet(rng, size, alphas, thetas);

  i = 0;
  float frac = EXPLORATION_FRACTION_RATIO;
  for (std::map<game::Move, Node*>::iterator it = children.begin(); it != children.end(); ++it) {
    it -> second -> _priority = it -> second -> _priority * (1 - frac) + thetas[i] * frac;
    ++i;
  }

  delete[] alphas;
  delete[] thetas;

  gsl_rng_free(rng);
}