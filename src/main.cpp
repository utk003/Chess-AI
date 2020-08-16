#include <iostream>
#include <functional>
#include <chrono>
#include <ctime>
#include <utility>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "graphics/opengl.h"
#include "mcts_network/network.h"
#include "util/thread_util.h"
#include "util/math_util.h"

int run_game() {
  network::NetworkStorage::initialize();

  auto *game = new game::Game(new game::Board(8, 8));
  game->board()->loadFromFile("../assets/game_states/chess_default_start.txt");

  game->setPlayer(piece::PieceColor::WHITE, player::PlayerType::HUMAN);
  game->setPlayer(piece::PieceColor::BLACK, player::PlayerType::HUMAN);

  // start graphics engine
  graphics::OpenGL *openGL = graphics::OpenGL::get_instance(game, "Chess");
  // start game
  game->startGame();
  // run graphics
  openGL->run();

  // once graphics closes, delete graphics engine
  delete openGL;

  // tell game to terminate
  game->endGame();
  // wait for game to finish processes, then delete game
  game->waitForDelete();
  delete game;

  network::NetworkStorage::flushStorage(); // delete any stored networks

  return 0;
}

int NETWORK_SAVE_INTERVAL = 0;

game::GameResult create_game_training_case() {
  auto *game = new game::Game(new game::Board(8, 8));
  game->board()->loadFromFile("../assets/game_states/chess_default_start.txt");

  game->setPlayer(piece::PieceColor::WHITE, player::PlayerType::AI);
  game->setPlayer(piece::PieceColor::BLACK, player::PlayerType::AI);

  game->startGame();
  thread::wait_for([&] { return game->isOver(); });
  game::GameResult result = game->getResult();

  // tell game to terminate
  game->endGame();
  // wait for game to finish processes, then delete game
  game->waitForDelete();
  delete game;

  return result;
}

double get_overall_result(double scale, double mcts_result, double game_result) {
  static double mcts_weight = 5.0;
  static double game_result_weight = 1.0;
  static double weights_sum = mcts_weight + game_result_weight; // 6.0

  return scale * (mcts_weight * mcts_result + game_result_weight * game_result) / weights_sum; // 5:1 weight
}

void
run_training_iteration(std::vector<std::pair<game::Board *, double>> &boards_to_train_on, const int TRAINING_INDEX) {
  game::GameResult result = create_game_training_case();

  // Set up network training parameters
  double lambda = network::Optimizer::DEFAULT_INITIAL_LAMBDA;
  double lambda_max = network::Optimizer::DEFAULT_MAX_LEARNING_CONSTANT;
  double lambda_change = network::Optimizer::DEFAULT_LEARNING_RATE;

  // Train network
  for (int i = 0; i < network::Optimizer::DEFAULT_TRAINING_REPETITIONS; ++i)
    for (auto &pair: boards_to_train_on)
      network::Optimizer::optimize(network::NetworkStorage::current_network(), pair.first,
                                   get_overall_result(20.0, pair.second, result.evaluate()),
                                   lambda, lambda_max, lambda_change);

  // Save network "if necessary"
  if ((TRAINING_INDEX + 1) % NETWORK_SAVE_INTERVAL == 0)
    network::NetworkStorage::saveNetwork();

  // clear all previous training cases
  for (auto &pair : boards_to_train_on)
    delete pair.first;
  boards_to_train_on.clear();
}

bool load_prev_network = false;
int train_network() {
  NETWORK_SAVE_INTERVAL = 20;
  network::NetworkStorage::SAVE_NETWORKS = true;

  std::vector<std::pair<game::Board *, double>> boards_to_train_on;
  network::NetworkStorage::setTestCaseSelector([&](game::Board *b, double d) -> void {
    // arbitrary 1/5 chance of any game state being a training case
    if (math::chance(1.0 / 5.0)) {
      double bounded_mcts_result = 2.0 * d - 1.0;                     // transform result from [0.0, 1.0] to [-1.0, 1.0]
      math::clamp(bounded_mcts_result, {-1.0, 1.0});   // clamp result to [-1.0, 1.0]
      boards_to_train_on.emplace_back(b->clone(), bounded_mcts_result); // store board AND result
    }
  });

//  load_prev_network = true;
  if (load_prev_network)
    network::NetworkStorage::initialize(network::NetworkStorage::LATEST_NETWORK);
  else
    network::NetworkStorage::initialize();

  const int NUM_TRAINING_ITERATIONS = 1;
  for (int i = 0; i < NUM_TRAINING_ITERATIONS; ++i) {
    run_training_iteration(boards_to_train_on, i);

    // print iteration completion time
    std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
    time_t time1 = std::chrono::system_clock::to_time_t(time_on_start);
    std::cout << "Completed Iteration #" << i + 1 << ": " << ctime(&time1);
  }

  network::NetworkStorage::flushStorage(); // delete any stored networks
  return 0;
}

int test() {
  math::random();
  return 0;
}

int main() {
  return run_game();
}