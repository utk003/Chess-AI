// ------------------------------------------------------------------------------ //
// MIT License                                                                    //
//                                                                                //
// Copyright (c) 2020 Utkarsh Priyam                                              //
//                                                                                //
// Permission is hereby granted, free of charge, to any person obtaining a copy   //
// of this software and associated documentation files (the "Software"), to deal  //
// in the Software without restriction, including without limitation the rights   //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      //
// copies of the Software, and to permit persons to whom the Software is          //
// furnished to do so, subject to the following conditions:                       //
//                                                                                //
// The above copyright notice and this permission notice shall be included in all //
// copies or substantial portions of the Software.                                //
//                                                                                //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  //
// SOFTWARE.                                                                      //
// ------------------------------------------------------------------------------ //

#include "initialization.h"

#include <filesystem>

#ifdef _WIN32
#  include <direct.h>
#  define cd _chdir
#else
#  include <unistd.h>
#  define cd chdir
#endif

#include <thread>
#include <iostream>
#include <cmath>
#include <functional>
#include <utility>

#include "../mcts_network/tree.h"

// extern variables
bool settings::PRINT_INITIALIZATION_DEBUG_INFORMATION = true;
bool settings::PRINT_GAME_SIMULATION_DEBUG_INFORMATION = false;

int settings::GAMES_PER_NETWORK_SAVE = -1;

std::function<bool()> settings::TRAINING_TERMINATION_CONDITION;
std::function<void(std::vector<std::pair<game::Board *, double>> &, game::Board *, double)>
  settings::SIMULATION_BOARD_SAVE_PROCEDURE;

// methods
void printNewLine(std::ostream &out = std::cout) {
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    out << std::endl;
}

void printSeed(const std::string &rng_seed, std::ostream &out = std::cout) {
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION) {
    out << "Random Number Generator Seed: \"";
    int lenMin1 = (int) rng_seed.length() - 1;
    if (rng_seed[lenMin1] != '\n')
      out << rng_seed;
    else
      out << rng_seed.substr(0, lenMin1) << "\\n";
    out << "\"" << std::endl;
  }
}

void init::initializeRNG(const std::string &rng_seed) {
  std::string string_seed = rng_seed;
  if (rng_seed.empty()) {
    auto time_on_start = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(time_on_start);
    string_seed = std::string(ctime(&time));
  }

  std::seed_seq twister_seed(string_seed.begin(), string_seed.end());
  math::mersenne_twister_32bit_randomizer = std::mt19937(twister_seed);
  math::rng_distribution = std::uniform_real_distribution<double>{0.0, 1.0};

  printSeed(string_seed);
  printNewLine();
}

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

void init::updateWorkingDirectory(const std::string &target_dir) {
  if (isAbsolute(target_dir)) {
    std::string target = target_dir;
    auto lenMin1 = target.length() - 1;
    if (target[lenMin1] == '/' || target[lenMin1] == '\\')
      target = target.substr(0, lenMin1);

    cd(target.c_str());
    if (std::filesystem::current_path() != target) {
      if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
        std::cerr << "Invalid working directory" << std::endl;
      FATAL_ASSERT
    }
  } else {
    if (target_dir.find_last_of("/\\") != std::string::npos) {
      FATAL_ASSERT
      return;
    }

    std::string current_path = std::filesystem::current_path();
    if (current_path.find(target_dir) == std::string::npos) {
      FATAL_ASSERT
      return;
    }

    std::string suffix = last_dir(current_path);
    while (suffix != target_dir) {
      current_path = current_path.substr(0, current_path.length() - suffix.length());
      suffix = last_dir(current_path);
      cd("..");
    }
  }

  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Current Working Directory: " << std::filesystem::current_path() << std::endl;

  printNewLine();
}

void init::updateMCTSParameters(double thread_usage_ratio, int simulation_move_depth, int simulations_per_thread) {
  int num_available_threads = (int) std::thread::hardware_concurrency();

  if (0.0 < thread_usage_ratio && thread_usage_ratio < 1.0) {
    tree::MCTS::DEFAULT_NUM_THREADS = (int) lround(num_available_threads * thread_usage_ratio);
    if (num_available_threads - tree::MCTS::DEFAULT_NUM_THREADS < 4)
      tree::MCTS::DEFAULT_NUM_THREADS = num_available_threads - 4;
  } else
    tree::MCTS::DEFAULT_NUM_THREADS = num_available_threads - 4;
  std::max(tree::MCTS::DEFAULT_NUM_THREADS, 1);

  tree::MCTS::SIMULATION_SEARCH_DEPTH = simulation_move_depth;
  tree::MCTS::NUM_SIMULATIONS_PER_THREAD = simulations_per_thread;

  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION) {
    std::cout << "Number of Available Threads: " << num_available_threads << std::endl;
    std::cout << "Number of MCTS Working Threads: " << tree::MCTS::DEFAULT_NUM_THREADS << std::endl;
    double utilization_ratio = 100.0 * tree::MCTS::DEFAULT_NUM_THREADS / num_available_threads;
    std::cout << "Thread Utilization Percentage: " << utilization_ratio << "%" << std::endl;
  }

  printNewLine();
}

void init::updateNetworkSettings(bool load_prev_net, bool save_net, const std::string &net_file_path) {
  if (load_prev_net) {
    if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
      std::cout << "Loading previous network (from \"" << net_file_path << "\")" << std::endl;
    network::NetworkStorage::initialize(net_file_path);
  } else {
    if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
      std::cout << "Generating new random network" << std::endl;
    network::NetworkStorage::initialize();
  }

  network::NetworkStorage::SAVE_NETWORKS = save_net;
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION) {
    if (network::NetworkStorage::SAVE_NETWORKS)
      std::cout << "Networks will be saved during training" << std::endl;
    else
      std::cout << "Networks will NOT be saved during training" << std::endl;
  }

  printNewLine();
}

void print(const std::string &desc, int val, const std::string &unit, bool putS = true, std::ostream &out = std::cout) {
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION) {
    out << desc << ": " << val << " " << unit;
    if (putS && val != 1)
      out << "s";
    out << std::endl;
  }
}

void init::updateTrainingParameters(const std::function<bool()> &termination_condition,
                                    int network_save_interval_in_games, double ratio_of_training_boards_to_save) {
  settings::GAMES_PER_NETWORK_SAVE = network_save_interval_in_games;
  print("Network Save Interval (for training)", network_save_interval_in_games, "training case");

  settings::TRAINING_TERMINATION_CONDITION = termination_condition;
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Training Termination Condition loaded" << std::endl;

  settings::SIMULATION_BOARD_SAVE_PROCEDURE = [&](auto &vec, game::Board *b, double d) -> void {
    if (math::chance(ratio_of_training_boards_to_save)) {
      double bounded_mcts_result = 2.0 * d - 1.0;                       // transform from [0.0, 1.0] to [-1.0, 1.0]
      math::clamp(bounded_mcts_result, {-1.0, 1.0});    // clamp result to [-1.0, 1.0]
      vec.emplace_back(b->clone(), bounded_mcts_result);                // store board AND result
    }
  };
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << 100 * ratio_of_training_boards_to_save << "% of Board Training Cases will be saved" << std::endl;

  printNewLine();
}

bool init::verify(bool is_fatal_if_fail) {
  if (network::NetworkStorage::current_network() == nullptr) {
    if (is_fatal_if_fail) FATAL_ASSERT
    else DEBUG_ASSERT
    return false;
  }
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Network is loaded properly" << std::endl;

  if (!settings::SIMULATION_BOARD_SAVE_PROCEDURE) {
    if (is_fatal_if_fail) FATAL_ASSERT
    else DEBUG_ASSERT
    return false;
  }
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Program can save training cases" << std::endl;

  if (settings::GAMES_PER_NETWORK_SAVE <= 0) {
    if (is_fatal_if_fail) FATAL_ASSERT
    else DEBUG_ASSERT
    return false;
  }
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Network Save Interval is well-defined" << std::endl;

  if (!settings::TRAINING_TERMINATION_CONDITION) {
    if (is_fatal_if_fail) FATAL_ASSERT
    else DEBUG_ASSERT
    return false;
  }
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Training can terminate safely" << std::endl;

  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Program is ready to simulate games" << std::endl;

  printNewLine();
  return true;
}