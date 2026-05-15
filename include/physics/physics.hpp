#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <btBulletDynamicsCommon.h>

#include "config/simulation_settings.hpp"
#include "physics/terrain_grid.hpp"

namespace whsim {

struct PhysicsTelemetry {
    float position_x = 0.0f;
    float position_y = 0.0f;
    float position_z = 0.0f;
    float speed = 0.0f;
    float velocity_x = 0.0f;
    float velocity_y = 0.0f;
    float velocity_z = 0.0f;
    float total_energy = 0.0f;
};

enum class CarSide : std::uint8_t {
    LF = 0,     // left forward side
    RF = 1,     // right forward side
    LB = 2,     // left backward side
    RB = 3      // right backward side
};

constexpr auto idx(CarSide side) { return static_cast<std::uint8_t>(side); }

class Physics final {
public:
    struct BulletObj {
        std::unique_ptr<btCollisionShape> shape{};
        std::unique_ptr<btMotionState> state{};
        std::unique_ptr<btRigidBody> rigid_body{};
    };

    explicit Physics(const SimulationSettings& settings);
    ~Physics();

    Physics(const Physics&) = delete;
    Physics& operator=(const Physics&) = delete;
    Physics(Physics&&) = delete;
    Physics& operator=(Physics&&) = delete;

    void Step(float dt);
    void ResetSimulation();
    void ApplySettings(const SimulationSettings& settings);

    [[nodiscard]] const std::vector<BulletObj>& Wheels() const noexcept;
    [[nodiscard]] const std::vector<BulletObj>& Suspensions() const noexcept;
    [[nodiscard]] const BulletObj& Terrain() const noexcept;
    [[nodiscard]] const TerrainGrid& TerrainGridData() const noexcept;
    [[nodiscard]] const BulletObj& CarBody() const noexcept;
    [[nodiscard]] PhysicsTelemetry Telemetry() const noexcept;

private:
    std::unique_ptr<btDefaultCollisionConfiguration> collision_config_;
    std::unique_ptr<btCollisionDispatcher> dispatcher_;
    std::unique_ptr<btBroadphaseInterface> broadphase_;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
    
    std::unique_ptr<btDiscreteDynamicsWorld> world_;

    SimulationSettings settings_;
    TerrainGrid terrain_grid_;
    btVector3 world_origin_offset_{0.0f, 0.0f, 0.0f};

    std::vector<BulletObj> wheels_;
    std::vector<BulletObj> suspensions_;
    BulletObj car_body_;
    BulletObj terrain_;

    void CreateTerrain();
    void CreateWheel(CarSide side);
    void ApplyMaterialSettings();
    void ApplyWheelDrive(CarSide side);
    void UpdateTerrainAroundWheel();
    void MoveTerrainBody();
    void RebaseWorld(const btVector3& delta);
};

} // namespace whsim
