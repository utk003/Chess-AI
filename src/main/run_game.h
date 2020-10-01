#ifndef CHESS_AI_MAIN_RUN_GAME_H_
#define CHESS_AI_MAIN_RUN_GAME_H_

#include <string>

#include "../player/player.fwd.h"
#include "../chess/game.fwd.h"

namespace game {

game::GameResult run_game(player::PlayerType white, player::PlayerType black, bool run_graphics = false,
                          const std::string &default_board_file_path = "assets/game_states/chess_default_start.txt");

}

#endif // CHESS_AI_MAIN_RUN_GAME_H_
