cmake_minimum_required(VERSION 3.16)    # CMake version check
project(Chess_AI)                       # Project name is "Chess_AI"
set(CMAKE_CXX_STANDARD 17)              # Using c++17

# src directory
set(MAIN_DIR src/main.cpp src/main/initialization.cpp src/main/run_game.cpp src/main/network/make_cases.cpp src/main/network/train.cpp)
set(GAME_DIR src/chess/piece.cpp src/chess/game.cpp)
set(GRAPHICS_DIR src/graphics/opengl.cpp src/graphics/shader.cpp)
set(MCTS_NETWORK_DIR src/mcts_network/decider.cpp src/mcts_network/network.cpp src/mcts_network/tree.cpp)
set(PLAYERS_DIR src/player/player.cpp)
set(UTIL_DIR src/util/math_util.cpp src/util/string_util.cpp src/util/thread_util.cpp src/util/assert_util.cpp)

# get all program dependencies
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(GSL REQUIRED gsl)
pkg_search_module(GLM glm)

if (NOT GLM_FOUND EQUAL 1)
    if (APPLE)
        set(GLM_PATH /usr/local/include)
    else ()
        set(GLM_PATH /usr/include)
    endif ()
    find_path(GLM_PATH glm/glm.h)
    set(GLM_INCLUDE_DIRS ${GLM_PATH})
endif ()

# get library binaries/get headers for dependencies
include_directories(libraries/include ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
link_directories(libraries ${GLFW_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS} ${GSL_LIBRARY_DIRS})

# create executable
add_executable(Chess_AI ${MAIN_DIR} ${GAME_DIR} ${GRAPHICS_DIR} ${MCTS_NETWORK_DIR} ${PLAYERS_DIR} ${UTIL_DIR})

# link headers for dependencies
target_link_libraries(Chess_AI ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${GSL_LIBRARIES})

# system-specific settings
if (APPLE)
    target_link_libraries(Chess_AI "-framework Cocoa -framework OpenGL -framework IOKit") # needed for static glfw library binary on OSX
elseif (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -rdynamic") # needed for multithreading/stack tracing
endif ()

# Debug/Release Macros
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Chess_AI PUBLIC DEBUG)
endif ()