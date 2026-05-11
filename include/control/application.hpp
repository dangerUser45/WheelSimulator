#pragma once

#include "control/sim_controller.hpp"
#include "control/window_controller.hpp"
#include "control/ui_controller.hpp"

#include "physics/phys_obj_view.hpp"
#include "physics/physics.hpp"

#include "render/sim_render.hpp"
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
    
    Physics physics_{};
    PhysObjView phys_obj_view_{physics_};

public:
    void RunLoop();
};

} // namespace whsim
