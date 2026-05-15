#pragma once

#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

namespace whsim {

class WindowController final {
private:
    GLFWwindow* window_ = nullptr;
    float scroll_y_offset_ = 0.0;

public:
    WindowController(int init_window_width, int init_window_height);
    ~WindowController();

    WindowController(WindowController&&) = delete;
    WindowController(const WindowController&) = delete;
    WindowController& operator=(WindowController&&) = delete;
    WindowController& operator=(const WindowController&) = delete;

    void ProcessInput() const;
    void AddScrollYOffset(double yoffset) noexcept;
    [[nodiscard]] bool ShouldClose() const;
    [[nodiscard]] float ConsumeScrollYOffset() noexcept;

    [[nodiscard]] GLFWwindow* Window() const noexcept;
};

} // namespace whsim
