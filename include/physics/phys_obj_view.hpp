#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "physics/physics.hpp"

namespace whsim {

enum class PhysObjKind : std::uint8_t {
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

    void Update();

    [[nodiscard]] const std::vector<PhysObj>& PhysObjects() const noexcept;
};

} // namespace whsim
