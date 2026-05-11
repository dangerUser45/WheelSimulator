#include <string>
#include <iostream>

#include <stb_load_image.hpp>

#include "control/ui_controller.hpp"

namespace whsim {

UIController::UIController()
{    
    // PRESETS_PATH defined in CMakeLists.txt
    // It points to the "presets/" directory in the project root.
    std::string preview = std::string(PRESETS_PATH) + "img/";

    switch(style_ui_) {
        case StyleUI::LIGHT: preview += "preview_light.png"; break;
        case StyleUI::DARK:  preview += "preview_dark.png"; break;
        default: std::cerr << "Error: unknown ui style type" << std::endl;
    }

    GLuint preview_texture = stb::LoadTexture(preview.c_str(),
        prev_img_.width, prev_img_.height);

    if(!preview_texture) {
        // throw std::runtime_error("Error: preview texture didn't load\n");
        //TODO обработать
    }
    prev_img_.texture = preview_texture;
}

UIController::~UIController()
{
    if (prev_img_.texture != 0) {
        glDeleteTextures(1, &prev_img_.texture);
        prev_img_.texture = 0;
    }
}

const PreviewImage& UIController::GetPreviewImage() const noexcept
{
    return prev_img_;
}

const MenuCond& UIController::GetMenuCond() const noexcept
{
    return menu_cond_;
}

void UIController::SetMenuCond(const MenuCond menu_cond) noexcept
{
    menu_cond_ = menu_cond;
}

} // namespace whsim