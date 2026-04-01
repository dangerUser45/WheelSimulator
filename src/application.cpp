#include <iostream>
#include <stdexcept>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "application.hpp"

namespace whsim {
  
static void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

Application::Application()
{
    // 1) Инициализация GLFW
    if(!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    // 2) Просим OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    #ifdef __APPLE__
    // Для macOS обычно нужен этот hint
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // 3) Создаём окно
    GLFWwindow* window = glfwCreateWindow(1280, 720, "WheelSimulator", nullptr, nullptr);
    if (!window)
        throw std::runtime_error("Failed to create GLFW window");

    window_ = window;
    
    // 4) Делаем контекст текущим
    glfwMakeContextCurrent(window);

    // 5) Загружаем OpenGL-функции через GLAD
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        // return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n';

    // 6) Начальный viewport
    glViewport(0, 0, 1280, 720);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSwapInterval(1);
    InitImGui();
}

Application::~Application()
{
    DestroyImGui();
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

void Application::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ui_initialized_ = true;
}

void Application::DestroyImGui()
{
    if (!ui_initialized_)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ui_initialized_ = false;
}

void Application::UpdatePlotData(float dt)
{
    if (!animate_graph_)
        return;

    for (std::size_t i = 0; i + 1 < plot_values_.size(); ++i)
        plot_values_[i] = plot_values_[i + 1];

    plot_phase_ += dt;
    plot_value_ =
        std::sin(plot_phase_ * 2.0f) * plot_amplitude_ +
        noise_dist_(rng_) * plot_noise_;

    plot_values_.back() = plot_value_;
}

void Application::RenderUi()
{
    ImGui::DockSpaceOverViewport();

    if (show_controls_ && ImGui::Begin("Controls", &show_controls_)) {
        if (ImGui::Button(animate_graph_ ? "Pause graph" : "Start graph"))
            animate_graph_ = !animate_graph_;

        if (ImGui::Button("Random background")) {
            clear_color_[0] = 0.2f + noise_dist_(rng_) * 0.2f;
            clear_color_[1] = 0.3f + noise_dist_(rng_) * 0.2f;
            clear_color_[2] = 0.4f + noise_dist_(rng_) * 0.2f;
            clear_color_[0] = std::clamp(clear_color_[0], 0.0f, 1.0f);
            clear_color_[1] = std::clamp(clear_color_[1], 0.0f, 1.0f);
            clear_color_[2] = std::clamp(clear_color_[2], 0.0f, 1.0f);
        }

        ImGui::ColorEdit3("Background", clear_color_.data());
        ImGui::SliderFloat("Amplitude", &plot_amplitude_, 0.1f, 2.0f);
        ImGui::SliderFloat("Noise", &plot_noise_, 0.0f, 0.5f);
    }
    ImGui::End();

    if (show_stats_ && ImGui::Begin("Stats", &show_stats_)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Current value: %.3f", plot_value_);
    }
    ImGui::End();

    if (ImGui::Begin("Random Plot")) {
        if (ImPlot::BeginPlot("Signal", ImVec2(-1, 300))) {
            ImPlot::SetupAxes("Frame", "Value");
            ImPlot::PlotLine("signal", plot_values_.data(), static_cast<int>(plot_values_.size()));
            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}

void Application::RunLoop()
{
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        processInput(window_);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        UpdatePlotData(ImGui::GetIO().DeltaTime);
        RenderUi();

        ImGui::Render();

        glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window_);
    }
}

} // namespace whsim
