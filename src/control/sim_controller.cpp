#include "control/sim_controller.hpp"

namespace whsim {

GLuint SimController::SimTexture() const noexcept { return sim_texture_; }

bool SimController::IsStopped() const noexcept
{
    return is_stopped_;
}

bool SimController::IsReseted() const noexcept
{
    return is_requested_reset_;
}

bool SimController::ShouldStepSimulation(const MenuCond& menu_cond) const noexcept
{
    if((!is_stopped_) && (menu_cond == MenuCond::SIMULATION)) return true;
    return false;
}

bool SimController::RequestReset() noexcept
{
    if(is_requested_reset_) {
        is_requested_reset_ = false;
        return true;
    }
    return false;
}

void SimController::SetStopFlag(bool new_stop_flag) noexcept
{
    is_stopped_ = new_stop_flag;
}

void SimController::SetResetFlag(bool new_reset_flag) noexcept
{
    is_requested_reset_ = new_reset_flag;
}

} // namespace whsim