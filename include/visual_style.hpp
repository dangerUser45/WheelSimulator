#pragma once

namespace whsim::vstyle {

struct glColor final {
    float r, g, b, alpha;
};

inline constexpr const glColor DarkBkgnd{33.0f / 255.0f,
                                         33.0f / 255.0f,
                                         33.0f / 255.0f, 1.0f};

} // namespace whsim::decor