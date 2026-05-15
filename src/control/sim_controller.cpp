#include "control/sim_controller.hpp"

namespace whsim {

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
    if(!is_stopped_ &&
        (menu_cond == MenuCond::SIMULATION || menu_cond == MenuCond::GRAPHICS)) {
        return true;
    }

    return false;
}

bool SimController::RequestReset() noexcept
{
    if(is_requested_reset_) {
        this->SetResetFlag(false);
        return true;
    }
    return false;
}

void SimController::SetStopFlag(bool new_stop_flag) noexcept
{
    is_stopped_ = new_stop_flag;
}

void SimController::ToggleStopFlag() noexcept
{
    is_stopped_ = !is_stopped_;
}

void SimController::SetResetFlag(bool new_reset_flag) noexcept
{
    is_requested_reset_ = new_reset_flag;
}

} // namespace whsim
