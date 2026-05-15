#pragma once

#include <memory>

#include "config/simulation_settings.hpp"
#include "control/sim_controller.hpp"
#include "control/ui_controller.hpp"
#include "control/window_controller.hpp"

#include "physics/phys_obj_view.hpp"
#include "physics/physics.hpp"

#include "render/sim_render.hpp"
#include "render/graphics.hpp"
#include "render/ui_layout.hpp"
#include "render/ui_render.hpp"

namespace whsim {

class Application final {
private:
    
    WindowController window_controller_{UILayout::WINDOW_WIDTH, UILayout::WINDOW_HEIGHT};
    SimController sim_controller_{};
    UIController ui_controller_{};
    
    UIRender ui_render_{window_controller_.Window()};
    SimRender sim_render_{};
    Graphics graphics_{};
    SimulationSettings settings_{};
    
    std::unique_ptr<Physics> physics_{};
    std::unique_ptr<PhysObjView> phys_obj_view_{};

    bool pause_toggle_shortcut_pressed_ = false;
    bool fullscreen_toggle_shortcut_pressed_ = false;
    bool reset_simulation_shortcut_pressed_ = false;

    bool back_navigation_shortcut_pressed_ = false;

private:
    void ProcessHotkeys();
    void ProccessMenuSwitching(GLFWwindow* const window, bool ctrl_pressed);
    void ProcessMenuSection(GLFWwindow* const window,
        const MenuCond menu_cond, bool ctrl_pressed, auto glfw_key);

    void ProccessSimulationStates(GLFWwindow* const window, bool ctrl_pressed);
    void ProccessSimReset(GLFWwindow* const window, bool ctrl_pressed, bool sim_is_active);
    void ProccessSimPause(GLFWwindow* const window, bool sim_is_active);
    void ProccessSimFullScreen(GLFWwindow* const window, bool sim_is_active);
    void ProccessBackNavigation(GLFWwindow* const window);

    void ProcessCameraSpeedScroll();

public:
    Application();

    void ResetSimulation();
    void RunLoop();
};

} // namespace whsim
