#pragma once

#include <glad/gl.h>

namespace whsim::stb {
GLuint LoadTexture(const char* path, int& out_w, int& out_h);
}
