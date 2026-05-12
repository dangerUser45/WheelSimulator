#include <LinearMath/btTransform.h>

#include "physics/phys_obj_view.hpp"

namespace whsim {

namespace {

void AddObj(auto& phys_objects_container,
    const Physics::BulletObj& object, PhysObjKind kind)
{
    if(!object.rigid_body) return;

    btTransform transform{};
    if (object.rigid_body->getMotionState() != nullptr) {
        object.rigid_body->getMotionState()->getWorldTransform(transform);
    } else {
        transform = object.rigid_body->getWorldTransform();
    }

    PhysObj phys_obj{};
    phys_obj.kind = kind;
    transform.getOpenGLMatrix(phys_obj.obj_matrix.data());
    phys_objects_container.push_back(phys_obj);
}

void FillObjKind(auto& phys_objects_container,
    const std::vector<Physics::BulletObj>& objects, PhysObjKind kind)
{
    for(auto&& object : objects) {
        AddObj(phys_objects_container, object, kind);
    }
}

}

PhysObjView::PhysObjView(const Physics& physics) :
    physics_(physics),
    phys_objects_{}
{
    Update();
}

void PhysObjView::Update()
{
    FillObjectsContainer();
}

void PhysObjView::FillObjectsContainer()
{
    phys_objects_.clear();
    FillObjKind(phys_objects_, physics_.Wheels(), PhysObjKind::Wheel);
    FillObjKind(phys_objects_, physics_.Suspensions(), PhysObjKind::Suspension);
    AddObj(phys_objects_, physics_.Terrain(), PhysObjKind::Terrain);
    AddObj(phys_objects_, physics_.CarBody(), PhysObjKind::CarBody);
}

const std::vector<PhysObj>& PhysObjView::PhysObjects() const noexcept
{
    return phys_objects_;
}

} // namespace whsim
