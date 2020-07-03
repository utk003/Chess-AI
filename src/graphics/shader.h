#ifndef CHESSAI_GRAPHICS_UTIL_SHADER_H_
#define CHESSAI_GRAPHICS_UTIL_SHADER_H_

#include <GL/glew.h>

namespace graphics {

GLuint loadShaders(const char * vertex_file_path,const char * fragment_file_path);

}

#endif // CHESSAI_GRAPHICS_UTIL_SHADER_H_