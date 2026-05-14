#include "render/graphics.hpp"

#include <algorithm>
#include <cmath>

namespace whsim {

namespace {

constexpr std::size_t MAX_SAMPLES = 2400;

void TrimFront(std::vector<float>& values)
{
    if (values.size() <= MAX_SAMPLES) {
        return;
    }

    values.erase(values.begin(), values.begin() + static_cast<long>(values.size() - MAX_SAMPLES));
}

} // namespace

void Graphics::Clear()
{
    series_ = GraphicsSeries{};
    previous_ = PhysicsTelemetry{};
    elapsed_time_ = 0.0f;
    has_previous_ = false;
}

void Graphics::Record(float dt, const PhysicsTelemetry& telemetry)
{
    const float safe_dt = std::max(dt, 0.0f);
    elapsed_time_ += safe_dt;

    float acceleration = 0.0f;
    if (has_previous_ && safe_dt > 0.0001f) {
        const float dx = telemetry.velocity_x - previous_.velocity_x;
        const float dy = telemetry.velocity_y - previous_.velocity_y;
        const float dz = telemetry.velocity_z - previous_.velocity_z;
        acceleration = std::sqrt(dx * dx + dy * dy + dz * dz) / safe_dt;
    }

    series_.time.push_back(elapsed_time_);
    series_.position_x.push_back(telemetry.position_x);
    series_.position_y.push_back(telemetry.position_y);
    series_.position_z.push_back(telemetry.position_z);
    series_.speed.push_back(telemetry.speed);
    series_.acceleration.push_back(acceleration);
    series_.total_energy.push_back(telemetry.total_energy);

    TrimFront(series_.time);
    TrimFront(series_.position_x);
    TrimFront(series_.position_y);
    TrimFront(series_.position_z);
    TrimFront(series_.speed);
    TrimFront(series_.acceleration);
    TrimFront(series_.total_energy);

    previous_ = telemetry;
    has_previous_ = true;
}

const GraphicsSeries& Graphics::Series() const noexcept
{
    return series_;
}

} // namespace whsim
