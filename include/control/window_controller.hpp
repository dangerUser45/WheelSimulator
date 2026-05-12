#pragma once

#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

namespace whsim {

class WindowController final {
private:
    GLFWwindow* window_ = nullptr;

public:
    WindowController(int init_window_width, int init_window_height);
    ~WindowController();

    WindowController(WindowController&&) = delete;
    WindowController(const WindowController&) = delete;
    WindowController& operator=(WindowController&&) = delete;
    WindowController& operator=(const WindowController&) = delete;

    void ProcessInput() const;
    [[nodiscard]] bool ShouldClose() const;

    [[nodiscard]] GLFWwindow* Window() const noexcept;
};

} // namespace whsim