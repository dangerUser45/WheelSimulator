#pragma once

#include <array>
#include <random>

#include <GLFW/glfw3.h>

namespace whsim {

class Application final {
private:
    static constexpr std::size_t kHistorySize = 240;

    GLFWwindow* window_ = nullptr;
    bool ui_initialized_ = false;
    bool show_controls_ = true;
    bool show_stats_ = true;
    bool animate_graph_ = true;

    std::array<float, 4> clear_color_ {0.08f, 0.09f, 0.12f, 1.0f};
    std::array<float, kHistorySize> plot_values_ {};
    float plot_phase_ = 0.0f;
    float plot_value_ = 0.0f;
    float plot_amplitude_ = 0.8f;
    float plot_noise_ = 0.15f;

    std::mt19937 rng_ {std::random_device{}()};
    std::normal_distribution<float> noise_dist_ {0.0f, 1.0f};

    void InitImGui();
    void DestroyImGui();
    void RenderUi();
    void UpdatePlotData(float dt);

public:
    Application();
    ~Application();

    void RunLoop();
};

} // namespace whsim