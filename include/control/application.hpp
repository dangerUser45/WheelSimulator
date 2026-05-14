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

    bool space_pressed_ = false;
    bool f11_pressed_ = false;
    bool escape_pressed_ = false;

private:
    void ProcessHotkeys();

public:
    Application();

    void ResetSimulation();
    void RunLoop();
};

} // namespace whsim
