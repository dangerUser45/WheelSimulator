#include <chrono>
#include <algorithm>
#include <memory>

#include "control/application.hpp"
#include "control/sim_controller.hpp"
#include "control/ui_controller.hpp"
#include "physics/phys_obj_view.hpp"

namespace whsim {

namespace {

using Clock = std::chrono::steady_clock;

bool KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool CtrlPressed(GLFWwindow* window)
{
    return
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
}

float DeltaTime(auto& previous_time)
{
    const auto current_time = Clock::now();
    std::chrono::duration<float> frame_time = current_time - previous_time;
    previous_time = current_time;

    return frame_time.count();
}

}

Application::Application() :
    physics_(std::make_unique<Physics>(settings_)),
    phys_obj_view_(std::make_unique<PhysObjView>(*physics_))
{}

void Application::ResetSimulation()
{
    physics_ = std::make_unique<Physics>(settings_);
    phys_obj_view_ = std::make_unique<PhysObjView>(*physics_);
    graphics_.Clear();
}

void Application::ProcessMenuSection(GLFWwindow* const window,
    const MenuCond menu_cond, bool ctrl_pressed, auto glfw_key)
{
    const bool menu_shortcut_pressed = ctrl_pressed && KeyPressed(window, glfw_key);
    if(menu_shortcut_pressed && ui_controller_.GetMenuCond() != menu_cond)
        ui_controller_.SetMenuCond(menu_cond);
}

void Application::ProccessMenuSwitching(GLFWwindow* const window, bool ctrl_pressed)
{
    this->ProcessMenuSection(window, MenuCond::MAIN,       ctrl_pressed, GLFW_KEY_1);
    this->ProcessMenuSection(window, MenuCond::SETTINGS,   ctrl_pressed, GLFW_KEY_2);
    this->ProcessMenuSection(window, MenuCond::SIMULATION, ctrl_pressed, GLFW_KEY_3);
    this->ProcessMenuSection(window, MenuCond::GRAPHICS,   ctrl_pressed, GLFW_KEY_4);
}

void Application::ProccessSimReset(GLFWwindow* const window, bool ctrl_pressed, bool sim_is_active)
{
    const bool reset_simulation_shortcut_pressed =
        ctrl_pressed && KeyPressed(window, GLFW_KEY_ENTER);
    
        if (reset_simulation_shortcut_pressed &&
        !reset_simulation_shortcut_pressed_ &&
        sim_is_active) {
        sim_controller_.SetStopFlag(true);
        sim_controller_.SetResetFlag(true);
    }
    reset_simulation_shortcut_pressed_ = reset_simulation_shortcut_pressed;
}

void Application::ProccessSimPause(GLFWwindow* const window, bool sim_is_active)
{
    const bool pause_toggle_shortcut_pressed = KeyPressed(window, GLFW_KEY_SPACE);
    if (pause_toggle_shortcut_pressed &&
        !pause_toggle_shortcut_pressed_ &&
        sim_is_active
    ) 
        sim_controller_.ToggleStopFlag();
    pause_toggle_shortcut_pressed_ = pause_toggle_shortcut_pressed;
}

void Application::ProccessSimFullScreen(GLFWwindow* const window, bool sim_is_active)
{
    const bool fullscreen_toggle_shortcut_pressed = KeyPressed(window, GLFW_KEY_F3);
    if (fullscreen_toggle_shortcut_pressed &&
        !fullscreen_toggle_shortcut_pressed_ &&
        sim_is_active
    )
        ui_controller_.ToggleSimulationFullscreen();
    fullscreen_toggle_shortcut_pressed_ = fullscreen_toggle_shortcut_pressed;
}

void Application::ProccessSimulationStates(GLFWwindow* const window, bool ctrl_pressed)
{
    const bool sim_is_active = ui_controller_.GetMenuCond() == MenuCond::SIMULATION;

    this->ProccessSimReset(window, ctrl_pressed, sim_is_active);
    this->ProccessSimPause(window, sim_is_active);
    this->ProccessSimFullScreen(window, sim_is_active);
}

void Application::ProccessBackNavigation(GLFWwindow* const window)
{
    const bool back_navigation_shortcut_pressed = KeyPressed(window, GLFW_KEY_ESCAPE);
    if (back_navigation_shortcut_pressed && !back_navigation_shortcut_pressed_) {
        if (ui_controller_.IsSimulationFullscreen()) {
            ui_controller_.SetSimulationFullscreen(false);
        } else if (ui_controller_.GetMenuCond() != MenuCond::MAIN) {
            ui_controller_.SetMenuCond(MenuCond::MAIN);
        }
    }
    back_navigation_shortcut_pressed_ = back_navigation_shortcut_pressed;
}

void Application::ProcessHotkeys()
{
    GLFWwindow* window = window_controller_.Window();
    const bool ctrl_pressed = CtrlPressed(window);

    this->ProccessMenuSwitching(window, ctrl_pressed);
    this->ProccessSimulationStates(window, ctrl_pressed);
    this->ProccessBackNavigation(window);
}

void Application::ProcessCameraSpeedScroll()
{
    if (ui_controller_.GetMenuCond() != MenuCond::SIMULATION ||
        settings_.camera.mode != CameraMode::Free) {
        return;
    }

    const auto wheel_delta =
        static_cast<float>(window_controller_.ConsumeScrollYOffset());
    if (wheel_delta == 0.0f) {
        return;
    }

    settings_.camera.free_move_speed = std::clamp(
        settings_.camera.free_move_speed + wheel_delta * 1.5f,
        0.5f,
        50.0f);
}

void Application::RunLoop()
{
    auto previous_time = Clock::now();
    
    while (!window_controller_.ShouldClose()) {

        glfwPollEvents();
        window_controller_.ProcessInput();
        ProcessHotkeys();
        ProcessCameraSpeedScroll();

        auto dt = DeltaTime(previous_time);
        if (sim_controller_.ShouldStepSimulation(ui_controller_.GetMenuCond())) {
            physics_->Step(dt);
            graphics_.Record(dt, physics_->Telemetry());
        }

        phys_obj_view_->Update();
        auto [sim_width, sim_height] =
            SimulationTextureSize(window_controller_.Window(), ui_controller_);
        const bool camera_input_enabled =
            ui_controller_.GetMenuCond() == MenuCond::SIMULATION;

        const GLuint sim_texture = sim_render_.Render(
            window_controller_.Window(),
            phys_obj_view_->PhysObjects(),
            phys_obj_view_->TerrainGridData(),
            sim_width,
            sim_height,
            dt,
            camera_input_enabled,
            settings_
        );

        settings_.camera.mode = sim_render_.CameraViewMode();
        const SimulationSettings previous_settings = settings_;
        
        ui_render_.Render(
            window_controller_.Window(),
            ui_controller_,
            sim_controller_,
            settings_,
            graphics_,
            sim_texture
        );

        if (PhysicsResetSettingsChanged(previous_settings, settings_)) {
            sim_controller_.SetStopFlag(true);
            this->ResetSimulation();
        } else {
            physics_->ApplySettings(settings_);
        }

        if (sim_controller_.RequestReset()) {
            this->ResetSimulation();
        }
    }
}

} // namespace whsim
