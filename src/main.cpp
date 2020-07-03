#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "chess/piece.fwd.h"
#include "chess/game.h"
#include "graphics/opengl.h"

void startGame(game::Game* game) { game -> startGame(); }

int main2() {
  game::Game* game = new game::Game(new game::Board(8, 8));
  game -> board() -> loadFromFile("../assets/game_states/chess_default_start.txt");

  game -> setPlayer(piece::PieceColor::WHITE, player::PlayerType::HUMAN);
  game -> setPlayer(piece::PieceColor::BLACK, player::PlayerType::RANDOM);

  // game -> board() -> loadFromFile("../game_state/save 0.txt");

  game -> startGame();

  graphics::OpenGL::run_graphics(game, "Chess");

  delete game;

  return 0;
}

int main() {
  game::Game* game = new game::Game(new game::Board(8, 8));
  game -> board() -> loadFromFile("../assets/game_states/chess_default_start.txt");

  game::Game* clone;

  for (int i = 0; i < 10000; ++i) {
    clone = game -> clone();
    delete clone;
  }
}