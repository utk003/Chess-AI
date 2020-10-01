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
