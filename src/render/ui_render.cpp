#include <glad/gl.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <stb_load_image.hpp>

#include "render/ui_render.hpp"
#include "render/ui_layout.hpp"
#include "render/ui_impl.hpp"

namespace whsim {

UIRender::UIRender(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    UIImpl::ConfigureImGui();
    UIImpl::ConfigureImGuiFont();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

UIRender::~UIRender()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void UIRender::Render(GLFWwindow* window, UIController& ui_ctrl,
                      SimController& sim_ctrl) const
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawUI(ui_ctrl, sim_ctrl);

    ImGui::Render();
    glClearColor(UILayout::CLEAR_COLOR_RED,
                 UILayout::CLEAR_COLOR_GREEN,
                 UILayout::CLEAR_COLOR_BLUE,
                 UILayout::CLEAR_COLOR_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void UIRender::DrawUI(UIController& ui_ctrl,
                      SimController& sim_ctrl) const
{
    UIImpl::DrawMenu(ui_ctrl, sim_ctrl);

    switch(ui_ctrl.GetMenuCond()) {
        case MenuCond::MAIN: {
            if(ui_ctrl.GetPreviewImage().texture)
                UIImpl::DrawPreviewImage(ui_ctrl.GetPreviewImage());    
            break;
        }
        case MenuCond::SETTINGS:
            UIImpl::DrawSettings(); break;
        
        case MenuCond::SIMULATION:
            UIImpl::DrawSimulation(sim_ctrl.SimTexture()); break;
        
        case MenuCond::GRAPHICS:
            UIImpl::DrawGraphics(); break;
    }
}

} // namespace whsim
