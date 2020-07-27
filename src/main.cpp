#include <iostream>
#include <functional>
#include <chrono>
#include <ctime>
#include <thread>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "graphics/opengl.h"
#include "mcts_network/network.h"
#include "util/util.h"

void start_game(game::Game *game) { game->startGame(); }

int run_game() {
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

  network::NetworkStorage::flushStorage(false); // delete any stored networks

  return 0;
}

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
  // Start time
  std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
  time_t time1 = std::chrono::system_clock::to_time_t(time_on_start);

  // Run game
  std::cout << "starting game " << TRAINING_INDEX;
  game::GameResult result = create_game_training_case();
  std::cout << " - " << "ending game " << TRAINING_INDEX << std::endl;

  // End time
  std::chrono::system_clock::time_point time_on_end = std::chrono::system_clock::now();
  time_t time2 = std::chrono::system_clock::to_time_t(time_on_end);

  // Print start and end times
  std::cout << "Start: " << ctime(&time1) << "End: " << ctime(&time2);

  // Set up network training parameters
  double lambda = network::Optimizer::DEFAULT_INITIAL_LAMBDA;
  double lambda_max = network::Optimizer::DEFAULT_MAX_LEARNING_CONSTANT;
  double lambda_change = network::Optimizer::DEFAULT_LEARNING_RATE;

  // Train network
  for (int i = 0; i < network::Optimizer::DEFAULT_TRAINING_REPETITIONS; ++i)
    for (auto &board: boards_to_train_on)
      network::Optimizer::optimize(network::NetworkStorage::current_network(), board, 0.0,
                                   lambda, lambda_max, lambda_change);

  // Save network "if necessary"
  if (network::NetworkStorage::NETWORK_SAVE_INTERVAL != 0 &&
      (TRAINING_INDEX + 1) % network::NetworkStorage::NETWORK_SAVE_INTERVAL == 0)
    network::NetworkStorage::storeNetwork();

  // clear all previous training cases
  for (auto &board : boards_to_train_on)
    delete board;
  boards_to_train_on.clear();
}

int train_network() {
  network::NetworkStorage::NETWORK_SAVE_INTERVAL = 10;

  std::vector<game::Board *> boards_to_train_on;
  network::NetworkStorage::setTestCaseSelector([&](game::Board *b) -> void {
    if (math::chance(1.0 / 7.0)) // arbitrary 1/7 chance of any game state being a training case
      boards_to_train_on.push_back(b->clone());
  });

  network::NetworkStorage::initialize();

  if (network::NetworkStorage::NETWORK_SAVE_INTERVAL != 0)
    network::NetworkStorage::storeNetwork();

  const int NUM_TRAINING_ITERATIONS = 5;
  for (int i = 0; i < NUM_TRAINING_ITERATIONS; ++i)
    run_training_iteration(boards_to_train_on, i);

  network::NetworkStorage::flushStorage(network::NetworkStorage::NETWORK_SAVE_INTERVAL != 0); // delete any stored networks
  return 0;
}

int test() {
  piece::PieceColor color = piece::PieceColor::NONE;
  piece::PieceType type = piece::PieceType::NONE;
  int ind;

  std::string spacer, modifier;

  std::cin >> ind >> spacer >> type >> color >> modifier;
  std::cout << ind << spacer << type << color << modifier;

  return 0;
}

int main() {
  return run_game();
}