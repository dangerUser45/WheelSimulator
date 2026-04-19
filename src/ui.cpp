#include <string>

#include <imgui.h>

#include "ui.hpp"

namespace whsim::ui {

namespace {

[[nodiscard]] bool LoadFont(ImGuiIO& io, const ImFontConfig& config, const ImWchar* glyph_ranges)
{
    std::string font_path = std::string(PRESETS_PATH) + "fonts/geist_bold.ttf";
     if(io.Fonts->AddFontFromFileTTF(font_path.c_str(), ui::BASE_FONT_SIZE, &config, glyph_ranges) != nullptr)
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
        hovered ? ui::MENU_BUTTON_HOVER_BG_PACKED : 0,
        active  ? ui::MENU_BUTTON_ACTIVE_INDICATOR_PACKED : 0,
        active  ? ui::MENU_BUTTON_ACTIVE_TEXT_PACKED
                : ui::MENU_BUTTON_TEXT_PACKED
    };
}

[[nodiscard]] bool DrawButton(const char* label, bool active)
{
    ImGui::PushID(label);
    const ImVec2 window_pos = ImGui::GetWindowPos();
    const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(window_pos.x, cursor_pos.y));
    ImGui::InvisibleButton("##btn", ui::MENU_BUTTON_SIZE);

    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 pmin = ImGui::GetItemRectMin();
    const ImVec2 pmax = ImGui::GetItemRectMax();
    const MenuButtonPalette palette = GetMenuButtonPalette(hovered, active);
    const ImVec2 row_min(window_pos.x, pmin.y);
    const ImVec2 row_max(window_pos.x + ui::LEFT_PANEL_WIDTH, pmax.y);

    draw->PushClipRect(
        window_pos,
        ImVec2(window_pos.x + ui::LEFT_PANEL_WIDTH, window_pos.y + ImGui::GetWindowSize().y),
        true);

    if (palette.fill != 0)
        draw->AddRectFilled(row_min, row_max, palette.fill, 0.0f);

    if (palette.indicator != 0)
        draw->AddRectFilled(
            row_min,
            ImVec2(row_min.x + ui::MENU_BUTTON_INDICATOR_WIDTH, row_max.y),
            palette.indicator,
            0.0f);

    draw->PopClipRect();

    const ImVec2 text_size = ImGui::CalcTextSize(label);
    const ImVec2 text_pos(
        window_pos.x + ui::MENU_BUTTON_TEXT_PADDING_X,
        pmin.y + (ui::MENU_BUTTON_SIZE.y - text_size.y) * 0.5f);

    draw->AddText(text_pos, palette.text, label);

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
    config.SizePixels = ui::BASE_FONT_SIZE;
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
    style.FontSizeBase = ui::BASE_FONT_SIZE;
}

void DrawPreviewImage(const PreviewImage& img)
{
    if(!img.texture) return;

    ImGuiViewport* vp = ImGui::GetMainViewport();

    const ImVec2 img_size{
        static_cast<float>(img.width) * ui::PREVIEW_SCALE,
        static_cast<float>(img.heigth) * ui::PREVIEW_SCALE
    };
    const ImVec2 pos(
        vp->Pos.x + (vp->Size.x + ui::LEFT_PANEL_WIDTH - img_size.x) * 0.5f,
        vp->Pos.y + (vp->Size.y - img_size.y) * 0.5f);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(img_size, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ui::CONTENT_BG_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::Begin("PreviewImage", nullptr, ui::IMGUI_BACKGROUND_WINDOW_FLAGS);
    
    const ImVec2 pmin(pos);
    const ImVec2 pmax(pmin.x + img_size.x, pmin.y + img_size.y);

    ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)img.texture, pmin, pmax);

    ImGui::End();
    ImGui::PopStyleColor(2);
}

void DrawMenu(MenuCond& menu_cond, GLuint& texture)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 pos = vp->Pos;
    ImVec2 size(ui::LEFT_PANEL_WIDTH, vp->Size.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ui::LEFT_PANEL_BG_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ui::LEFT_PANEL_PADDING);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ui::EDGE_TO_EDGE_WINDOW_BORDER_SIZE);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ui::LEFT_PANEL_ITEM_SPACING);

    ImGui::Begin("LeftBar", nullptr, ui::IMGUI_FIXED_WINDOW_FLAGS);
    
    float buttons_block_height =
        (ui::MENU_BUTTON_SIZE.y + ui::LEFT_PANEL_ITEM_SPACING.y * 2) * ui::NUM_BUTTONS;
    float left_panel_top_space =
        (size.y - buttons_block_height) * 0.5;
  
    ImGui::Dummy(ImVec2(0.0f, left_panel_top_space));

    if (DrawButton("Settings", menu_cond == MenuCond::SETTINGS)) {
        menu_cond = MenuCond::SETTINGS;
        texture = static_cast<GLuint>(NULL);
    }
    if (DrawButton("Simulation", menu_cond == MenuCond::SIMULATION)) {
        menu_cond = MenuCond::SIMULATION;
        texture = static_cast<GLuint>(NULL);
    }
    if (DrawButton("Graphics", menu_cond == MenuCond::GRAPHICS)) {
        menu_cond = MenuCond::GRAPHICS;
        texture = static_cast<GLuint>(NULL);
    }

    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(1);
}

template <typename FuncT>
void DrawSection(FuncT draw_smth, const char* label)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 pos_window(vp->Pos.x + ui::LEFT_PANEL_WIDTH, vp->Pos.y);
    ImVec2 size_window(vp->Size.x - ui::LEFT_PANEL_WIDTH, vp->Size.y);

    ImGui::SetNextWindowPos(pos_window, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size_window, ImGuiCond_Always);

    ImVec2 pos_text(
        pos_window.x + ui::CONTENT_PANEL_MARGIN,
        pos_window.y + ui::CONTENT_PANEL_MARGIN
    );

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ui::CONTENT_BG_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border,   ui::CONTENT_BG_COLOR);

    ImGui::Begin(label, nullptr, ui::IMGUI_CONTENT_WINDOW_FLAGS);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetIO().Fonts->Fonts[0];

    ImU32 text_color = ui::MENU_HEADER_TEXT_COLOR;

    draw->AddText(font, ui::SIZE_FONT_HEADER_MENU, pos_text, text_color, label);

    ImGui::SetCursorPos(ImVec2(0.0f, ui::CONTENT_PANEL_MARGIN + ui::SIZE_FONT_HEADER_MENU));
    ImGui::Separator();

    draw_smth(draw, pos_window, size_window);
    
    ImGui::End();
    ImGui::PopStyleColor(2);
}

void DrawSimulation()
{
    auto draw_rect = [](auto draw, ImVec2 pos_window, ImVec2 size_window) {
        ImVec2 sim_pmin(
            pos_window.x + ui::CONTENT_PANEL_MARGIN,
            pos_window.y + ui::SIMULATION_PANEL_MARGIN);
        
        ImVec2 sim_size(
            size_window.x - ui::CONTENT_PANEL_MARGIN * 2.0f,
            size_window.y - (ui::SIMULATION_PANEL_MARGIN
                        + ui::CONTENT_PANEL_MARGIN));

        ImVec2 sim_pmax(sim_pmin.x + sim_size.x, sim_pmin.y + sim_size.y);
        draw->AddRect(sim_pmin, sim_pmax,
            ui::MENU_HEADER_TEXT_COLOR, ui::SIM_RECT_ROUNDING,
            ImDrawFlags_RoundCornersAll, 1.0f);
    };

    DrawSection(draw_rect, "Simulation");
}

void DrawSettings()
{
    auto draw_settings = [](auto, ImVec2, ImVec2) {
        static bool enable_simulation = true;
        static bool show_collision_mesh = false;
        static bool auto_center_camera = true;
        static float wheel_radius = 0.34f;
        static float tire_pressure = 2.2f;
        static float suspension_stiffness = 16.0f;
        static float damping_ratio = 0.55f;
        static float camber_angles[2] = {-1.0f, 1.0f};
        static int solver_iterations = 12;
        static int drive_mode = 1;
        static int quality_mode = 2;
        static float tune_factors[8] = {
            0.82f, 0.64f, 0.71f, 0.58f,
            0.67f, 0.49f, 0.76f, 0.61f
        };
        static constexpr const char* drive_modes[] = {
            "Comfort",
            "Sport",
            "Track"
        };

        const float child_pos_y = ImGui::GetCursorPosY() + ui::CONTENT_PANEL_MARGIN;
        ImGui::SetCursorPos(ImVec2(ui::CONTENT_PANEL_MARGIN, child_pos_y));

        const ImVec2 child_size(
            ImGui::GetContentRegionAvail().x - ui::CONTENT_PANEL_MARGIN,
            ImGui::GetContentRegionAvail().y - ui::CONTENT_PANEL_MARGIN);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::LEFT_PANEL_BG_COLOR);
        ImGui::PushStyleColor(ImGuiCol_Border, ui::LEFT_PANEL_BORDER_COLOR);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ui::SIM_RECT_ROUNDING);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

        ImGui::BeginChild(
            "SettingsPanel",
            child_size,
            ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
            ImGuiWindowFlags_None);

        ImGui::TextUnformatted("General");
        ImGui::Separator();
        ImGui::Checkbox("Enable simulation", &enable_simulation);
        ImGui::Checkbox("Show collision mesh", &show_collision_mesh);
        ImGui::Checkbox("Auto-center camera", &auto_center_camera);
        ImGui::SliderFloat("Wheel radius", &wheel_radius, 0.20f, 0.80f, "%.2f m");
        ImGui::SliderInt("Solver iterations", &solver_iterations, 4, 64);

        ImGui::Spacing();
        ImGui::TextUnformatted("Dynamics");
        ImGui::Separator();
        ImGui::DragFloat("Tire pressure", &tire_pressure, 0.05f, 1.0f, 3.5f, "%.2f bar");
        ImGui::DragFloat("Suspension stiffness", &suspension_stiffness, 0.25f, 1.0f, 30.0f, "%.2f");
        ImGui::SliderFloat("Damping ratio", &damping_ratio, 0.0f, 1.0f, "%.2f");
        ImGui::DragFloat2("Camber", camber_angles, 0.1f, -10.0f, 10.0f, "%.1f deg");

        ImGui::Spacing();
        ImGui::TextUnformatted("Modes");
        ImGui::Separator();
        ImGui::Combo("Drive mode", &drive_mode, drive_modes, IM_ARRAYSIZE(drive_modes));
        ImGui::RadioButton("Low", &quality_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Medium", &quality_mode, 1);
        ImGui::SameLine();
        ImGui::RadioButton("High", &quality_mode, 2);

        if (ImGui::Button("Reset tuning")) {
            wheel_radius = 0.34f;
            tire_pressure = 2.2f;
            suspension_stiffness = 16.0f;
            damping_ratio = 0.55f;
            solver_iterations = 12;
            camber_angles[0] = -1.0f;
            camber_angles[1] = 1.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply track preset")) {
            drive_mode = 2;
            quality_mode = 2;
            tire_pressure = 2.6f;
            suspension_stiffness = 22.0f;
            damping_ratio = 0.70f;
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Advanced");
        ImGui::Separator();
        for (int i = 0; i < IM_ARRAYSIZE(tune_factors); ++i) {
            ImGui::PushID(i);
            const std::string label = "Tune factor " + std::to_string(i + 1);
            ImGui::SliderFloat(label.c_str(), &tune_factors[i], 0.0f, 1.0f, "%.2f");
            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    };

    DrawSection(draw_settings, "Settings");
}

void DrawGraphics()
{
    auto draw_graphics = [](auto draw, ImVec2 pos_window, ImVec2 size_window){};
    DrawSection(draw_graphics, "Graphics");
}

} // namespace whsim
