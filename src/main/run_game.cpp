// ------------------------------------------------------------------------------ //
// MIT License                                                                    //
//                                                                                //
// Copyright (c) 2020 Utkarsh Priyam                                              //
//                                                                                //
// Permission is hereby granted, free of charge, to any person obtaining a copy   //
// of this software and associated documentation files (the "Software"), to deal  //
// in the Software without restriction, including without limitation the rights   //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      //
// copies of the Software, and to permit persons to whom the Software is          //
// furnished to do so, subject to the following conditions:                       //
//                                                                                //
// The above copyright notice and this permission notice shall be included in all //
// copies or substantial portions of the Software.                                //
//                                                                                //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  //
// SOFTWARE.                                                                      //
// ------------------------------------------------------------------------------ //

#include "run_game.h"

#include "initialization.h"

#include "../graphics/opengl.h"
#include "../util/thread_util.h"

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