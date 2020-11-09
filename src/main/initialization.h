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

#ifndef CHESS_AI_MAIN_INITIALIZATION_H_
#define CHESS_AI_MAIN_INITIALIZATION_H_

#include <string>
#include <functional>

#include "../mcts_network/network.h"

namespace settings {

extern bool PRINT_INITIALIZATION_DEBUG_INFORMATION;
extern bool PRINT_GAME_SIMULATION_DEBUG_INFORMATION;

extern int GAMES_PER_NETWORK_SAVE;
extern std::function<bool()> TRAINING_TERMINATION_CONDITION;

extern std::function<void(std::vector<std::pair<game::Board *, double>> &, game::Board *, double)>
  SIMULATION_BOARD_SAVE_PROCEDURE;

}

namespace init {

void initializeRNG(const std::string &rng_seed = "");

void updateWorkingDirectory(const std::string &target_dir = "Chess-AI");
void
updateMCTSParameters(double thread_usage_ratio = 1.0, int simulation_move_depth = 8, int simulations_per_thread = 125);
void updateNetworkSettings(bool load_prev_network = true, bool save_networks = true,
                           const std::string &network_file_path = network::NetworkStorage::LATEST_NETWORK_FILE_PATH);
void updateTrainingParameters(const std::function<bool()> &termination_condition = [] { return true; },
                              int network_save_interval_in_games = 20, double ratio_of_training_boards_to_save = 1.0);

bool verify(bool is_fatal_if_fail = false);

}

#endif // CHESS_AI_MAIN_INITIALIZATION_H_
