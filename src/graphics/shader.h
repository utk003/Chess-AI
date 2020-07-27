#ifndef CHESS_AI_GRAPHICS_UTIL_SHADER_H_
#define CHESS_AI_GRAPHICS_UTIL_SHADER_H_

#include <GL/glew.h>

namespace graphics {

GLuint loadShaders(const char *vertex_file_path, const char *fragment_file_path);

}

#endif // CHESS_AI_GRAPHICS_UTIL_SHADER_H_