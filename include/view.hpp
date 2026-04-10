#pragma once

#include <GLFW/glfw3.h>

#include "visual_style.hpp"

namespace whsim {

class View final {
public:
    GLFWwindow* window_ = nullptr;
    vstyle::glColor cur_bkgnd_color = vstyle::DarkBkgnd;

    GLuint preview_texture_ = 0;
    int preview_texture_w_ = 0;
    int preview_texture_h_ = 0;
    bool show_menu_ = false;

    View();
    ~View();

    void InitWindow();
    void InitImGui();
    void DestroyImGui();
    void ProcessInput() const;
    void RenderUI();
    void UpdatePlotData(float dt);
    void DrawBackgroundImage();
};

} // namespace whsim