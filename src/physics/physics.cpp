#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <LinearMath/btVector3.h>

#include "physics/physics.hpp"

namespace whsim {

namespace {

void RemoveRigidBody(btDiscreteDynamicsWorld& world,
                     const Physics::BulletObj& object)
{
    if (object.rigid_body != nullptr) {
        world.removeRigidBody(object.rigid_body.get());
    }
}

}

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
    )),
    terrain_heights_{},
    wheels_(4),
    suspensions_(4),
    car_body_{},
    terrain_{}
{
    constexpr float EARTH_GRAVITATIONAL_ACCELERATION = -9.81f;
    world_->setGravity(btVector3(0.0f, 0.0f, EARTH_GRAVITATIONAL_ACCELERATION));

    CreateTerrain();
    CreateWheel(CarSide::LF); // TODO do a full car
}

Physics::~Physics()
{
    for(auto&& wheel : wheels_) RemoveRigidBody(*world_, wheel);
    for(auto&& suspension : suspensions_) RemoveRigidBody(*world_, suspension);
    RemoveRigidBody(*world_, car_body_);
    RemoveRigidBody(*world_, terrain_);
}

void Physics::CreateTerrain()
{        
    //                length          axis Z        axis Y
    //         ↑←----------------→        ⬤------→
    //         |       -→        |        |
    //         |    o-----o      |        | 
    // width   |    |     |      |        ↓ axis X
    //         |    o-----o      |   
    //         |                 |
    //         ↓_________________|

    constexpr std::size_t init_width_terrain  = 128;
    constexpr std::size_t init_length_terrain = 128;
    
    terrain_heights_.resize(init_width_terrain * init_length_terrain);
    
    constexpr float min_height = -20.0f;
    constexpr float max_height = 20.0f;

    for(std::size_t length_idx = 0; length_idx < init_length_terrain; ++length_idx)
        for(std::size_t width_idx = 0; width_idx < init_width_terrain; ++width_idx) {
            float height = 0;

            // height = std::min();
            terrain_heights_[length_idx * init_width_terrain + width_idx] = height;
        }

    auto terrain_shape = std::make_unique<btHeightfieldTerrainShape>(
        init_width_terrain, init_length_terrain,
        terrain_heights_.data(),
        1.0f,                       //height scale: with scale=1.0f => 1metr = 1.0f
        min_height, max_height,
        2,                          // height axis: 0-X, 1-Y, 2-Z 
        PHY_FLOAT,                  // floating data 
        false                       // flipQuadEdges
    );
    terrain_shape->setLocalScaling(btVector3(0.2f, 0.2f, 0.2f)); // TODO возможно стоит поменять

    btTransform ground_transform{};
    ground_transform.setIdentity();
    ground_transform.setOrigin(btVector3{0.0f, 0.0f, 0.0f});

    auto terrain_motion = std::make_unique<btDefaultMotionState>(ground_transform);


    btRigidBody::btRigidBodyConstructionInfo info(
        0.0f,                          // mass = 0 => static body
        terrain_motion.get(),
        terrain_shape.get(),
        btVector3(0.0f, 0.0f, 0.0f)   // position in rigid body hitbox
    );

    auto terrain_body = std::make_unique<btRigidBody>(info);

    constexpr float terrain_friction = 1.0f;
    terrain_body->setFriction(terrain_friction);

    world_->addRigidBody(terrain_body.get());

    terrain_ = {std::move(terrain_shape), std::move(terrain_motion), std::move(terrain_body)};
}

void Physics::CreateWheel(CarSide side)
{
    constexpr float wheel_width  = 0.12f;
    constexpr float wheel_radius = 0.35f;
    constexpr float wheel_mass   = 1.00f;

    auto wheel_shape = std::make_unique<btCylinderShapeX>(
        btVector3{wheel_width * 0.5, wheel_radius, wheel_radius}
    );

    btVector3 inertia {0.0f, 0.0f, 0.0f};
    wheel_shape->calculateLocalInertia(wheel_mass, inertia); // TODO исправить инерцию с равномерно распределённой на более реалистичную

    btTransform start_transform;
    start_transform.setIdentity();
    start_transform.setOrigin(btVector3(0.0f, 0.0f, wheel_radius));

    auto motion_state = std::make_unique<btDefaultMotionState>(start_transform);

    btRigidBody::btRigidBodyConstructionInfo info{
        wheel_mass,
        motion_state.get(),
        wheel_shape.get(),
        inertia
    };

    auto wheel_body = std::make_unique<btRigidBody>(info);

    constexpr float wheel_friction          = 1.20f;
    constexpr float wheel_rolling_friction  = 0.02f;
    constexpr float wheel_spinning_friction = 0.02f;

    wheel_body->setFriction(wheel_friction);
    wheel_body->setRollingFriction(wheel_rolling_friction);
    wheel_body->setSpinningFriction(wheel_spinning_friction);
    wheel_body->setDamping(0.02f, 0.05f);
    wheel_body->setActivationState(DISABLE_DEACTIVATION);

    world_->addRigidBody(wheel_body.get());

    wheels_[idx(side)] =
        {std::move(wheel_shape), std::move(motion_state), std::move(wheel_body)};
}

void Physics::ApplyWheelTorque(btVector3& torque_vec, CarSide side)
{
    wheels_[idx(side)].rigid_body->applyTorque(torque_vec);
}

void Physics::Step(float dt)
{   
    btVector3 torque_vec{0.0f, 20.0f, 0.0f};
    ApplyWheelTorque(torque_vec, CarSide::LF);

    world_->stepSimulation(dt, 8, 1.0f / 60.0f);
}

void Physics::ResetSimulation()
{
    // if (wheel_body_ == nullptr) {
    //     return;
    // }

    // btTransform start_transform;
    // start_transform.setIdentity();
    // start_transform.setOrigin(btVector3(0.0f, 0.0f, WHEEL_RADIUS));

    // wheel_body_->setWorldTransform(start_transform);
    // wheel_body_->setCenterOfMassTransform(start_transform);
    // wheel_body_->setInterpolationWorldTransform(start_transform);

    // if (wheel_body_->getMotionState() != nullptr) {
    //     wheel_body_->getMotionState()->setWorldTransform(start_transform);
    // }

    // wheel_body_->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    // wheel_body_->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    // wheel_body_->clearForces();
    // wheel_body_->activate(true);

    // world_->updateSingleAabb(wheel_body_);
}

const std::vector<Physics::BulletObj>& Physics::Wheels() const noexcept { return wheels_; }
const std::vector<Physics::BulletObj>& Physics::Suspensions() const noexcept { return suspensions_; }
const Physics::BulletObj& Physics::Terrain() const noexcept { return terrain_; }
const Physics::BulletObj& Physics::CarBody() const noexcept { return car_body_; }

} // namespace whsim
