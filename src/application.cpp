#include "application.hpp"

namespace whsim {

void Application::RunLoop()
{
    while (!glfwWindowShouldClose(view_.window_)) {
        glfwPollEvents();
        view_.ProcessInput();

        view_.RenderUI();
    }
}

} // namespace whsim
