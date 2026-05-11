#pragma once

#include "control/sim_controller.hpp"
#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

#include <imgui.h>

#include "control/ui_controller.hpp"

namespace whsim::UIImpl {

void ConfigureImGui();
void ConfigureImGuiFont();
void DrawPreviewImage(const PreviewImage& preview_image);
void DrawMenu(UIController& ui_ctrl, SimController& sim_ctrl);
void DrawSimulation(GLuint simulation_texture);
void DrawSettings();
void DrawGraphics();

} // namespace whsim::UIImpl
