#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <LinearMath/btVector3.h>

#include <cmath>

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

Physics::Physics(const SimulationSettings& settings) :
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
    settings_(settings),
    terrain_grid_(settings_.terrain),
    world_origin_offset_{0.0f, 0.0f, 0.0f},
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

    auto terrain_shape = std::make_unique<btHeightfieldTerrainShape>(
        terrain_grid_.SamplesX(),
        terrain_grid_.SamplesY(),
        terrain_grid_.HeightsData(),
        1.0f,                       //height scale: with scale=1.0f => 1metr = 1.0f
        settings_.terrain.min_height,
        settings_.terrain.max_height,
        2,                          // height axis: 0-X, 1-Y, 2-Z 
        PHY_FLOAT,                  // floating data 
        false                       // flipQuadEdges
    );
    terrain_shape->setLocalScaling(btVector3(
        settings_.terrain.cell_size,
        settings_.terrain.cell_size,
        1.0f
    ));

    btTransform ground_transform{};
    ground_transform.setIdentity();
    ground_transform.setOrigin(btVector3{
        terrain_grid_.CenterX(),
        terrain_grid_.CenterY(),
        0.0f
    });

    auto terrain_motion = std::make_unique<btDefaultMotionState>(ground_transform);


    btRigidBody::btRigidBodyConstructionInfo info(
        0.0f,                          // mass = 0 => static body
        terrain_motion.get(),
        terrain_shape.get(),
        btVector3(0.0f, 0.0f, 0.0f)   // position in rigid body hitbox
    );

    auto terrain_body = std::make_unique<btRigidBody>(info);

    terrain_body->setFriction(settings_.terrain.friction);

    world_->addRigidBody(terrain_body.get());

    terrain_ = {std::move(terrain_shape), std::move(terrain_motion), std::move(terrain_body)};
}

void Physics::CreateWheel(CarSide side)
{
    const float wheel_width = settings_.wheel.width;
    const float wheel_radius = settings_.wheel.radius;
    const float wheel_mass = settings_.wheel.mass;

    auto wheel_shape = std::make_unique<btCylinderShapeX>(
        btVector3{wheel_width * 0.5f, wheel_radius, wheel_radius}
    );

    btVector3 inertia {0.0f, 0.0f, 0.0f};

    // TODO исправить инерцию с равномерно распределённой на более реалистичную
    wheel_shape->calculateLocalInertia(wheel_mass, inertia);

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

    wheel_body->setFriction(settings_.wheel.friction);
    wheel_body->setRollingFriction(settings_.wheel.rolling_friction);
    wheel_body->setSpinningFriction(settings_.wheel.spinning_friction);
    wheel_body->setDamping(0.02f, 0.05f);
    wheel_body->setActivationState(DISABLE_DEACTIVATION);

    world_->addRigidBody(wheel_body.get());

    wheels_[idx(side)] =
        {std::move(wheel_shape), std::move(motion_state), std::move(wheel_body)};
}

void Physics::ApplyWheelDrive(CarSide side)
{
    btRigidBody* body = wheels_[idx(side)].rigid_body.get();

    if (body == nullptr) {
        return;
    }

    const float current_speed = body->getAngularVelocity().x();
    const float torque =
        (settings_.wheel.target_angular_speed - current_speed) *
        settings_.wheel.drive_torque;

    body->applyTorque(btVector3{torque, 0.0f, 0.0f});
}

void Physics::Step(float dt)
{
    UpdateTerrainAroundWheel();
    ApplyWheelDrive(CarSide::LF);

    world_->stepSimulation(dt, 8, 1.0f / 60.0f);
}

void Physics::ApplySettings(const SimulationSettings& settings)
{
    settings_ = settings;
    ApplyMaterialSettings();
}

void Physics::ApplyMaterialSettings()
{
    for (BulletObj& wheel : wheels_) {
        if (wheel.rigid_body == nullptr) {
            continue;
        }

        wheel.rigid_body->setFriction(settings_.wheel.friction);
        wheel.rigid_body->setRollingFriction(settings_.wheel.rolling_friction);
        wheel.rigid_body->setSpinningFriction(settings_.wheel.spinning_friction);
    }

    if (terrain_.rigid_body != nullptr) {
        terrain_.rigid_body->setFriction(settings_.terrain.friction);
    }
}

void Physics::UpdateTerrainAroundWheel()
{
    const btRigidBody* body = wheels_[idx(CarSide::LF)].rigid_body.get();

    if (body == nullptr) {
        return;
    }

    const btVector3 origin = body->getWorldTransform().getOrigin() +
        world_origin_offset_;
    const btVector3 old_center{
        terrain_grid_.CenterX(),
        terrain_grid_.CenterY(),
        0.0f
    };

    if (terrain_grid_.CenterAround(origin.x(), origin.y())) {
        const btVector3 new_center{
            terrain_grid_.CenterX(),
            terrain_grid_.CenterY(),
            0.0f
        };
        const btVector3 delta = new_center - old_center;
        world_origin_offset_ += delta;
        RebaseWorld(delta);
        MoveTerrainBody();
    }
}

void Physics::MoveTerrainBody()
{
    if (terrain_.rigid_body == nullptr) {
        return;
    }

    btTransform ground_transform{};
    ground_transform.setIdentity();
    ground_transform.setOrigin(btVector3{
        terrain_grid_.CenterX() - world_origin_offset_.x(),
        terrain_grid_.CenterY() - world_origin_offset_.y(),
        0.0f
    });

    terrain_.rigid_body->setWorldTransform(ground_transform);

    if (terrain_.state != nullptr) {
        terrain_.state->setWorldTransform(ground_transform);
    }

    world_->updateSingleAabb(terrain_.rigid_body.get());
}

void Physics::RebaseWorld(const btVector3& delta)
{
    auto rebase = [this, &delta](BulletObj& object) {
        if (object.rigid_body == nullptr || object.rigid_body == terrain_.rigid_body) {
            return;
        }

        btTransform transform = object.rigid_body->getWorldTransform();
        transform.setOrigin(transform.getOrigin() - delta);
        object.rigid_body->setWorldTransform(transform);

        if (object.state != nullptr) {
            object.state->setWorldTransform(transform);
        }

        world_->updateSingleAabb(object.rigid_body.get());
    };

    for (BulletObj& wheel : wheels_) {
        rebase(wheel);
    }
    for (BulletObj& suspension : suspensions_) {
        rebase(suspension);
    }
    rebase(car_body_);
}

PhysicsTelemetry Physics::Telemetry() const noexcept
{
    PhysicsTelemetry telemetry{};
    const BulletObj* target = nullptr;

    if (wheels_[idx(CarSide::LF)].rigid_body != nullptr) {
        target = &wheels_[idx(CarSide::LF)];
    } else if (car_body_.rigid_body != nullptr) {
        target = &car_body_;
    }

    if (target != nullptr) {
        const btVector3 position =
            target->rigid_body->getWorldTransform().getOrigin() +
            world_origin_offset_;
        const btVector3 velocity = target->rigid_body->getLinearVelocity();

        telemetry.position_x = position.x();
        telemetry.position_y = position.y();
        telemetry.position_z = position.z();
        telemetry.velocity_x = velocity.x();
        telemetry.velocity_y = velocity.y();
        telemetry.velocity_z = velocity.z();
        telemetry.speed = velocity.length();
    }

    constexpr float gravity = 9.81f;
    auto add_energy = [&telemetry](const BulletObj& object) {
        if (object.rigid_body == nullptr || object.rigid_body->isStaticObject()) {
            return;
        }

        const float mass = 1.0f / object.rigid_body->getInvMass();
        const btVector3 velocity = object.rigid_body->getLinearVelocity();
        const btVector3 angular_velocity = object.rigid_body->getAngularVelocity();
        const float height = object.rigid_body->getWorldTransform().getOrigin().z();
        telemetry.total_energy +=
            0.5f * mass * velocity.length2() +
            0.5f * mass * angular_velocity.length2() +
            mass * gravity * height;
    };

    for (const BulletObj& wheel : wheels_) {
        add_energy(wheel);
    }
    for (const BulletObj& suspension : suspensions_) {
        add_energy(suspension);
    }
    add_energy(car_body_);

    return telemetry;
}

const std::vector<Physics::BulletObj>& Physics::Wheels() const noexcept { return wheels_; }
const std::vector<Physics::BulletObj>& Physics::Suspensions() const noexcept { return suspensions_; }
const Physics::BulletObj& Physics::Terrain() const noexcept { return terrain_; }
const TerrainGrid& Physics::TerrainGridData() const noexcept { return terrain_grid_; }
const Physics::BulletObj& Physics::CarBody() const noexcept { return car_body_; }

} // namespace whsim
