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
  const int NUM_CASES_PER_DROPOUT = settings::GAMES_PER_NETWORK_SAVE;
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
        case_counter = NUM_CASES_PER_DROPOUT;
      }
      if (++case_counter >= NUM_CASES_PER_DROPOUT) {
        if (save_network) {
          save_network.store(false);
          network::NetworkStorage::saveNetwork();
        }

        case_counter = 0;
        Optimizer::dropout(net);
      }
    } else
      thread::sleep_millis(1);
}