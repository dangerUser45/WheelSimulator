#pragma once

namespace whsim::vstyle {

struct glColor final {
    float r, g, b, alpha;
};

inline constexpr glColor DarkBkgnd{
    17.0f / 255.0f,
    17.0f / 255.0f,
    19.0f / 255.0f,
    1.0f,
};

} // namespace whsim::vstyle
