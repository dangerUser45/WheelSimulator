#pragma once

#include <utility>

#ifndef GLFW_INCLUDED
#include <GLFW/glfw3.h>
#define GLFW_INCLUDED
#endif

#include "control/sim_controller.hpp"
#include "control/ui_controller.hpp"
#include "config/simulation_settings.hpp"
#include "render/graphics.hpp"

namespace whsim {

[[nodiscard]] std::pair<int, int> SimulationTextureSize(
    GLFWwindow* window,
    const UIController& ui_ctrl);

class UIRender final {
private:
    void DrawUI(
        UIController& ui_ctrl,
        SimController& sim_ctrl,
        SimulationSettings& settings,
        const Graphics& graphics,
        GLuint sim_texture) const;

public:
    UIRender(GLFWwindow* window);
    ~UIRender();

    UIRender(const UIRender&) = delete;
    UIRender& operator=(const UIRender&) = delete;
    UIRender(UIRender&&) = delete;
    UIRender& operator=(UIRender&&) = delete;

    void Render(
        GLFWwindow* window,
        UIController& ui_ctrl,
        SimController& sim_ctrl,
        SimulationSettings& settings,
        const Graphics& graphics,
        GLuint sim_texture) const;
};

} // namespace whsim
