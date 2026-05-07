#pragma once

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>
#include <memory>
#include <vector>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <btBulletDynamicsCommon.h>

namespace whsim {

class Physics final {
private:
    std::unique_ptr<btDefaultCollisionConfiguration> collision_config_;
    std::unique_ptr<btCollisionDispatcher> dispatcher_;
    std::unique_ptr<btBroadphaseInterface> broadphase_;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
    
    std::unique_ptr<btDiscreteDynamicsWorld> world_;

    std::vector<std::unique_ptr<btCollisionShape>> shapes_;
    std::vector<std::unique_ptr<btMotionState>> motion_states_;
    std::vector<std::unique_ptr<btRigidBody>> rigid_bodies_;

    void CreateGround();
    void CreateWheel();


public:
    Physics();
    ~Physics();

    void Step(float dt);
};

} // namespace whsim
