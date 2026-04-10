#include <iostream>
#include <stdexcept>

#include <glad/gl.h>  
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <implot.h>

#include "visual_style.hpp"
#include "ui_layout.hpp"
#include "stb_load_image.hpp"
#include "view.hpp"

namespace whsim {

namespace {


// TODO добавить physic_const.hpp и заменить на константы в этом файле
struct SettingsState {
    bool show_settings = true;
    bool show_grid = true;
    bool show_forces = false;

    float spring_k = 15000.0f;
    float damper_c = 1200.0f;
    float wheel_mass = 35.0f;

    int road_mode = 0;
};

void DrawSettingsPanel(SettingsState& s)
{
    if (!s.show_settings) {
        return;
    }

    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 panel_pos(vp->Pos.x + vp->Size.x - 360.0f, vp->Pos.y + 80.0f);
    ImVec2 panel_size(340.0f, vp->Size.y - 120.0f);

    ImGui::SetNextWindowPos(panel_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Settings", nullptr, flags);

    ImGui::Text("Simulation settings");
    ImGui::Separator();

    ImGui::Checkbox("Show grid", &s.show_grid);
    ImGui::Checkbox("Show forces", &s.show_forces);

    ImGui::SliderFloat("Spring k", &s.spring_k, 1000.0f, 50000.0f, "%.1f");
    ImGui::SliderFloat("Damper c", &s.damper_c, 10.0f, 10000.0f, "%.1f");
    ImGui::SliderFloat("Wheel mass", &s.wheel_mass, 1.0f, 100.0f, "%.1f");

    const char* road_modes[] = { "Flat", "Sine", "Step", "Random" };
    ImGui::Combo("Road mode", &s.road_mode, road_modes, IM_ARRAYSIZE(road_modes));

    ImGui::End();
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

bool DrawTopButton(const char* label, ImVec2 pos, ImVec2 size) {
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin("TopButtonOverlay", nullptr, flags);

    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton(label, size);

    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pmin = ImGui::GetItemRectMin();
    ImVec2 pmax = ImGui::GetItemRectMax();

    ImU32 bg = hovered
        ? IM_COL32(80, 90, 110, 220)
        : IM_COL32(50, 55, 70, 180);

    draw->AddRectFilled(pmin, pmax, bg, 12.0f);
    draw->AddText(
        ImVec2(pmin.x + 18.0f, pmin.y + 12.0f),
        IM_COL32(255, 255, 255, 255),
        label
    );

    ImGui::End();
    return clicked;
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
    GLFWwindow* window = glfwCreateWindow(ui::win_width, ui::win_height, "WheelSimulator", nullptr, nullptr);
    if (!window)
        throw std::runtime_error("Error: Failed to create GLFW window\n");

    window_ = window;
    
    // 4) Делаем контекст текущим
    glfwMakeContextCurrent(window);

    // 5) Загружаем OpenGL-функции через GLAD
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
        std::runtime_error("Error: Failed to initialize GLAD\n");

    // 6) Начальный viewport
    glViewport(0, 0, ui::win_width, ui::win_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    glfwSetWindowSizeLimits(window_, ui::min_win_width, ui::min_win_height, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void View::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsLight();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    // TODO переместить удаление в функцию и вызывать после удаления стартового экрана
    if (preview_texture_ != 0) {
      glDeleteTextures(1, &preview_texture_);
      preview_texture_ = 0;
  }
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

    // FIXME убрать это
    ImVec2 button_pos{200, 200};
    ImVec2 button_size{200,100};

    if(DrawTopButton("Open Settings", button_pos, button_size)) 
        show_menu_ = !show_menu_;
    
    SettingsState state{};
    DrawSettingsPanel(state);

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

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    ImVec2 image_size{static_cast<float>(preview_texture_w_),
                      static_cast<float>(preview_texture_h_)};

    ImVec2 pmin(
        vp->Pos.x + (vp->Size.x - image_size.x) * 0.5f,
        vp->Pos.y + (vp->Size.y - image_size.y) * 0.5f);

    ImVec2 pmax(
        pmin.x + image_size.x,
        pmin.y + image_size.y);

    draw_list->AddImage(
        (ImTextureID)(intptr_t)preview_texture_,
        pmin,
        pmax);
}

} // namespace whsim