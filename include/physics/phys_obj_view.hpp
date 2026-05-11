#pragma once

#include <array>

#include "physics/physics.hpp"

namespace whsim {

enum class PhysObjKind : uint8_t {
    Wheel,
    Suspension,
    CarBody,
    Terrain
};

struct PhysObj {
    PhysObjKind kind;
    std::array<btScalar, 16> obj_matrix;
};

class PhysObjView final {
private:
    const Physics& physics_;
    std::vector<PhysObj> phys_objects_;

    void FillObjectsContainer();
    
public:
    PhysObjView(const Physics& physics);

    std::vector<PhysObj>& PhysObjects() const noexcept;
};

} // namespace whsim