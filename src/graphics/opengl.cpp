#include "opengl.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#include <string>

#include "../util/string_util.h"

std::map<GLFWwindow *, graphics::OpenGL *> graphics::OpenGL::_opengl_map;

void graphics::OpenGL::run_graphics(game::Game *game, const std::string &game_name) {
  OpenGL *opengl = get_instance(game, game_name);
  opengl->run();
  delete opengl;
}

graphics::OpenGL *graphics::OpenGL::get_instance(game::Game *game, const std::string &game_name) {
  auto *instance = new OpenGL(game, game_name);
  instance->initialize();

  if (instance->_window == nullptr) {
    debug_assert();
    delete instance;
    return nullptr;
  }
  _opengl_map[instance->_window] = instance;

  return instance;
}

graphics::OpenGL::OpenGL(game::Game *game, const std::string &game_name) {
  if (game == nullptr) {
    debug_assert();
    return;
  }
  _game = game;
  _board = game->board()->clone();

  asset_file_path = ASSET_2D_DIRECTORY;

  _white = game->white_player();
  if (_white != nullptr)
    _human_white = _white->type().isHumanPlayer() ? (player::HumanPlayer *) _white: nullptr;
  else debug_assert();

  _black = game->black_player();
  if (_black != nullptr)
    _human_black = _black->type().isHumanPlayer() ? (player::HumanPlayer *) _black: nullptr;
  else debug_assert();

  _game_name = game_name;
  _window_title = _game_name + ": " + _white->type().toString() + " vs " + _black->type().toString();

  _file_save_counter = 0;
  _show_expanded_ui = false;

  int board_size = _board->length() * _board->width();
  for (int i = 0; i < board_size; ++i)
    _overlays.emplace_back(4, false);
}

graphics::OpenGL::~OpenGL() {
  _opengl_map.erase(_window);
  delete _board; // -> b/c it's a clone

  // Clean up shader
  glDeleteProgram(_shader_programID);
  glDeleteVertexArrays(1, &_vaoID);

  glfwTerminate();
}

GLuint graphics::OpenGL::getCoordBuffer(int rInt, int cInt) {
  float r = (float) rInt - 4.0f;
  float c = (float) cInt - 4.0f;

  GLfloat g_vertex_buffer_data[] = {
    2.0f * c, 2.0f * r, 0.0f,
    2.0f * c, 2.0f * (r + 1.0f), 0.0f,
    2.0f * (c + 1.0f), 2.0f * (r + 1.0f), 0.0f,
    2.0f * c, 2.0f * r, 0.0f,
    2.0f * (c + 1.0f), 2.0f * (r + 1.0f), 0.0f,
    2.0f * (c + 1.0f), 2.0f * r, 0.0f,
  };

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  return vertexbuffer;
}

GLuint graphics::OpenGL::getTextureBuffer() {
  GLfloat g_vertex_buffer_data[] = {
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
  };

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  return vertexbuffer;
}

void graphics::OpenGL::loadTexture(const std::string &fileName, std::map<std::string, GLuint> &text_map) {
  int w, h, channels;
  stbi_uc *buf = stbi_load(fileName.c_str(), &w, &h, &channels, 4);

  if (buf != nullptr) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    glGenerateMipmap(GL_TEXTURE_2D);

    text_map[fileName] = textureID;
  } else debug_assert();

  stbi_image_free(buf);
}

void graphics::OpenGL::loadTextures() {
  std::string pieceNames[6] = {"rook", "pawn", "king", "queen", "knight", "bishop"};
  std::string pieceColors[2] = {"black", "white"};
  std::string backColors[2] = {"0", "1"};

  std::string str;
  for (const std::string &backColor: backColors) {
    for (const std::string &name: pieceNames)
      for (const std::string &pieceColor: pieceColors) {
        str = string::combine({asset_file_path, "piece/", backColor, "-", pieceColor, "_", name, ".png"});
        loadTexture(str, _texture_map);
      }
    str = string::combine({asset_file_path, "piece/", backColor, "-transparent.png"});
    loadTexture(str, _texture_map);
  }

  std::string boardUI[4] = {"selected", "previous_move", "normal_move", "attacking_move"};
  for (const std::string &name: boardUI)
    for (int j = 1; j <= 4; ++j)
      for (int i = 1; i <= j; ++i) {
        str = string::combine(
          {asset_file_path, "overlay/", name, "/", std::to_string(i), "-", std::to_string(j), ".png"}
        );
        loadTexture(str, _texture_map);
      }
}

void graphics::OpenGL::mouseClicked(GLFWwindow *window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    int r = 7 - (int) (8 * y / h), c = (int) (8 * x / w);

    if (_human_white != nullptr)
      _human_white->clickSquare(r, c);
    if (_human_black != nullptr)
      _human_black->clickSquare(r, c);
  }
}

void graphics::OpenGL::keyboardPressed(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_Q:
        promotionHelper(piece::PieceType::QUEEN);
        break;

      case GLFW_KEY_R:
        promotionHelper(piece::PieceType::ROOK);
        break;

      case GLFW_KEY_K:
        promotionHelper(piece::PieceType::KNIGHT);
        break;

      case GLFW_KEY_B:
        promotionHelper(piece::PieceType::BISHOP);
        break;

      case GLFW_KEY_Z: {
        int countHumans = (_human_white != nullptr) + (_human_black != nullptr);
        if (countHumans > 0)
          _game->board()->undoMove(_game, 3 - countHumans);
        break;
      }

      case GLFW_KEY_S:
        _board->saveToFile("save " + std::to_string(_file_save_counter++));
        break;

      case GLFW_KEY_TAB:
        _show_expanded_ui = !_show_expanded_ui;
        break;

      default:
        break;
    }
  }
}

void graphics::OpenGL::initialize() {
  // Exit if game doesn't exist
  if (_game == nullptr) {
    debug_assert();
    return;
  }

  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    debug_assert();
    return;
  }

  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

  // Open a window and create its OpenGL context
  _window = glfwCreateWindow(1024, 1024, _window_title.c_str(), nullptr, nullptr);
  if (_window == nullptr) {
    std::cerr << "Failed to open GLFW window. Note: Intel GPUs are not OpenGL 3.3 compatible." << std::endl;
    glfwTerminate();
    debug_assert();
    return;
  }
  glfwMakeContextCurrent(_window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    glfwTerminate();
    debug_assert();
    return;
  }

  // Tell game which graphics engine is active
  _game->setGraphics(this);

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(_window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // OpenGL settings
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Load all textures
  loadTextures();

  // Create and compile our GLSL program from the shaders
  _shader_programID = graphics::loadShaders("shaders/vertex_shader.vs", "shaders/fragment_shader.fs");

  // Mouse button callback
  glfwSetMouseButtonCallback(_window, onMouseButtonClick);
  // Keyboard key callback
  glfwSetKeyCallback(_window, onKeyboardKeyPress);

  // Set up VAO
  glGenVertexArrays(1, &_vaoID);
  glBindVertexArray(_vaoID);

  // Get a handle for our "MVP" uniform
  _mvp_matrixID = glGetUniformLocation(_shader_programID, "MVP");
  glm::mat4 proj_matrix = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.0f, 100.0f);
  glm::mat4 view_matrix = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 model_matrix = glm::mat4(1.0f);
  _mvp_matrix = proj_matrix * view_matrix * model_matrix;

  // Get texture sampler ID
  _texture_samplerID = glGetUniformLocation(_shader_programID, "texture_sampler");
}

void graphics::OpenGL::updateGraphics(game::Board *board) {
  if (board == nullptr) {
    debug_assert();
    return;
  }

  delete _temp_board;
  _temp_board = board->clone();
}

void graphics::OpenGL::checkBoardUpdate() {
  if (_temp_board == nullptr)
    return;

  delete _board;
  _board = _temp_board;
  _temp_board = nullptr;
}

void graphics::OpenGL::renderOverlays(GLuint textbuff) {
  // selected square overlay + attacked squares overlays
  int x = _game->selected_x(), y = _game->selected_y();
  if (x != -1 && y != -1) {
    _overlays[locMap(_board, x, y)][0] = true;

    // attack squares if extra ui enabled
    if (_show_expanded_ui) {
      auto *moves = new std::vector<game::Move>();
      _board->getMovesFromSquare(x, y, moves);
      for (const game::Move &move: *moves)
        _overlays[locMap(_board, move.endingRow(), move.endingColumn())][2 + move.isAttack(_board)] = true;
      delete moves;
    }
  }

  // previous move overlay
  game::Move *pastMove = _board->getLastMove();
  if (pastMove != nullptr) {
    _overlays[locMap(_board, pastMove->startingRow(), pastMove->startingColumn())][1] = true;
    _overlays[locMap(_board, pastMove->endingRow(), pastMove->endingColumn())][1] = true;
  }
  delete pastMove;

  std::string total, filePath;
  std::string textures[4] = {"selected", "previous_move", "normal_move", "attacking_move"};
  for (int r = 0; r < _board->length(); ++r)
    for (int c = 0; c < _board->width(); ++c) {
      int index = locMap(_board, r, c);
      total = std::to_string(_overlays[index][0] + _overlays[index][1] + _overlays[index][2] + _overlays[index][3]);
      int count = 0;
      for (int i = 0; i < 4; ++i) {
        if (!_overlays[index][i])
          continue;
        _overlays[index][i] = false;

        filePath = string::combine(
          {asset_file_path, "overlay/", textures[i], "/", std::to_string(++count), "-", total, ".png"}
        );
        renderSquare(r, c, textbuff, filePath);
      }
    }
}

void graphics::OpenGL::run() {
  int r, c, len = _board->length(), wid = _board->width();
  do {
    checkBoardUpdate();

    if (_game->isOver()) {
      game::GameResult result = _game->getResult();
      if (result.isBlackWin())
        _window_title = _game_name + ": Black won";
      else if (result.isWhiteWin())
        _window_title = _game_name + ": White won";
      else if (result.isStalemate())
        _window_title = _game_name + ": Stalemate";
    }
    else {
      if (!_game->getResult().isGameUndecided())
        debug_assert();
      _window_title = _game_name + ": " + _white->type().toString() + " vs " + _black->type().toString();
    }
    // Update Window Title
    glfwSetWindowTitle(_window, _window_title.c_str());

    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our shaders
    glUseProgram(_shader_programID);

    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glUniformMatrix4fv(_mvp_matrixID, 1, GL_FALSE, &_mvp_matrix[0][0]);
    glUniform1i(_texture_samplerID, 0);


    // Enable drawing attributes/textures
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glActiveTexture(GL_TEXTURE0);


    // Create VBOs
    GLuint textbuff = getTextureBuffer();

    // Draw Images
    std::string filePath;
    // 64 squares/pieces
    for (r = 0; r < len; ++r)
      for (c = 0; c < wid; ++c) {
        int backColorIndex = (r + c) % 2;
        filePath = _board->getPiece(r, c)->image_file_path();
        filePath = string::combine({asset_file_path, "piece/", std::to_string(backColorIndex), "-", filePath});

        renderSquare(r, c, textbuff, filePath);
      }
    renderOverlays(textbuff);

    // Clean up VBOs
    glDeleteBuffers(1, &textbuff);

    // Disable drawing attributes
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Swap buffers
    glfwSwapBuffers(_window);
    glfwPollEvents();

    // ++count; // loop counter -> for debugging only

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(_window) == 0);
}

void graphics::OpenGL::renderSquare(int r, int c, GLuint textbuff, const std::string &image_file_path) {
  GLuint buffer = getCoordBuffer(r, c);

  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, textbuff);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindTexture(GL_TEXTURE_2D, _texture_map[image_file_path]);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDeleteBuffers(1, &buffer);
}