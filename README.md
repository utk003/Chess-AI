# Chess-AI
Fun with Chess and ANNs


TODO Finish README (later)


The final output is program.build in the build folder. Keep the file there because it needs the assets and shaders, which are linked using relative file paths.

See logs.txt for a push-to-push change/update log, my thoughts on the project, and the issues I encountered/how I fixed them.


NOTE:
The included make file only works for MacOS (I believe), and the code only compiles with Clang. GCC throws a bunch of errors when compiling the code. I will be looking into this soon in an attempt to make this project potentially cross-platform.

The libraries required to build this project are the following: GLFW, GLEW, GSL, [stb](https://github.com/nothings/stb)