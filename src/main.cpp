#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "graphics/opengl.h"
#include "mcts_network/tree.h"
#include "mcts_network/network.h"

void startGame(game::Game* game) { game -> startGame(); }

int runGame() {
  game::Game* game = new game::Game(new game::Board(8, 8));
  game -> board() -> loadFromFile("../assets/game_states/chess_default_start.txt");

  game -> setPlayer(piece::PieceColor::WHITE, player::PlayerType::HUMAN);
  game -> setPlayer(piece::PieceColor::BLACK, player::PlayerType::MCTS);

  // game -> board() -> loadFromFile("../game_state/save 0.txt");

  // start graphics engine
  graphics::OpenGL* openGL = graphics::OpenGL::get_instance(game, "Chess");
  // start game
  game -> startGame();
  // run graphics
  openGL -> run();

  // once graphics closes, delete graphics engine
  delete openGL;

  // tell game to terminate
  game -> endGame();
  // wait for game to finish processes, then delete game
  game -> waitForDelete();
  delete game;

  return 0;
}

int test() {
  game::Board* board = new game::Board(8, 8);
  game::Game* game = new game::Game(board);
  game::Game* clone;
  board -> loadFromFile("../assets/game_states/chess_default_start.txt");

  game -> setPlayer(piece::PieceColor::WHITE, player::PlayerType::MCTS);
  game -> setPlayer(piece::PieceColor::BLACK, player::PlayerType::HUMAN);
  player::Player* player = game -> white_player();

  std::cout << "setup complete" << std::endl;
  std::cout << "running processes" << std::endl;

  player -> playNextMove();

  std::cout << "get readout now" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(15));
  std::cout << "terminating..." << std::endl;
  
  return 0;
}

int main() {
  return runGame();
}