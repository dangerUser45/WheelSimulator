#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <btBulletDynamicsCommon.h>

namespace whsim {

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

    Physics();
    ~Physics();

    Physics(const Physics&) = delete;
    Physics& operator=(const Physics&) = delete;
    Physics(Physics&&) = delete;
    Physics& operator=(Physics&&) = delete;

    void Step(float dt);
    void ResetSimulation();

    [[nodiscard]] const std::vector<BulletObj>& Wheels() const noexcept;
    [[nodiscard]] const std::vector<BulletObj>& Suspensions() const noexcept;
    [[nodiscard]] const BulletObj& Terrain() const noexcept;
    [[nodiscard]] const BulletObj& CarBody() const noexcept;

private:
    std::unique_ptr<btDefaultCollisionConfiguration> collision_config_;
    std::unique_ptr<btCollisionDispatcher> dispatcher_;
    std::unique_ptr<btBroadphaseInterface> broadphase_;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
    
    std::unique_ptr<btDiscreteDynamicsWorld> world_;

    std::vector<float> terrain_heights_;

    // какие в целом объекты:
    //      земля (массив высот)
    //      4 колеса
    //      4 подвески (пружины)
    //      1 корпус машины

    // struct BulletObj;

    std::vector<BulletObj> wheels_;
    std::vector<BulletObj> suspensions_;
    BulletObj car_body_;
    BulletObj terrain_;

    void CreateTerrain();
    void CreateWheel(CarSide side);
    void ApplyWheelTorque(btVector3& torque_vec, CarSide side);
};

} // namespace whsim
