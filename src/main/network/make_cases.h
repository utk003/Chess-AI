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

#ifndef CHESS_AI_MAIN_NETWORK_MAKE_CASES_H_
#define CHESS_AI_MAIN_NETWORK_MAKE_CASES_H_

#include <vector>
#include <utility>
#include <string>
#include <atomic>

#include "../initialization.h"
#include "../../chess/game.fwd.h"

namespace network {

static std::vector<std::pair<game::Board *, double>> training_boards;
extern std::atomic_bool save_network;

void generate_training_cases(std::vector<std::string> &file_paths, int &sim_count, int num_simulations = 1);
void generate_training_cases(const std::function<bool()> &termination_condition,
                             std::vector<std::string> &file_paths,
                             int &sim_count, int num_simulations = 1);

std::string save_training_case(const std::string &case_index, const std::pair<game::Board *, double> &training_case);
std::pair<game::Board *, double> load_training_case(const std::string &file_path, bool pad_file_path = true);

}

#endif // CHESS_AI_MAIN_NETWORK_MAKE_CASES_H_
