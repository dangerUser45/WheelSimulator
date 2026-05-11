#include <LinearMath/btTransform.h>

#include "physics/phys_obj_view.hpp"

namespace whsim {

namespace {

void FillObjKind(auto& phys_objects_container,
    const std::vector<Physics::BulletObj>& obj, PhysObjKind kind, std::size_t& idx)
{
    for(auto&& object : obj) {
        if(!object.rigid_body) continue;

        btTransform transform{};
        object.rigid_body->getMotionState()->getWorldTransform(transform);

        phys_objects_container[idx].kind = kind;
        transform.getOpenGLMatrix(phys_objects_container[idx].obj_matrix.data());
        ++idx;
    }
}

void FillObjKind(auto& phys_objects_container,
    const Physics::BulletObj& object, PhysObjKind kind, std::size_t& idx)
{
    if(!object.rigid_body) return;

    btTransform transform{};
    object.rigid_body->getMotionState()->getWorldTransform(transform);

    phys_objects_container[idx].kind = kind;
    transform.getOpenGLMatrix(phys_objects_container[idx].obj_matrix.data());
    ++idx;
}

}

PhysObjView::PhysObjView(const Physics& physics) :
    physics_(physics),
    phys_objects_(10)
{}

void PhysObjView::FillObjectsContainer()
{
    std::size_t idx = 0;
    FillObjKind(phys_objects_, physics_.Wheels(),      PhysObjKind::Wheel,      idx);
    FillObjKind(phys_objects_, physics_.Suspensions(), PhysObjKind::Suspension, idx);
    FillObjKind(phys_objects_, physics_.Terrain(),     PhysObjKind::Terrain,    idx);
    FillObjKind(phys_objects_, physics_.CarBody(),     PhysObjKind::CarBody,    idx);
}

} // namespace whsim