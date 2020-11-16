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

#include "train.h"

#include <string>
#include <vector>

#include "make_cases.h"
#include "../../util/string_util.h"
#include "../../util/thread_util.h"

void network::train::train_network(const std::vector<std::string> &training_case_files) {
  train_network(training_case_files, network::NetworkStorage::current_network(),
                settings::TRAINING_TERMINATION_CONDITION);
}

void network::train::train_network(const std::vector<std::string> &training_case_files, Network *net,
                                   const std::function<bool()> &termination_condition) {
  const int NUM_CASES_PER_SAVE = settings::GAMES_PER_NETWORK_SAVE;
  const double LAMBDA_MIN = 0.0001, LAMBDA_MAX = 10.0, DEFAULT_LAMBDA = Optimizer::DEFAULT_INITIAL_LAMBDA;

  double lambda = DEFAULT_LAMBDA;
  int case_counter = 0;

  while (!termination_condition())
    if (!training_case_files.empty()) {
      const std::string &file_path = training_case_files[math::random((int) training_case_files.size())];
      auto training_case = load_training_case(file_path, !string::endsWith(file_path, ".txt"));
      Optimizer::optimize(net, training_case.first, training_case.second, lambda);
      delete training_case.first;

      if (lambda < LAMBDA_MIN || LAMBDA_MAX < lambda) {
        lambda = DEFAULT_LAMBDA;
        Optimizer::dropout(net);
      }
      if (++case_counter >= NUM_CASES_PER_SAVE) {
        if (save_network) {
          save_network.store(false);
          network::NetworkStorage::saveNetwork();
        }

        case_counter = 0;

      }
    } else
      thread::sleep_millis(1);
}