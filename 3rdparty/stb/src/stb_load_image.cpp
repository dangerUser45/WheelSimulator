#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace whsim::stb {

GLuint LoadTexture(const char* path, int& out_w, int& out_h)
{
    int channels = 0;
    unsigned char* data = stbi_load(path, &out_w, &out_h, &channels, 4);
    if (!data) {
        return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D,          // texture type
        0,                      // mitmap level
        GL_RGBA,                // internal format of saving data in OpenGL
        out_w,                  // weight
        out_h,                  // height
        0,                      // border
        GL_RGBA,                // format input data
        GL_UNSIGNED_BYTE,       // type each component
        data                    // pointer to pixels array
    );

    stbi_image_free(data);
    return texture;
}

} // namespace whsim::stb
