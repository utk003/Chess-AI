#include <iostream>
#include <functional>
#include <chrono>
#include <ctime>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "graphics/opengl.h"
#include "mcts_network/network.h"
#include "util/thread_util.h"

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

void run_training_iteration(std::vector<game::Board *> &boards_to_train_on, const int TRAINING_INDEX) {
//  // Start time
//  std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
//  time_t time1 = std::chrono::system_clock::to_time_t(time_on_start);
//
//  // Run game
//  std::cout << "starting game " << TRAINING_INDEX;
  game::GameResult result = create_game_training_case();
//  std::cout << " - " << "ending game " << TRAINING_INDEX << std::endl;
//
//  // End time
//  std::chrono::system_clock::time_point time_on_end = std::chrono::system_clock::now();
//  time_t time2 = std::chrono::system_clock::to_time_t(time_on_end);
//
//  // Print start and end times
//  std::cout << "Start: " << ctime(&time1) << "End: " << ctime(&time2);

  // Set up network training parameters
  double lambda = network::Optimizer::DEFAULT_INITIAL_LAMBDA;
  double lambda_max = network::Optimizer::DEFAULT_MAX_LEARNING_CONSTANT;
  double lambda_change = network::Optimizer::DEFAULT_LEARNING_RATE;

  // Train network
  for (int i = 0; i < network::Optimizer::DEFAULT_TRAINING_REPETITIONS; ++i)
    for (auto &board: boards_to_train_on)
      network::Optimizer::optimize(network::NetworkStorage::current_network(), board, result.evaluate() * 100.0,
                                   lambda, lambda_max, lambda_change);

  // Save network "if necessary"
  if ((TRAINING_INDEX + 1) % NETWORK_SAVE_INTERVAL == 0)
    network::NetworkStorage::saveNetwork();

  // clear all previous training cases
  for (auto &board : boards_to_train_on)
    delete board;
  boards_to_train_on.clear();
}

int train_network() {
  NETWORK_SAVE_INTERVAL = 20;
  network::NetworkStorage::SAVE_NETWORKS = true;

  std::vector<game::Board *> boards_to_train_on;
  network::NetworkStorage::setTestCaseSelector([&](game::Board *b) -> void {
    // arbitrary 1/5 chance of any game state being a training case
    if (math::chance(1.0 / 5.0))
      boards_to_train_on.push_back(b->clone());
  });

  network::NetworkStorage::initialize();

  const int NUM_TRAINING_ITERATIONS = 1000;
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
#if defined(__clang__)
  std::cout << "clang" << std::endl;
#elif defined(__GNUC__)
  std::cout << "gcc" << std::endl;
#endif

  return 0;
}

int main() {
  return test();
}