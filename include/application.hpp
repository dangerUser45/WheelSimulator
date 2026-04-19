#pragma once

#include "view.hpp"
#include "physics.hpp"

namespace whsim {

class Application final {
private:
    View view_{};
    Physics physic_{};

public:
    void RunLoop();
};

} // namespace whsim
