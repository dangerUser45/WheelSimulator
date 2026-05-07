#include <BulletCollision/CollisionShapes/btCylinderShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btVector3.h>
#include <btBulletDynamicsCommon.h>

#include "physics.hpp"

namespace whsim {

Physics::Physics() :
    collision_config_(std::make_unique<btDefaultCollisionConfiguration>()),
    dispatcher_(std::make_unique<btCollisionDispatcher>(collision_config_.get())),
    broadphase_(std::make_unique<btDbvtBroadphase>()),
    solver_(std::make_unique<btSequentialImpulseConstraintSolver>()),
    world_(std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher_.get(),
        broadphase_.get(),
        solver_.get(),
        collision_config_.get()
    ))
{
    constexpr float EARTH_GRAVITATIONAL_ACCELERATION = -9.81f;
    world_->setGravity(btVector3(0.0f, EARTH_GRAVITATIONAL_ACCELERATION, 0.0f));

    CreateGround();
    CreateWheel();
}

Physics::~Physics() {}

void Physics::CreateGround()
{
    auto&& ground_shape = shapes_.emplace_back(std::make_unique<btStaticPlaneShape>(
        btVector3(0.0f, 1.0f, 0.0f),   // normal vector 
        0.0f                           // offset
    ));

    btTransform ground_transform;
    ground_transform.setIdentity();
    ground_transform.setOrigin(btVector3(0.0f, 0.0f, 0.0f));

    auto&& ground_motion_state = motion_states_.emplace_back(
        std::make_unique<btDefaultMotionState>(ground_transform)
    );

    btRigidBody::btRigidBodyConstructionInfo info{
        0.0f,                          // mass = 0 => static body
        ground_motion_state.get(),
        ground_shape.get(),
        btVector3(0.0f, 0.0f, 0.0f)    // inertia для static body = 0
    };

    auto&& ground_body = rigid_bodies_.emplace_back(std::make_unique<btRigidBody>(info));
    
    constexpr float GROUND_FRICTION = 1.0f;
    ground_body->setFriction(GROUND_FRICTION);

    world_->addRigidBody(ground_body.get());
}

void Physics::CreateWheel()
{
    constexpr float WHEEL_RADIUS = 0.35f;
    constexpr float WHEEL_MASS   = 0.35f;
    constexpr float WHEEL_WIDTH  = 0.12f;

    auto&& wheel_shape = shapes_.emplace_back(std::make_unique<btCylinderShapeX>(
        btVector3{WHEEL_WIDTH * 0.5, WHEEL_RADIUS, WHEEL_RADIUS}
    ));

    btVector3 inertia {0.0f, 0.0f, 0.0f};
    wheel_shape->calculateLocalInertia(WHEEL_MASS, inertia); // TODO исправить инерцию с равномерно распределённой на более реалистичную

    btTransform start_transform;
    start_transform.setIdentity();
    start_transform.setOrigin(btVector3(0.0f, 1.0f, 0.0f));

    auto&& motion_state = motion_states_.emplace_back(
        std::make_unique<btDefaultMotionState>(start_transform)
    );

    btRigidBody::btRigidBodyConstructionInfo info{
        WHEEL_MASS,
        motion_state.get(),
        wheel_shape.get(),
        inertia
    };

    auto&& wheel_body = rigid_bodies_.emplace_back(std::make_unique<btRigidBody>(info));

    constexpr float WHEEL_FRICTION          = 1.20f;
    constexpr float WHEEL_ROLLING_FRICTION  = 0.02f;
    constexpr float WHEEL_SPINNING_FRICTION = 0.02f;

    wheel_body->setFriction(WHEEL_FRICTION);
    wheel_body->setRollingFriction(WHEEL_ROLLING_FRICTION);
    wheel_body->setSpinningFriction(WHEEL_SPINNING_FRICTION);

    world_->addRigidBody(wheel_body.get());
}

void Physics::Step(float dt) { world_->stepSimulation(dt, 8, 1.0f / 60.0f); }



} // namespace whsim
