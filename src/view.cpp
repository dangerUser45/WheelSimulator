#include <iostream>
#include <stdexcept>

#include <glad/gl.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "ui_theme.hpp"
#include "visual_style.hpp"
#include "ui_layout.hpp"
#include "stb_load_image.hpp"
#include "view_panels.hpp"
#include "view.hpp"

namespace whsim {

namespace {

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

} // namespace

View::View()
{
    InitWindow();
    InitImGui();
}

View::~View()
{
    DestroyImGui();
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

void View::InitWindow()
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
    GLFWwindow* window = glfwCreateWindow(ui::WindowWidth, ui::WindowHeight, "WheelSimulator", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("Error: Failed to create GLFW window\n");
    }

    window_ = window;
    
    // 4) Делаем контекст текущим
    glfwMakeContextCurrent(window);

    // 5) Загружаем OpenGL-функции через GLAD
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Error: Failed to initialize GLAD\n");
    }

    // 6) Начальный viewport
    glViewport(0, 0, ui::WindowWidth, ui::WindowHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    glfwSetWindowSizeLimits(
        window_,
        ui::MinWindowWidth,
        ui::MinWindowHeight,
        GLFW_DONT_CARE,
        GLFW_DONT_CARE);
}

void View::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    radix::ConfigureImGuiFonts();
    radix::ApplyImGuiTheme();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // SETUP_PATH defined in CMakeLists.txt
    // It points to the "setup/" directory in the project root.
    const std::string preview = std::string(SETUP_PATH) + "preview.png";

    std::cerr << preview << std::endl;
    GLuint preview_texture = stb::LoadTexture(preview.c_str(), preview_texture_w_, preview_texture_h_);

    if(!preview_texture)
        throw std::runtime_error("Error: preview texture didn't load\n");

    preview_texture_ = preview_texture;
}

void View::DestroyImGui()
{
    if (preview_texture_ != 0) {
        glDeleteTextures(1, &preview_texture_);
        preview_texture_ = 0;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

bool View::ShouldClose() const
{
    return glfwWindowShouldClose(window_) != 0;
}

void View::ProcessInput() const
{
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window_, true);
}

void View::RenderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawBackgroundImage();
    ui::DrawHeroPanel();

    if (ui::DrawTopToolbar(show_settings_)) {
        show_settings_ = !show_settings_;
    }

    ui::DrawSettingsPanel(settings_, &show_settings_);
    ui::DrawStatusPanel(settings_);

    ImGui::Render();

    glClearColor(cur_bkgnd_color.r, cur_bkgnd_color.g, cur_bkgnd_color.b, cur_bkgnd_color.alpha);

    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window_);
}

void View::DrawBackgroundImage()
{
    if (preview_texture_ == 0) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    ImVec2 image_size{static_cast<float>(preview_texture_w_),
                      static_cast<float>(preview_texture_h_)};

    ImVec2 pmin(
        viewport->Pos.x + (viewport->Size.x - image_size.x) * 0.5f,
        viewport->Pos.y + (viewport->Size.y - image_size.y) * 0.5f);

    ImVec2 pmax(
        pmin.x + image_size.x,
        pmin.y + image_size.y);

    draw_list->AddImage(
        (ImTextureID)(intptr_t)preview_texture_,
        pmin,
        pmax);

    const ImVec2 overlay_max(
        viewport->Pos.x + viewport->Size.x,
        viewport->Pos.y + viewport->Size.y);

    draw_list->AddRectFilled(
        viewport->Pos,
        overlay_max,
        IM_COL32(
            10,
            13,
            18,
            static_cast<int>(255.0f * ui::BackgroundOverlayAlpha)));
}

} // namespace whsim
