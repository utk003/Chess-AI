#include "run_game.h"

#include "initialization.h"

#include "../graphics/opengl.h"
#include "../util/thread_util.h"

bool game::play_game(player::PlayerType white, player::PlayerType black, const std::string &default_board_file_path) {
  if (!init::verify())
    return false;

  run_game(white, black, true, default_board_file_path);

  return true;
}

game::GameResult game::run_game(player::PlayerType white, player::PlayerType black, bool run_graphics,
                                const std::string &default_board_file_path) {
  auto *game = new game::Game(new game::Board(8, 8));
  game->board()->loadFromFile(default_board_file_path);

  game->setPlayer(piece::PieceColor::WHITE, white);
  game->setPlayer(piece::PieceColor::BLACK, black);

  if (run_graphics) {
    // start graphics engine
    graphics::OpenGL *openGL = graphics::OpenGL::get_instance(game, "Chess");
    // start game
    game->startGame();
    // run graphics
    openGL->run();
    // once graphics closes, delete graphics engine
    delete openGL;
  } else {
    game->startGame();
    thread::wait_for([&] { return game->isOver(); });
  }
  game::GameResult result = game->getResult();

  // tell game to terminate
  game->endGame();
  // wait for game to finish processes, then delete game
  game->waitForDelete();
  delete game;

  return result;
}