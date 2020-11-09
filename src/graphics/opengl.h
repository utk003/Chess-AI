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

#ifndef CHESS_AI_GRAPHICS_OPENGL_H_
#define CHESS_AI_GRAPHICS_OPENGL_H_

#include "opengl.fwd.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <string>

#include "shader.h"

#include "../chess/piece.fwd.h"
#include "../chess/game.h"
#include "../player/player.h"

namespace graphics {

class OpenGL : game::BoardController {
  public:
    static void run_graphics(game::Game *game, const std::string &game_name);
    static OpenGL *get_instance(game::Game *game, const std::string &game_name);

    ~OpenGL();

    void run();

    void updateGraphics(game::Board *board);

  private:
    static std::map<GLFWwindow *, OpenGL *> _opengl_map;

    inline const static std::string ASSET_2D_DIRECTORY = "assets/2D/";
    std::string asset_file_path;

    OpenGL(game::Game *g, const std::string &game_name);
    void initialize();

    static GLuint getCoordBuffer(int rInt, int cInt);
    static GLuint getTextureBuffer();

    static void loadTexture(const std::string &fileName, std::map<std::string, GLuint> &text_map);
    void loadTextures();

    void renderSquare(int r, int c, GLuint textbuff, const std::string &image_file_path);
    void renderOverlays(GLuint textbuff);

    void mouseClicked(GLFWwindow *window, int button, int action, int mods);
    inline static void onMouseButtonClick(GLFWwindow *window, int button, int action, int mods) {
      _opengl_map[window]->mouseClicked(window, button, action, mods);
    }

    void keyboardPressed(GLFWwindow *window, int key, int scancode, int action, int mods);
    inline static void onKeyboardKeyPress(GLFWwindow *window, int key, int scancode, int action, int mods) {
      _opengl_map[window]->keyboardPressed(window, key, scancode, action, mods);
    }

    inline void promotionHelper(piece::PieceType type) {
      if (_human_white != nullptr)
        _human_white->setPawnUpgradeType(type);
      if (_human_black != nullptr)
        _human_black->setPawnUpgradeType(type);
    }

    bool _show_expanded_ui;
    std::vector<std::vector<bool>> _overlays;

    GLFWwindow *_window;
    GLuint _shader_programID, _vaoID, _mvp_matrixID, _texture_samplerID;
    glm::mat4 _mvp_matrix;

    game::Game *_game;
    game::Board *_board;

    game::Board *_temp_board;
    void checkBoardUpdate();

    player::Player *_white;
    player::Player *_black;

    player::HumanPlayer *_human_white;
    player::HumanPlayer *_human_black;

    std::map<std::string, GLuint> _texture_map;
    std::string _window_title, _game_name;

    int _file_save_counter;
};

}

#endif // CHESS_AI_GRAPHICS_OPENGL_H_