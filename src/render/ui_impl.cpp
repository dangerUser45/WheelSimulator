#include <string>
#include <cstdint>
#include <algorithm>

#include <imgui.h>

#include "control/sim_controller.hpp"
#include "control/ui_controller.hpp"

#include "config/simulation_settings.hpp"
#include "render/ui_layout.hpp"

namespace whsim::UIImpl {

namespace {

[[nodiscard]] bool LoadFont(ImGuiIO& io, const ImFontConfig& config, const ImWchar* glyph_ranges)
{
    std::string font_path = std::string(PRESETS_PATH) + "font/geist_bold.ttf";
     if(io.Fonts->AddFontFromFileTTF(font_path.c_str(), UILayout::BASE_FONT_SIZE, &config, glyph_ranges) != nullptr)
        return true;

    return false;
}

struct MenuButtonPalette {
    ImU32 fill;
    ImU32 indicator;
    ImU32 text;
};

[[nodiscard]] MenuButtonPalette GetMenuButtonPalette(bool hovered, bool active)
{
    return {
        hovered ? UILayout::MENU_BUTTON_HOVER_BG_PACKED : 0,
        active  ? UILayout::MENU_BUTTON_ACTIVE_INDICATOR_PACKED : 0,
        active  ? UILayout::MENU_BUTTON_ACTIVE_TEXT_PACKED
                : UILayout::MENU_BUTTON_TEXT_PACKED
    };
}

[[nodiscard]] bool DrawButton(const char* label, bool active)
{
    ImGui::PushID(label);
    const ImVec2 window_pos = ImGui::GetWindowPos();
    const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(window_pos.x, cursor_pos.y));
    ImGui::InvisibleButton("##btn", UILayout::MENU_BUTTON_SIZE);

    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 pmin = ImGui::GetItemRectMin();
    const ImVec2 pmax = ImGui::GetItemRectMax();
    const MenuButtonPalette palette = GetMenuButtonPalette(hovered, active);
    const ImVec2 row_min(window_pos.x, pmin.y);
    const ImVec2 row_max(window_pos.x + UILayout::LEFT_PANEL_WIDTH, pmax.y);

    draw->PushClipRect(
        window_pos,
        ImVec2(window_pos.x + UILayout::LEFT_PANEL_WIDTH, window_pos.y + ImGui::GetWindowSize().y),
        true);

    if (palette.fill != 0)
        draw->AddRectFilled(row_min, row_max, palette.fill, 0.0f);

    if (palette.indicator != 0)
        draw->AddRectFilled(
            row_min,
            ImVec2(row_min.x + UILayout::MENU_BUTTON_INDICATOR_WIDTH, row_max.y),
            palette.indicator,
            0.0f);

    draw->PopClipRect();

    const ImVec2 text_size = ImGui::CalcTextSize(label);
    const ImVec2 text_pos(
        window_pos.x + UILayout::MENU_BUTTON_TEXT_PADDING_X,
        pmin.y + (UILayout::MENU_BUTTON_SIZE.y - text_size.y) * 0.5f);

    draw->AddText(text_pos, palette.text, label);

    ImGui::PopID();
    return clicked;
}

[[nodiscard]] bool DrawSimControlButton(const char* label, bool active, ImU32 accent)
{
    ImGui::PushID(label);

    const float x_offset = (UILayout::LEFT_PANEL_WIDTH - UILayout::SIMULATION_BUTTON_SIZE.x) * 0.5f;
    ImGui::SetCursorPosX(x_offset);
    ImGui::InvisibleButton("##control", UILayout::SIMULATION_BUTTON_SIZE);

    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 pmin = ImGui::GetItemRectMin();
    const ImVec2 pmax = ImGui::GetItemRectMax();

    const ImU32 fill = hovered
        ? UILayout::SIMULATION_BUTTON_HOVER_BG_PACKED
        : UILayout::SIMULATION_BUTTON_BG_PACKED;
    const ImU32 border = hovered || active ? accent : UILayout::SIMULATION_BUTTON_BORDER_PACKED;
    const ImU32 text = hovered || active ? accent : UILayout::SIMULATION_BUTTON_TEXT_PACKED;

    draw->AddRectFilled(
        pmin,
        pmax,
        fill,
        UILayout::SIMULATION_BUTTON_ROUNDING,
        ImDrawFlags_RoundCornersAll);

    draw->AddRect(
        pmin,
        pmax,
        border,
        UILayout::SIMULATION_BUTTON_ROUNDING,
        ImDrawFlags_RoundCornersAll,
        1.25f);

    const ImVec2 text_size = ImGui::CalcTextSize(label);
    const ImVec2 text_pos(
        pmin.x + (UILayout::SIMULATION_BUTTON_SIZE.x - text_size.x) * 0.5f,
        pmin.y + (UILayout::SIMULATION_BUTTON_SIZE.y - text_size.y) * 0.5f);

    draw->AddText(text_pos, text, label);

    ImGui::PopID();
    return clicked;
}

} // namespace

void ConfigureImGui()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(0.0f, 0.0f);
}

void ConfigureImGuiFont()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config{};
    config.SizePixels = UILayout::BASE_FONT_SIZE;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = false;

    const ImWchar* glyph_ranges = io.Fonts->GetGlyphRangesCyrillic();
    if (!LoadFont(io, config, glyph_ranges))
        io.Fonts->AddFontDefault(&config);
    else {
        //TODO обработать случай
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = UILayout::BASE_FONT_SIZE;
}

void DrawPreviewImage(const PreviewImage& img)
{
    if(!img.texture) return;

    ImGuiViewport* vp = ImGui::GetMainViewport();

    const ImVec2 img_size{
        static_cast<float>(img.width) * UILayout::PREVIEW_SCALE,
        static_cast<float>(img.height) * UILayout::PREVIEW_SCALE
    };
    const ImVec2 pos(
        vp->Pos.x + (vp->Size.x + UILayout::LEFT_PANEL_WIDTH - img_size.x) * 0.5f,
        vp->Pos.y + (vp->Size.y - img_size.y) * 0.5f);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(img_size, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, UILayout::CONTENT_BG_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::Begin("PreviewImage", nullptr, UILayout::IMGUI_BACKGROUND_WINDOW_FLAGS);
    
    const ImVec2 pmin(pos);
    const ImVec2 pmax(pmin.x + img_size.x, pmin.y + img_size.y);

    ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)img.texture, pmin, pmax);

    ImGui::End();
    ImGui::PopStyleColor(2);
}

void DrawMenu(UIController& ui_ctrl, SimController& sim_ctrl)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 pos = vp->Pos;
    ImVec2 size(UILayout::LEFT_PANEL_WIDTH, vp->Size.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, UILayout::LEFT_PANEL_BG_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, UILayout::LEFT_PANEL_PADDING);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UILayout::EDGE_TO_EDGE_WINDOW_BORDER_SIZE);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, UILayout::LEFT_PANEL_ITEM_SPACING);

    ImGui::Begin("LeftBar", nullptr, UILayout::IMGUI_FIXED_WINDOW_FLAGS);
    
    const bool show_controls = ui_ctrl.GetMenuCond() == MenuCond::SIMULATION;
    
    const float control_block_height =
        UILayout::SIMULATION_BUTTON_SIZE.y * 2.0f + UILayout::LEFT_PANEL_ITEM_SPACING.y;

    const float menu_block_height =
        UILayout::MENU_BUTTON_SIZE.y * static_cast<float>(UILayout::NUM_BUTTONS) +
        UILayout::LEFT_PANEL_ITEM_SPACING.y * static_cast<float>(UILayout::NUM_BUTTONS - 1);
    
    const float menu_top =
        menu_block_height < size.y ? (size.y - menu_block_height) * 0.5f : 20.0f;
    
    const float controls_top = std::max(
        20.0f,
        menu_top - control_block_height - UILayout::SIMULATION_TO_SECTION_SPACING);

    if (show_controls) {
        ImGui::SetCursorPosY(controls_top);

        const char* pause_label = sim_ctrl.IsStopped() ? "Continue" : "Pause";
        const ImU32 pause_accent = sim_ctrl.IsStopped()
            ? UILayout::SIMULATION_PLAY_ACCENT_PACKED
            : UILayout::SIMULATION_PAUSE_ACCENT_PACKED;

        if (DrawSimControlButton(pause_label, sim_ctrl.IsStopped(), pause_accent)) {
            sim_ctrl.SetStopFlag(!sim_ctrl.IsStopped());
        }

        if (DrawSimControlButton("Reset", false, UILayout::SIMULATION_RESET_ACCENT_PACKED)) {
            sim_ctrl.SetStopFlag(true);
            sim_ctrl.SetResetFlag(true);
        }
    }

    ImGui::SetCursorPosY(menu_top);

    if (DrawButton("Settings", ui_ctrl.GetMenuCond() == MenuCond::SETTINGS)) {
        ui_ctrl.SetMenuCond(MenuCond::SETTINGS);
    }
    if (DrawButton("Simulation", ui_ctrl.GetMenuCond() == MenuCond::SIMULATION)) {
        ui_ctrl.SetMenuCond(MenuCond::SIMULATION);
    }
    if (DrawButton("Graphics", ui_ctrl.GetMenuCond() == MenuCond::GRAPHICS)) {
        ui_ctrl.SetMenuCond(MenuCond::GRAPHICS);
    }

    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(1);
}

template <typename FuncT>
void DrawSection(FuncT draw_smth, const char* label, bool transparent_background = false)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 pos_window(vp->Pos.x + UILayout::LEFT_PANEL_WIDTH, vp->Pos.y);
    ImVec2 size_window(vp->Size.x - UILayout::LEFT_PANEL_WIDTH, vp->Size.y);

    ImGui::SetNextWindowPos(pos_window, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size_window, ImGuiCond_Always);

    ImVec2 pos_text(
        pos_window.x + UILayout::CONTENT_PANEL_MARGIN,
        pos_window.y + UILayout::CONTENT_PANEL_MARGIN
    );

    const ImVec4 window_bg = transparent_background
        ? ImVec4(
            UILayout::CONTENT_BG_COLOR.x,
            UILayout::CONTENT_BG_COLOR.y,
            UILayout::CONTENT_BG_COLOR.z,
            0.0f)
        : UILayout::CONTENT_BG_COLOR;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, window_bg);
    ImGui::PushStyleColor(ImGuiCol_Border,   window_bg);

    ImGui::Begin(label, nullptr, UILayout::IMGUI_CONTENT_WINDOW_FLAGS);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetIO().Fonts->Fonts[0];

    ImU32 text_color = UILayout::MENU_HEADER_TEXT_COLOR;

    draw->AddText(font, UILayout::SIZE_FONT_HEADER_MENU, pos_text, text_color, label);

    ImGui::SetCursorPos(ImVec2(0.0f, UILayout::CONTENT_PANEL_MARGIN + UILayout::SIZE_FONT_HEADER_MENU));
    ImGui::Separator();

    draw_smth(draw, pos_window, size_window);
    
    ImGui::End();
    ImGui::PopStyleColor(2);
}

void DrawSimulation(const GLuint simulation_texture)
{
    auto draw_rect = [simulation_texture](auto draw, ImVec2 pos_window, ImVec2 size_window) {
        ImVec2 sim_pmin(
            pos_window.x + UILayout::CONTENT_PANEL_MARGIN,
            pos_window.y + UILayout::SIMULATION_PANEL_MARGIN);
        
        ImVec2 sim_size(
            size_window.x - UILayout::CONTENT_PANEL_MARGIN * 2.0f,
            size_window.y - (UILayout::SIMULATION_PANEL_MARGIN
                        + UILayout::CONTENT_PANEL_MARGIN));

        ImVec2 sim_pmax(sim_pmin.x + sim_size.x, sim_pmin.y + sim_size.y);

        draw->AddRectFilled(
            sim_pmin,
            sim_pmax,
            ImGui::GetColorU32(UILayout::CONTENT_BG_COLOR),
            UILayout::SIM_RECT_ROUNDING,
            ImDrawFlags_RoundCornersAll);

        if (simulation_texture != 0) {
            draw->AddImageRounded(
                (ImTextureID)(intptr_t)simulation_texture,
                sim_pmin,
                sim_pmax,
                ImVec2(0.0f, 1.0f),
                ImVec2(1.0f, 0.0f),
                IM_COL32_WHITE,
                UILayout::SIM_RECT_ROUNDING,
                ImDrawFlags_RoundCornersAll);
        }

        draw->AddRect(sim_pmin, sim_pmax,
            UILayout::MENU_HEADER_TEXT_COLOR, UILayout::SIM_RECT_ROUNDING,
            ImDrawFlags_RoundCornersAll, 1.0f);
    };

    DrawSection(draw_rect, "Simulation", true);
}

void DrawSimulationFullscreen(const GLuint simulation_texture)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(vp->Size, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, UILayout::CONTENT_BG_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border, UILayout::CONTENT_BG_COLOR);

    ImGui::Begin(
        "SimulationFullscreen",
        nullptr,
        UILayout::IMGUI_BACKGROUND_WINDOW_FLAGS);

    if (simulation_texture != 0) {
        const ImVec2 pmin = vp->Pos;
        const ImVec2 pmax(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y);

        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)(intptr_t)simulation_texture,
            pmin,
            pmax,
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f));
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}

void DrawSettings(SimulationSettings& settings)
{
    auto draw_settings = [&settings](auto draw, ImVec2 pos_window, ImVec2 size_window){
        (void)draw;
        (void)pos_window;
        (void)size_window;

        constexpr float settings_width = 320.0f;
        const char* camera_modes[] = {"FollowWheel", "Free"};
        int camera_mode = settings.camera.mode == CameraMode::FollowWheel
            ? 0
            : 1;

        ImGui::SetCursorPos(ImVec2(
            UILayout::CONTENT_PANEL_MARGIN,
            UILayout::SIMULATION_PANEL_MARGIN));

        ImGui::PushItemWidth(settings_width);

        ImGui::TextUnformatted("Wheel");
        ImGui::SliderFloat("Radius", &settings.wheel.radius, 0.10f, 1.00f, "%.2f");
        ImGui::SliderFloat("Width", &settings.wheel.width, 0.05f, 0.50f, "%.2f");
        ImGui::SliderFloat("Mass", &settings.wheel.mass, 0.10f, 20.00f, "%.2f");
        ImGui::SliderFloat("Speed", &settings.wheel.target_angular_speed, -40.00f, 40.00f, "%.2f");

        ImGui::Spacing();
        ImGui::TextUnformatted("Friction");
        ImGui::SliderFloat("Wheel friction", &settings.wheel.friction, 0.00f, 3.00f, "%.2f");
        ImGui::SliderFloat("Rolling friction", &settings.wheel.rolling_friction, 0.00f, 0.25f, "%.3f");
        ImGui::SliderFloat("Spinning friction", &settings.wheel.spinning_friction, 0.00f, 0.25f, "%.3f");
        ImGui::SliderFloat("Terrain friction", &settings.terrain.friction, 0.00f, 3.00f, "%.2f");

        ImGui::Spacing();
        ImGui::TextUnformatted("Terrain");
        ImGui::SliderFloat("Height", &settings.terrain.height_amplitude, 0.00f, 0.50f, "%.2f");
        ImGui::SliderFloat("Frequency", &settings.terrain.noise_frequency, 0.05f, 2.00f, "%.2f");

        ImGui::Spacing();
        ImGui::TextUnformatted("Camera");
        if (ImGui::Combo("View mode", &camera_mode, camera_modes, 2)) {
            settings.camera.mode = camera_mode == 0
                ? CameraMode::FollowWheel
                : CameraMode::Free;
        }

        ImGui::PopItemWidth();
    };
    DrawSection(draw_settings, "Settings");
}

void DrawGraphics()
{
    auto draw_graphics = [](auto draw, ImVec2 pos_window, ImVec2 size_window){
        (void)draw;
        (void)pos_window;
        (void)size_window;
    };
    DrawSection(draw_graphics, "Graphics");
}

} // namespace whsim::UIImpl
