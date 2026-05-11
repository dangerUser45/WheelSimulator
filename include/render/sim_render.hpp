#pragma once

#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

namespace whsim {

class SimRender final {
public:
    void Render() const;
    void DrawRigidBody();

    GLuint SimTexture() const noexcept;
};

} // namespace whsim