#ifndef CHESS_AI_MAIN_NETWORK_MAKE_CASES_H_
#define CHESS_AI_MAIN_NETWORK_MAKE_CASES_H_

#include <vector>
#include <utility>
#include <string>

#include "../../chess/game.fwd.h"

namespace network {

static std::vector<std::pair<game::Board *, double>> training_boards;

bool generate_training_cases(const std::function<bool()> &termination_condition = [] { return true; });

void save_training_case(const std::string &file_path, const std::pair<game::Board *, double> &training_case);
std::pair<game::Board *, double> load_training_case(const std::string &file_path);

}

#endif // CHESS_AI_MAIN_NETWORK_MAKE_CASES_H_
