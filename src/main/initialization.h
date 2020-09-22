#ifndef CHESS_AI_MAIN_INITIALIZATION_H_
#define CHESS_AI_MAIN_INITIALIZATION_H_

#include <string>

#include "../mcts_network/network.h"

namespace settings {

static bool PRINT_INITIALIZATION_DEBUG_INFORMATION = true;
static bool PRINT_BOARD_SIMULATION_DATA = false;

static int GAMES_PER_NETWORK_SAVE = -1;
static int NUMBER_OF_GAMES_TO_SIMULATE = -1;

static std::function<void(std::vector<std::pair<game::Board *, double>> &, game::Board *, double)>
  SIMULATION_BOARD_SAVE_PROCEDURE;

}

namespace init {

void initializeRNG(const std::string &rng_seed = "");

void updateWorkingDirectory(const std::string &target_dir = "Chess-AI");
void
updateMCTSParameters(double thread_usage_ratio = 1.0, int simulation_move_depth = 8, int simulations_per_thread = 125);
void updateNetworkSettings(bool load_prev_network = true, bool save_networks = true,
                           const std::string &network_file_path = network::NetworkStorage::LATEST_NETWORK_FILE_PATH);
void updateTrainingParameters(int network_save_interval_in_games = 1,
                              int number_of_game_plays = 100,
                              double ratio_of_training_boards_to_save = 1.0);

bool verify(bool is_fatal_if_fail = false);

}

#endif // CHESS_AI_MAIN_INITIALIZATION_H_
