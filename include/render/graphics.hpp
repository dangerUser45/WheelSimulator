#pragma once

#include <vector>

#include "physics/physics.hpp"

namespace whsim {

struct GraphicsSeries {
    std::vector<float> time{};
    std::vector<float> position_x{};
    std::vector<float> position_y{};
    std::vector<float> position_z{};
    std::vector<float> speed{};
    std::vector<float> acceleration{};
    std::vector<float> total_energy{};
};

class Graphics final {
private:
    GraphicsSeries series_{};
    PhysicsTelemetry previous_{};
    float elapsed_time_ = 0.0f;
    bool has_previous_ = false;

public:
    void Clear();
    void Record(float dt, const PhysicsTelemetry& telemetry);

    [[nodiscard]] const GraphicsSeries& Series() const noexcept;
};

} // namespace whsim
