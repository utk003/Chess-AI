cmake_minimum_required(VERSION 3.16)    # CMake version check
project(Chess_AI)                       # Project name is "Chess_AI"
set(CMAKE_CXX_STANDARD 17)              # Using c++17

# main.cpp file
set(MAIN_FILE src/main.cpp)

# src/chess directory
set(SRC_CHESS_PIECE src/chess/piece.fwd.h src/chess/piece.h src/chess/piece.cpp)
set(SRC_CHESS_GAME src/chess/game.fwd.h src/chess/game.h src/chess/game.cpp)
set(SRC_CHESS_DIR ${SRC_CHESS_PIECE} ${SRC_CHESS_GAME})

# src/graphics directory
set(SRC_GRAPHICS_OPENGL src/graphics/opengl.h src/graphics/opengl.cpp)
set(SRC_GRAPHICS_SHADER src/graphics/shader.h src/graphics/shader.cpp)
set(SRC_GRAPHICS_DIR ${SRC_GRAPHICS_OPENGL} ${SRC_GRAPHICS_SHADER})

# src/mcts_network directory
set(SRC_MCTS_NETWORK_DECIDER src/mcts_network/decider.fwd.h src/mcts_network/decider.h src/mcts_network/decider.cpp)
set(SRC_MCTS_NETWORK_NETWORK src/mcts_network/network.fwd.h src/mcts_network/network.h src/mcts_network/network.cpp)
set(SRC_MCTS_NETWORK_TREE src/mcts_network/tree.fwd.h src/mcts_network/tree.h src/mcts_network/tree.cpp)
set(SRC_MCTS_NETWORK_DIR ${SRC_MCTS_NETWORK_DECIDER} ${SRC_MCTS_NETWORK_NETWORK} ${SRC_MCTS_NETWORK_TREE})

# src/player directory
set(SRC_PLAYER_PLAYER src/player/player.fwd.h src/player/player.h  src/player/player.cpp)
set(SRC_PLAYER_DIR ${SRC_PLAYER_PLAYER})

# src/util directory
set(SRC_UTIL_MATH_UTIL src/util/math_util.h src/util/math_util.cpp)
set(SRC_UTIL_STRING_UTIL src/util/string_util.h src/util/string_util.cpp)
set(SRC_UTIL_THREAD_UTIL src/util/string_util.h src/util/thread_util.cpp)
set(SRC_UTIL_ASSERT_UTIL src/util/assert_util.h)

set(SRC_UTIL_DIR ${SRC_UTIL_MATH_UTIL} ${SRC_UTIL_STRING_UTIL} ${SRC_UTIL_THREAD_UTIL} ${SRC_UTIL_ASSERT_UTIL})

# src directory
set(SRC_DIR ${MAIN_FILE} ${SRC_CHESS_DIR} ${SRC_GRAPHICS_DIR} ${SRC_MCTS_NETWORK_DIR} ${SRC_PLAYER_DIR} ${SRC_UTIL_DIR})

# external library linking
include_directories(libraries/include)
link_directories(libraries)

# executable
set(SOURCE_FILES ${SRC_DIR})
add_executable(Chess_AI ${SOURCE_FILES})

# external libraries
target_link_libraries(Chess_AI "glfw3 -framework Cocoa -framework OpenGL -framework IOKit")
target_link_libraries(Chess_AI glew)
target_link_libraries(Chess_AI gsl)
target_link_libraries(Chess_AI gslcblas)

# Debug/Release Macros
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Chess_AI PUBLIC DEBUG=1)
else ()
    target_compile_definitions(Chess_AI PUBLIC DEBUG=0)
endif ()