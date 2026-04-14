#include "application.hpp"

namespace whsim {

void Application::RunLoop()
{
    while (!view_.ShouldClose()) {
        glfwPollEvents();
        view_.ProcessInput();

        view_.RenderUI();
    }
}

} // namespace whsim
