cmake_minimum_required(VERSION 3.16)    # CMake version check
project(Chess_AI)                       # Project name is "Chess_AI"
set(CMAKE_CXX_STANDARD 17)              # Using c++17

# main.cpp file
set(MAIN_FILE src/main.cpp)

# src directory
set(SRC_DIR ${MAIN_FILE})

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
set(SRC_PLAYER_PLAYER src/player/player.fwd.h src/player/player.h src/player/player.cpp)
set(SRC_PLAYER_DIR ${SRC_PLAYER_PLAYER})

# src/util directory
set(SRC_UTIL_MATH_UTIL src/util/math_util.h src/util/math_util.cpp)
set(SRC_UTIL_STRING_UTIL src/util/string_util.h src/util/string_util.cpp)
set(SRC_UTIL_THREAD_UTIL src/util/string_util.h src/util/thread_util.cpp)
set(SRC_UTIL_ASSERT_UTIL src/util/assert_util.h src/util/assert_util.cpp)

set(SRC_UTIL_DIR ${SRC_UTIL_MATH_UTIL} ${SRC_UTIL_STRING_UTIL} ${SRC_UTIL_THREAD_UTIL} ${SRC_UTIL_ASSERT_UTIL})

# get all program dependencies
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(GSL REQUIRED gsl)
pkg_search_module(GLM REQUIRED glm)

# get library binaries/get headers for dependencies
include_directories(libraries/include ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
link_directories(libraries ${GLFW_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS} ${GSL_LIBRARY_DIRS} ${GLM_LIBRARY_DIRS})

# create executable
set(SOURCE_FILES ${SRC_DIR} ${SRC_CHESS_DIR} ${SRC_GRAPHICS_DIR} ${SRC_MCTS_NETWORK_DIR} ${SRC_PLAYER_DIR} ${SRC_UTIL_DIR})
add_executable(Chess_AI ${SOURCE_FILES})

# link headers for dependencies
target_link_libraries(Chess_AI ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${GSL_LIBRARIES})

# system-specific settings
if (APPLE)
    target_link_libraries(Chess_AI "-framework Cocoa -framework OpenGL -framework IOKit") # needed for static glfw library binary on OSX
endif ()

# required linker flags in makefile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -rdynamic") # needed for multithreading/stack tracing

# Debug/Release Macros
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Chess_AI PUBLIC DEBUG)
endif ()