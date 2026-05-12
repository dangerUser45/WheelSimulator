#pragma once

#include <utility>

#ifndef GLFW_INCLUDED
#include <GLFW/glfw3.h>
#define GLFW_INCLUDED
#endif

#include "control/sim_controller.hpp"
#include "control/ui_controller.hpp"

namespace whsim {

[[nodiscard]] std::pair<int, int> SimulationTextureSize(GLFWwindow* window);

class UIRender final {
private:
    void DrawUI(UIController& ui_ctrl,
                SimController& sim_ctrl,
                GLuint sim_texture) const;

public:
    UIRender(GLFWwindow* window);
    ~UIRender();

    UIRender(const UIRender&) = delete;
    UIRender& operator=(const UIRender&) = delete;
    UIRender(UIRender&&) = delete;
    UIRender& operator=(UIRender&&) = delete;

    void Render(GLFWwindow* window, UIController& ui_ctrl,
                SimController& sim_ctrl, GLuint sim_texture) const;
};

} // namespace whsim
