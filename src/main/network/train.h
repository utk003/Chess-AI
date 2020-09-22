#ifndef CHESS_AI_MAIN_NETWORK_TRAIN_H_
#define CHESS_AI_MAIN_NETWORK_TRAIN_H_

#include <vector>
#include <string>
#include <functional>

#include "../../mcts_network/network.fwd.h"

namespace network::train {

bool train_network(Network *net, const std::vector<std::string> &training_case_files,
                   const std::function<bool()> &termination_condition = [] { return true; });

}

#endif // CHESS_AI_MAIN_NETWORK_TRAIN_H_
