#ifndef CHESS_AI_MAIN_NETWORK_TRAIN_H_
#define CHESS_AI_MAIN_NETWORK_TRAIN_H_

#include <vector>
#include <string>
#include <functional>

#include "../initialization.h"
#include "../../mcts_network/network.fwd.h"

namespace network::train {

void train_network(const std::vector<std::string> &training_case_files = {});
void train_network(const std::vector<std::string> &training_case_files, Network *net,
                   const std::function<bool()> &termination_condition);

}

#endif // CHESS_AI_MAIN_NETWORK_TRAIN_H_
