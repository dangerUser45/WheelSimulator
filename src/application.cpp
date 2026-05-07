#include <chrono>

#include "application.hpp"

namespace whsim {

void Application::RunLoop()
{
    using Clock = std::chrono::steady_clock;
    auto previous_time = Clock::now();

    while (!view_.ShouldClose()) {
        const auto current_time = Clock::now();
        std::chrono::duration<float> frame_time = current_time - previous_time;
        previous_time = current_time;

        glfwPollEvents();
        view_.ProcessInput();

        physic_.Step(frame_time.count());

        view_.RenderUI(physic_);
    }
}

} // namespace whsim
