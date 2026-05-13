#pragma once

#include <cstdint>

namespace whsim {

enum class CameraMode : std::uint8_t {
    FollowWheel,
    Free
};

struct WheelSettings {
    float radius = 0.35f;
    float width = 0.12f;
    float mass = 1.0f;
    float friction = 1.2f;
    float rolling_friction = 0.02f;
    float spinning_friction = 0.02f;
    float target_angular_speed = 12.0f;
    float drive_torque = 5.0f;
};

struct TerrainSettings {
    int samples_x = 128;
    int samples_y = 128;
    float cell_size = 0.08f;
    float min_height = -1.0f;
    float max_height = 1.0f;
    float friction = 1.0f;
};

struct CameraSettings {
    CameraMode mode = CameraMode::FollowWheel;
    float start_x = 0.0f;
    float start_y = -4.5f;
    float start_z = 2.0f;
    float target_x = 0.0f;
    float target_y = 0.0f;
    float target_z = 0.35f;
    float yaw = 1.57079632679f;
    float pitch = -0.35f;
    float min_pitch = -1.41371669412f;
    float max_pitch = 1.41371669412f;
    float follow_distance = 4.5f;
    float free_move_speed = 5.0f;
    float key_look_speed = 1.8f;
    float mouse_look_speed = 0.003f;
    float max_frame_time = 0.1f;
};

struct SimulationSettings {
    WheelSettings wheel{};
    TerrainSettings terrain{};
    CameraSettings camera{};
};

[[nodiscard]] constexpr bool PhysicsSettingsChanged(
    const SimulationSettings& lhs,
    const SimulationSettings& rhs) noexcept
{
    return
        lhs.wheel.radius != rhs.wheel.radius ||
        lhs.wheel.width != rhs.wheel.width ||
        lhs.wheel.mass != rhs.wheel.mass ||
        lhs.wheel.friction != rhs.wheel.friction ||
        lhs.wheel.rolling_friction != rhs.wheel.rolling_friction ||
        lhs.wheel.spinning_friction != rhs.wheel.spinning_friction ||
        lhs.wheel.target_angular_speed != rhs.wheel.target_angular_speed ||
        lhs.wheel.drive_torque != rhs.wheel.drive_torque ||
        lhs.terrain.samples_x != rhs.terrain.samples_x ||
        lhs.terrain.samples_y != rhs.terrain.samples_y ||
        lhs.terrain.cell_size != rhs.terrain.cell_size ||
        lhs.terrain.min_height != rhs.terrain.min_height ||
        lhs.terrain.max_height != rhs.terrain.max_height ||
        lhs.terrain.friction != rhs.terrain.friction;
}

[[nodiscard]] constexpr bool WheelMeshSettingsChanged(
    const SimulationSettings& lhs,
    const SimulationSettings& rhs) noexcept
{
    return
        lhs.wheel.radius != rhs.wheel.radius ||
        lhs.wheel.width != rhs.wheel.width;
}

} // namespace whsim
