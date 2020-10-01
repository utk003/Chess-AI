# Chess-AI
Fun with Chess and ANNs


TODO Finish README (later)


To build the project (at least, in its current state), you need [CMake](https://cmake.org/).

See logs.txt for a push-to-push change/update log, my thoughts on the project, and the issues I encountered/how I fixed them.


NOTE:
This code compiles correctly with both [Clang](https://clang.llvm.org/) and [GCC](https://gcc.gnu.org/) (as far as I can tell/tested). In order to get/compile/run the program for yourself, you need a few external libraries/header files: [GLFW](https://www.glfw.org/), [GLEW](http://glew.sourceforge.net/), [GSL](https://www.gnu.org/software/gsl/), [GLM](https://glm.g-truc.net/0.9.9/index.html), [stb](https://github.com/nothings/stb)

# Instructions for use
Clone repository
Configure parameters in main.cpp
Build/Run program

## Clone respository
This is self-explanatory

## Configure parameters in main.cpp
Most of the configurable options are in the first method initialize(...)

The variable "end_count" tells how many games to simulate before terminating program.
All other settings are detailed by names/comments, but can be left to default values.

## Build/Run Program
Build the program as you normally would (from the right directory)

Run the program from the parent directory of the "src" folder.
If the directory name is not "Chess-AI", provide the directory name or absolute directory path as an argument to the program while running it.