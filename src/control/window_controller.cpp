#include <stdexcept>

#include <glad/gl.h>

#include "control/window_controller.hpp"
#include "render/ui_layout.hpp"

namespace whsim {

namespace {

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double, double yoffset)
{
    auto* controller =
        static_cast<WindowController*>(glfwGetWindowUserPointer(window));

    if (controller != nullptr) {
        controller->AddScrollYOffset(yoffset);
    }
}

} // namespace

WindowController::WindowController(int init_window_width, int init_window_height)
{
    if(!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(init_window_width, init_window_height,
                                          "WheelSimulator", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("Error: Failed to create GLFW window\n");
    }

    window_ = window;
    glfwSetWindowUserPointer(window_, this);
    
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Error: Failed to initialize GLAD\n");
    }

    glViewport(0, 0, UILayout::WINDOW_WIDTH, UILayout::WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);

    glfwSetWindowSizeLimits(window_, UILayout::MIN_WINDOW_WIDTH, UILayout::MIN_WINDOW_HEIGTH,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);
}

WindowController::~WindowController()
{
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

void WindowController::ProcessInput() const
{
    const bool escape_pressed =
        glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    const bool ctrl_pressed =
        glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window_, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    if (escape_pressed && ctrl_pressed) {
        glfwSetWindowShouldClose(window_, true);
    }
}

bool WindowController::ShouldClose() const
{
    return glfwWindowShouldClose(window_) != 0;
}

void WindowController::AddScrollYOffset(double yoffset) noexcept
{
    scroll_y_offset_ += static_cast<float>(yoffset);
}

float WindowController::ConsumeScrollYOffset() noexcept
{
    const float yoffset = scroll_y_offset_;
    scroll_y_offset_ = 0.0;
    return yoffset;
}

GLFWwindow* WindowController::Window() const noexcept { return window_; }

} // namespace whsim
