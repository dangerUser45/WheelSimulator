#pragma once

#include <GLFW/glfw3.h>

#include "view_panels.hpp"
#include "visual_style.hpp"

namespace whsim {

class View final {
private:
    GLFWwindow* window_ = nullptr;
    vstyle::glColor cur_bkgnd_color = vstyle::DarkBkgnd;

    GLuint preview_texture_ = 0;
    int preview_texture_w_ = 0;
    int preview_texture_h_ = 0;
    bool show_settings_ = false;
    ui::SettingsState settings_{};

    void DrawBackgroundImage();
    void InitWindow();
    void InitImGui();
    void DestroyImGui();

public:
    View();
    ~View();

    View(const View&) = delete;
    View& operator=(const View&) = delete;
    View(View&&) = delete;
    View& operator=(View&&) = delete;

    [[nodiscard]] bool ShouldClose() const;
    void ProcessInput() const;
    void RenderUI();
};

} // namespace whsim
