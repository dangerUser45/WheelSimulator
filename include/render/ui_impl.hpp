#pragma once

#include "control/sim_controller.hpp"
#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

#include <imgui.h>

#include "config/simulation_settings.hpp"
#include "control/ui_controller.hpp"
#include "render/graphics.hpp"

namespace whsim::UIImpl {

void ConfigureImGui();
void ConfigureImGuiFont();
void DrawPreviewImage(const PreviewImage& preview_image);
void DrawMenu(UIController& ui_ctrl, SimController& sim_ctrl);
void DrawSimulation(GLuint simulation_texture);
void DrawSimulationFullscreen(GLuint simulation_texture);
void DrawSettings(SimulationSettings& settings);
void DrawGraphics(const Graphics& graphics);

} // namespace whsim::UIImpl
