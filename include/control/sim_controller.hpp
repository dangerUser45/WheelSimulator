#pragma once

#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

#include "control/ui_controller.hpp"

namespace whsim {

class SimController final {
private:
    GLuint sim_texture_{0};
    bool is_stopped_ = true;
    bool is_requested_reset_ = false;

public:
    [[nodiscard]] GLuint SimTexture() const noexcept;
    [[nodiscard]] bool IsStopped() const noexcept;
    [[nodiscard]] bool IsReseted() const noexcept;

    [[nodiscard]] bool ShouldStepSimulation(const MenuCond&) const noexcept;
    [[nodiscard]] bool RequestReset() noexcept;

    void SetStopFlag(bool new_stop_flag) noexcept;
    void SetResetFlag(bool new_reset_flag) noexcept;
};

} // namespace whsim