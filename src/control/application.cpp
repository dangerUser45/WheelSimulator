#include <chrono>

#include "control/application.hpp"

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

void Application::RunLoop()
{
    auto previous_time = Clock::now();
    
    while (!window_controller_.ShouldClose()) {

        glfwPollEvents();
        window_controller_.ProcessInput();

        auto dt = DeltaTime(previous_time);
        if (sim_controller_.ShouldStepSimulation())
            physics_.Step(dt);
        
        sim_render_.Render();
        ui_render_.Render(window_controller_.Window(), ui_controller_, sim_controller_);

        if (sim_controller_.RequestReset()) {
            physics_.ResetSimulation();
        }
    }
}

} // namespace whsim
