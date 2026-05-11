#pragma once

#include <cstdint>

#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

namespace whsim {

enum class MenuCond : std::uint8_t {MAIN, SETTINGS, SIMULATION, GRAPHICS};
enum class StyleUI : std::uint8_t {LIGHT, DARK};

struct PreviewImage {
    GLuint texture;
    int width, height;
};

class UIController final {
private:
    PreviewImage prev_img_{};
    StyleUI style_ui_{StyleUI::DARK};
    MenuCond menu_cond_{MenuCond::MAIN};

public:
    UIController();
    ~UIController();

    const PreviewImage& GetPreviewImage() const noexcept;
    const MenuCond& GetMenuCond() const noexcept;

    void SetMenuCond(const MenuCond menu_cond) noexcept;
};

} // namespace whsim