#pragma once

#include "control/ui_controller.hpp"

namespace whsim {

class SimController final {
private:
    bool is_stopped_ = true;
    bool is_requested_reset_ = false;

public:
    [[nodiscard]] bool IsStopped() const noexcept;
    [[nodiscard]] bool IsReseted() const noexcept;

    [[nodiscard]] bool ShouldStepSimulation(const MenuCond&) const noexcept;
    [[nodiscard]] bool RequestReset() noexcept;

    void SetStopFlag(bool new_stop_flag) noexcept;
    void SetResetFlag(bool new_reset_flag) noexcept;
};

} // namespace whsim
