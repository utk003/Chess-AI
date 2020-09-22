#include "train.h"

#include <utility>
#include <string>
#include <vector>

#include "make_cases.h"
#include "../initialization.h"

bool network::train::train_network(Network *net, const std::vector<std::string> &training_case_files,
                                   const std::function<bool()> &termination_condition) {
  if (!init::verify())
    return false;

  double lambda = Optimizer::DEFAULT_INITIAL_LAMBDA;
  int cases_per_cycle = 20; // TODO make configurable --> # of cases to train on b/w successive dropouts
  while (!termination_condition()) {
    for (int i = 0; i < cases_per_cycle; ++i) {
      auto training_case = load_training_case(training_case_files[math::random((int) training_case_files.size())]);
      Optimizer::optimize(net, training_case.first, training_case.second, lambda);
      delete training_case.first;
    }
    Optimizer::dropout(net);
  }

  return true;
}