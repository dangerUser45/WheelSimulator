#pragma once

#include "view.hpp"

namespace whsim {

class Application final {
private:
    View view_{};

public:
    void RunLoop();
};

} // namespace whsim
