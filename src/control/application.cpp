#include <chrono>
#include <memory>

#include "control/application.hpp"
#include "physics/phys_obj_view.hpp"

namespace whsim {

namespace {

using Clock = std::chrono::steady_clock;

bool KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
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

void Application::ProcessHotkeys()
{
    GLFWwindow* window = window_controller_.Window();
    const bool simulation_is_active =
        ui_controller_.GetMenuCond() == MenuCond::SIMULATION;

    const bool space_pressed = KeyPressed(window, GLFW_KEY_SPACE);
    if (space_pressed && !space_pressed_ && simulation_is_active) {
        sim_controller_.ToggleStopFlag();
    }
    space_pressed_ = space_pressed;

    const bool f11_pressed = KeyPressed(window, GLFW_KEY_F11);
    if (f11_pressed && !f11_pressed_ && simulation_is_active) {
        ui_controller_.ToggleSimulationFullscreen();
    }
    f11_pressed_ = f11_pressed;

    const bool escape_pressed = KeyPressed(window, GLFW_KEY_ESCAPE);
    if (escape_pressed && !escape_pressed_) {
        if (ui_controller_.IsSimulationFullscreen()) {
            ui_controller_.SetSimulationFullscreen(false);
        } else if (ui_controller_.GetMenuCond() != MenuCond::MAIN) {
            ui_controller_.SetMenuCond(MenuCond::MAIN);
        }
    }
    escape_pressed_ = escape_pressed;
}

void Application::RunLoop()
{
    auto previous_time = Clock::now();
    
    while (!window_controller_.ShouldClose()) {

        glfwPollEvents();
        window_controller_.ProcessInput();
        ProcessHotkeys();

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
