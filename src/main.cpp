#include <iostream>
#include <chrono>
#include <ctime>
#include <utility>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "graphics/opengl.h"
#include "mcts_network/network.h"
#include "mcts_network/tree.h"
#include "util/thread_util.h"
#include "util/math_util.h"
#include "util/string_util.h"

bool LOAD_PREV_NETWORK = false;
int NUM_TRAINING_ITERATIONS = 1;
int NETWORK_SAVE_INTERVAL = 20;

int run_game(player::PlayerType white = player::PlayerType::HUMAN,
             player::PlayerType black = player::PlayerType::HUMAN,
             const std::string &s = "assets/game_states/chess_default_start.txt") {

  if (LOAD_PREV_NETWORK)
    network::NetworkStorage::initialize(network::NetworkStorage::LATEST_NETWORK_FILE_PATH);
  else
    network::NetworkStorage::initialize();

  auto *game = new game::Game(new game::Board(8, 8));
  game->board()->loadFromFile(s);

  game->setPlayer(piece::PieceColor::WHITE, white);
  game->setPlayer(piece::PieceColor::BLACK, black);

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

game::GameResult create_game_training_case() {
  auto *game = new game::Game(new game::Board(8, 8));
  game->board()->loadFromFile("assets/game_states/chess_default_start.txt");

  game->setPlayer(piece::PieceColor::WHITE, player::PlayerType::AI);
  game->setPlayer(piece::PieceColor::BLACK, player::PlayerType::MCTS);

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

double get_overall_result(double mcts_result, double game_result, double scale = 20.0) {
  static double mcts_weight = 5.0;
  static double game_result_weight = 1.0;
  static double weights_sum = mcts_weight + game_result_weight; // 6.0

  return scale * (mcts_weight * mcts_result + game_result_weight * game_result) / weights_sum; // 5:1 weight
}

long file_counter = 0;
void
run_training_iteration(std::vector<std::pair<game::Board *, double>> &boards_to_train_on, const int TRAINING_INDEX) {
  game::GameResult result = create_game_training_case();

  // Update training case target value
  for (auto &pair : boards_to_train_on) {
    // update
    pair.second = get_overall_result(pair.second, result.evaluate());
    // save case to file
    pair.first->saveToFile("training_case/case_" + std::to_string(file_counter++),
                           [&](std::ostream &out) -> void { out << string::from_double(pair.second) << std::endl; });
  }

  // Set up network training parameters
  double lambda = network::Optimizer::DEFAULT_INITIAL_LAMBDA;
  double lambda_max = network::Optimizer::DEFAULT_MAX_LEARNING_CONSTANT;
  double lambda_change = network::Optimizer::DEFAULT_LEARNING_RATE;

  // Train network
  network::Network *network = network::NetworkStorage::current_network();
  int training_reps = 2 * std::max((int) boards_to_train_on.size(), network::Optimizer::DEFAULT_TRAINING_REPETITIONS);
  for (int i = 0; i < training_reps; ++i) // More training = good ???
    for (auto &pair: boards_to_train_on)
      network::Optimizer::optimize(network, pair.first, pair.second, lambda, lambda_max, lambda_change);

  // Save network "if necessary"
  if (TRAINING_INDEX % NETWORK_SAVE_INTERVAL == 0)
    network::NetworkStorage::saveNetwork();

  // clear all previous training cases
  for (auto &pair : boards_to_train_on)
    delete pair.first;
  boards_to_train_on.clear();

  // print iteration completion time
  std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
  time_t time1 = std::chrono::system_clock::to_time_t(time_on_start);
  std::cout << "Completed Iteration #" << TRAINING_INDEX << ": " << ctime(&time1);
}

int train_network() {
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

  if (LOAD_PREV_NETWORK)
    network::NetworkStorage::initialize(network::NetworkStorage::LATEST_NETWORK_FILE_PATH);
  else
    network::NetworkStorage::initialize();

  for (int i = 1; i <= NUM_TRAINING_ITERATIONS; ++i)
    run_training_iteration(boards_to_train_on, i);

  network::NetworkStorage::flushStorage(); // delete any stored networks
  return 0;
}

int test() {
  auto *game = new game::Game(new game::Board(8, 8));
  game->board()->loadFromFile("assets/game_states/chess_default_start.txt");

  for (int i = 0; i < 10000; ++i)
    delete game->clone();
  std::cout << "Done generating" << std::endl;
  thread::sleep_seconds(15);
//  std::cout << "Done deleting" << std::endl;
//  thread::sleep_seconds(15);
  std::cout << "Over" << std::endl;

  return 0;
}

// Directory methods/macros
#include <filesystem>

#ifdef _WIN32
#  include <direct.h>
#  define cd _chdir // MSDN recommends against using chdir name
#else

#  include <unistd.h>

#  define cd chdir
#endif

std::string last_dir(const std::string &path) {
  if (path.length() == 0)
    return "";

  int ind = path.find_last_of("/\\");
  if (ind + 1 == path.length())
    return last_dir(path.substr(0, path.length() - 1));
  else
    return path.substr(ind + 1);
}

bool isAbsolute(const std::string &dir) {
#ifdef _WIN32
  switch (dir.length()) {
    case 0:
      return false;

    case 1:
    case 2:
      return dir[0] == '\\';

    default:
      return dir[0] == '\\' || (dir[1] == ':' && dir[2] == '\\');
  }
#else
  return dir.length() > 0 && (dir[0] == '/' || dir[0] == '~');
#endif
}

void updateWorkingDirectory(const std::string &target_dir = "Chess-AI") {
  if (isAbsolute(target_dir)) {
    std::string target = target_dir;
    auto lenMin1 = target.length() - 1;
    if (target[lenMin1] == '/' || target[lenMin1] == '\\')
      target = target.substr(0, lenMin1);

    cd(target.c_str());
    if (std::filesystem::current_path() != target) {
      std::cerr << "Invalid working directory" << std::endl;
      fatal_assert();
    }
  } else {
    if (target_dir.find_last_of("/\\") != std::string::npos) {
      debug_assert();
      return;
    }

    std::string current_path = std::filesystem::current_path();
    if (current_path.find(target_dir) == std::string::npos) {
      debug_assert();
      return;
    }

    std::string suffix = last_dir(current_path);
    while (suffix != target_dir) {
      current_path = current_path.substr(0, current_path.length() - suffix.length());
      suffix = last_dir(current_path);
      cd("..");
    }
  }
}

void updateMCTSParameters() {
  int num_available_threads = (int) std::thread::hardware_concurrency();

  tree::MCTS::DEFAULT_NUM_THREADS = std::max(num_available_threads - 4, 1);
  tree::MCTS::SIMULATION_SEARCH_DEPTH = 8;
  tree::MCTS::NUM_SIMULATIONS_PER_THREAD = 125;

  std::cout << "Number of Available Threads: " << num_available_threads << std::endl;
  std::cout << "Number of MCTS Working Threads: " << tree::MCTS::DEFAULT_NUM_THREADS << std::endl;
}

void updateWorkingDirectory(int num_args, char **args) {
  if (num_args <= 1)
    updateWorkingDirectory();
  else
    updateWorkingDirectory(args[1]); // args[0] is the command used to run this program??

  std::cout << "Current Working Directory: " << std::filesystem::current_path() << std::endl;
}

void updateTrainingParameters() {
  LOAD_PREV_NETWORK = false;
  std::cout << (LOAD_PREV_NETWORK ? "Loading previous network (from \"network-dump/latest.txt\")"
                                  : "Generating new random network") << std::endl;

  NUM_TRAINING_ITERATIONS = 100;
  std::cout << "Number of Training Iterations: " << NUM_TRAINING_ITERATIONS << " iteration";
  if (NUM_TRAINING_ITERATIONS != 1)
    std::cout << "s";
  std::cout << std::endl;

  NETWORK_SAVE_INTERVAL = 1;
  std::cout << "Network Save Interval: " << NETWORK_SAVE_INTERVAL << " iteration";
  if (NETWORK_SAVE_INTERVAL != 1)
    std::cout << "s";
  std::cout << std::endl;
}

int main(int num_args, char **args) {
  updateWorkingDirectory(num_args, args);
  std::cout << std::endl;

  updateMCTSParameters();
  std::cout << std::endl;

  updateTrainingParameters();
  std::cout << std::endl;

  return train_network();//run_game(player::PlayerType::HUMAN, player::PlayerType::AI);
}