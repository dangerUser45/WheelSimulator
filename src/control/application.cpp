#include <chrono>
#include <memory>

#include "control/application.hpp"
#include "physics/phys_obj_view.hpp"

namespace whsim {

namespace {

using Clock = std::chrono::steady_clock;

float DeltaTime(auto& previous_time)
{
    const auto current_time = Clock::now();
    std::chrono::duration<float> frame_time = current_time - previous_time;
    previous_time = current_time;

    return frame_time.count();
}

}

Application::Application() :
    physics_(std::make_unique<Physics>()),
    phys_obj_view_(std::make_unique<PhysObjView>(*physics_))
{}

void Application::ResetSimulation()
{
    physics_ = std::make_unique<Physics>();
    phys_obj_view_ = std::make_unique<PhysObjView>(*physics_);
}

void Application::RunLoop()
{
    auto previous_time = Clock::now();
    
    while (!window_controller_.ShouldClose()) {

        glfwPollEvents();
        window_controller_.ProcessInput();

        auto dt = DeltaTime(previous_time);
        if (sim_controller_.ShouldStepSimulation(ui_controller_.GetMenuCond()))
            physics_->Step(dt);

        phys_obj_view_->Update();
        auto [sim_width, sim_height] =
            SimulationTextureSize(window_controller_.Window());
        const bool camera_input_enabled =
            ui_controller_.GetMenuCond() == MenuCond::SIMULATION;

        const GLuint sim_texture = sim_render_.Render(
            window_controller_.Window(),
            phys_obj_view_->PhysObjects(),
            sim_width,
            sim_height,
            dt,
            camera_input_enabled
        );
        
        ui_render_.Render(
            window_controller_.Window(),
            ui_controller_,
            sim_controller_,
            sim_texture
        );

        if (sim_controller_.RequestReset()) {
            this->ResetSimulation();
        }
    }
}

} // namespace whsim
