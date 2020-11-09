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

#include "main/initialization.h"
#include "main/run_game.h"
#include "main/network/make_cases.h"
#include "main/network/train.h"

#include <thread>

#include "mcts_network/tree.h"
#include "util/thread_util.h"

// command line arguments
char **arguments;
int num_args;

// create termination lambda variables
int game_count = 0, end_count = 1000; // end_count = how many games to simulate for training before exiting the program

// The initialize() method initializes all of the different modules of this program
// and ensures that all necessary preconditions are met.
//
// Initialization can be verified using init::verify(), which will return true iff
// the program believes that it is ready to run any of its features, including but
// not limited to training the network and playing a game with graphics (using OpenGL).
void initialize(const std::function<bool()> &termination_condition = [] { return true; }) {
  settings::PRINT_INITIALIZATION_DEBUG_INFORMATION = true;
  settings::PRINT_GAME_SIMULATION_DEBUG_INFORMATION = true;

  // Update current working directory
  num_args <= 0 ? init::updateWorkingDirectory(): init::updateWorkingDirectory(arguments[0]);

  // param 1: (string) rng seed - default = load from system clock time
  init::initializeRNG();

  // param 1: (double) fraction of threads to use per MCTS - default = 100% usage
  // param 2: (int) number of moves deep to search (MCTS) - default = 8 moves
  // param 3: (int) number of search iterations (MCTS) - default = 125 iterations
  init::updateMCTSParameters(0.15, 2, 125);

  // param 1: (bool) load previous network from file - default = true
  // param 2: (bool) save trained networks to files - default = true
  // param 3: (string) file path to previous network - default = "network_dump/latest.txt"
  init::updateNetworkSettings();
  // param 1: (lambda) when to terminate program training - default = return true; = do not train
  // param 2: (int) network training save/dropout interval - default = 20 iterations
  // param 3: (double) ratio of boards from simulations to save - default = 100% saved
  init::updateTrainingParameters(termination_condition, 100);
  // for this ^^ (lambda), remember that counter vars need to remain existent after this method's execution finishes
  // ie make sure the counters aren't local variables or uNdEfInEd BeHaViOr....
  if (!settings::PRINT_INITIALIZATION_DEBUG_INFORMATION)
    std::cout << "Program Initialization Complete!" << std::endl << std::endl;
}

void training_helper(std::atomic_bool &done, const std::vector<std::string> &vec) {
  network::train::train_network(vec);
  done.store(true);
}

void simulation_helper(std::atomic_bool &done, std::vector<std::string> &vec, int num_game_sims) {
  network::generate_training_cases(vec, game_count, num_game_sims);
  done.store(true);
}

void execute_training() {
  int num_threads = (int) std::thread::hardware_concurrency() - 3;
  int threads_per_sim = tree::MCTS::DEFAULT_NUM_THREADS + 2;
  int num_game_sims = 0;
  while (num_threads >= threads_per_sim) {
    num_threads -= threads_per_sim;
    ++num_game_sims;
  }

  std::vector<std::string> files;
  for (int i = 0; i < 66; ++i)
    files.push_back("board_" + std::to_string(i));

  std::atomic_bool done1(false), done2(false);

  std::cout << "Launching Network Trainer: " << files.size() << " preloaded files for training" << std::endl;
  thread::create(training_helper, std::ref(done1), files);

  std::cout << "Launching Game Simulator: " << num_game_sims << " simulations per cycle" << std::endl;
  thread::create(simulation_helper, std::ref(done2), std::ref(files), num_game_sims);

  std::cout << std::endl << "Training will complete after " << end_count << " simulation";
  if (end_count != 1)
    std::cout << "s";
  std::cout << std::endl << std::endl;
  thread::wait_for([&] { return done1 && done2; });

  std::cout << "Program Execution Complete!!" << std::endl << std::endl;
}

void execute_gameplay(player::PlayerType white = player::PlayerType::HUMAN,
                      player::PlayerType black = player::PlayerType::AI) {
  std::cout << "Starting Game" << std::endl << std::endl;
  game::run_game(white, black, true);
  std::cout << "Program Execution Complete!!" << std::endl << std::endl;
}

// The execute() method is the core of the entire program, where all of the independent
// functions provided in this program can be run. Specifically, the program can decide
// to run training procedures or play an actual game between 2 human players, between
// a human and a computer, or even between 2 computers!
//
// There is planned support for external (possibly command-line-based) modifiers which
// will allow users to configure program usage at runtime rather than compile-time.
//
// There is also a planned UI which will allow users to select configurations while the
// program is running. However, this is not a high-priority feature.
void execute() {
//  execute_training();
//  execute_gameplay(); // white, black
}

// The terminate() method deletes any pointers, etc. and clears any containers.
// It's main role is to prevent memory leaks by responsibly managing data.
//
// The method effectively acts as a reset switch, by clearing most settings
// and freeing memory allocated during the initialization phase of the program.
//
// This method can be thought of as the inverse of the initialize() method.
void terminate() {
  std::cout << "Releasing remaining allocated memory..." << std::endl << std::endl;
  network::NetworkStorage::flushStorage(); // delete any stored networks
}

void printSpacing(int front, int end, std::ostream &out = std::cout) {
  for (int i = 0; i < front; ++i)
    out << "\n";
  out << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-";
  for (int i = 0; i < end; ++i)
    out << "\n";
  out.flush();
}

// The main() method is the main method from where the initialize(), execute(),
// and terminate() methods are called. It also saves any command-line arguments.
int main(int len, char **args) {
  // shift arguments by 1 b/c args[0] is the command used to run the program??
  arguments = args + 1;
  num_args = len - 1;

  // initialize, run, and close program
  printSpacing(1, 2);
  initialize([&] { return game_count >= end_count; });

  printSpacing(0, 2);
  if (settings::PRINT_INITIALIZATION_DEBUG_INFORMATION) {
    std::cout << "PROGRAM INITIALIZATION VERIFICATION:" << "\n";
    if (init::verify()) {
      printSpacing(0, 2);
      execute(); // execute iff initialization worked properly
    } else
      std::cout << std::endl;
  } else if (init::verify())
    execute(); // execute iff initialization worked properly

  printSpacing(0, 2);
  terminate();

  printSpacing(0, 1);

  // exit
  return 0;
}