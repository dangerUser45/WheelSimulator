#pragma once

#include <GLFW/glfw3.h>

#include "physics.hpp" 

namespace whsim {

enum class MenuCond {MAIN, SETTINGS, SIMULATION, GRAPHICS};
enum class StyleUI {LIGHT, DARK};

struct PreviewImage {
    GLuint texture;
    int width, heigth;
};

class View final {
private:
    
    PreviewImage prev_img_{};
    GLFWwindow* window_ = nullptr;

    MenuCond menu_cond_{MenuCond::MAIN};
    StyleUI style_ui_{StyleUI::DARK};
    
    void InitWindow();
    void InitImGui();
    void DestroyImGui();

    void DrawUI(Physics& physics);

public:
    View();
    ~View();

    View(const View&) = delete;
    View& operator=(const View&) = delete;
    View(View&&) = delete;
    View& operator=(View&&) = delete;

    [[nodiscard]] bool ShouldClose() const;
    void ProcessInput() const;
    void RenderUI(Physics& physics);
};

} // namespace whsim
