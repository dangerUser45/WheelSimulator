#include <iostream>

#include <glad/gl.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "ui.hpp"
#include "stb_load_image.hpp"
#include "view.hpp"
#include "physics.hpp"

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
    GLFWwindow* window = glfwCreateWindow(ui::WINDOW_WIDTH, ui::WINDOW_HEIGTH,
                                          "WheelSimulator", nullptr, nullptr);
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
    glViewport(0, 0, ui::WINDOW_WIDTH, ui::WINDOW_HEIGTH);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    glfwSetWindowSizeLimits(window_, ui::MIN_WINDOW_WIDTH, ui::MIN_WINDOW_HEIGTH,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void View::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ui::ConfigureImGui();
    ui::ConfigureImGuiFont();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // PRESETS_PATH defined in CMakeLists.txt
    // It points to the "presets/" directory in the project root.
    std::string preview = std::string(PRESETS_PATH) + "img/";

    switch(style_ui_) {
        case StyleUI::LIGHT: preview += "preview_light.png"; break;
        case StyleUI::DARK:  preview += "preview_dark.png"; break;
        default: std::cerr << "Error: unknown ui style type" << std::endl;
    }

    GLuint preview_texture = stb::LoadTexture(preview.c_str(),
        prev_img_.width, prev_img_.heigth);

    std::cerr << "load preview status: " << preview_texture << std::endl;
    if(!preview_texture) {
        // throw std::runtime_error("Error: preview texture didn't load\n");
        //TODO обработать
    }
    prev_img_.texture = preview_texture;
}

void View::DestroyImGui()
{
    if (prev_img_.texture != 0) {
        glDeleteTextures(1, &prev_img_.texture);
        prev_img_.texture = 0;
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

void View::RenderUI(Physics& physics)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawUI(physics);

    ImGui::Render();
    glClearColor(ui::CLEAR_COLOR_RED,
                 ui::CLEAR_COLOR_GREEN,
                 ui::CLEAR_COLOR_BLUE,
                 ui::CLEAR_COLOR_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);
}

void View::DrawUI(Physics& physics)
{
    if(prev_img_.texture) 
        ui::DrawPreviewImage(prev_img_);

    ui::DrawMenu(menu_cond_, prev_img_.texture);
    switch(menu_cond_) {
        case MenuCond::MAIN:
            break;
        case MenuCond::SETTINGS:
            ui::DrawSettings();
            break;
        case MenuCond::SIMULATION:
            ui::DrawSimulation();
            break;
        case MenuCond::GRAPHICS:
            ui::DrawGraphics();
            break;
    }
}

} // namespace whsim
