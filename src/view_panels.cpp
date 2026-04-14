#include <cstdio>

#include <imgui.h>

#include "ui_layout.hpp"
#include "view_panels.hpp"

namespace whsim::ui {

namespace {

constexpr ImGuiWindowFlags kFloatingCardFlags =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoSavedSettings;

void BeginCard(const char* name, const ImVec2& pos, const ImVec2& size)
{
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::Begin(name, nullptr, kFloatingCardFlags);
}

bool DrawButton(const char* label, const ImVec2& size, bool primary)
{
    if (!primary) {
        const ImGuiStyle& style = ImGui::GetStyle();
        ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_FrameBg]);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_FrameBgHovered]);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_FrameBgActive]);
    }

    const bool pressed = ImGui::Button(label, size);

    if (!primary) {
        ImGui::PopStyleColor(3);
    }

    return pressed;
}

void DrawBadge(const char* label)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const ImVec2 text_size = ImGui::CalcTextSize(label);
    const ImVec2 padding(12.0f, 6.0f);
    const ImVec2 pmax(
        start.x + text_size.x + padding.x * 2.0f,
        start.y + text_size.y + padding.y * 2.0f);

    draw->AddRectFilled(start, pmax, IM_COL32(17, 43, 77, 255), 999.0f);
    draw->AddRect(start, pmax, IM_COL32(54, 111, 182, 160), 999.0f);
    draw->AddText(
        ImVec2(start.x + padding.x, start.y + padding.y),
        IM_COL32(207, 231, 255, 255),
        label);

    ImGui::Dummy(ImVec2(pmax.x - start.x, pmax.y - start.y));
}

void DrawMetric(const char* label, const char* value)
{
    ImGui::BeginGroup();
    ImGui::TextDisabled("%s", label);
    ImGui::Text("%s", value);
    ImGui::EndGroup();
}

const char* RoadModeLabel(int road_mode)
{
    constexpr const char* kRoadModes[] = {"Flat", "Sine", "Step", "Random"};

    if (road_mode < 0 || road_mode >= static_cast<int>(IM_ARRAYSIZE(kRoadModes))) {
        return "Unknown";
    }

    return kRoadModes[road_mode];
}

} // namespace

void DrawHeroPanel()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 panel_pos(viewport->WorkPos.x + PageInset, viewport->WorkPos.y + PageInset);
    const ImVec2 panel_size(HeroPanelWidth, HeroPanelHeight);

    BeginCard("HeroPanel", panel_pos, panel_size);

    DrawBadge("RADIX-LIKE SURFACE");
    ImGui::Spacing();

    ImGui::Text("Wheel Simulator");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextWrapped(
        "The interface now uses calmer surfaces, tighter spacing and softer "
        "controls so the UI reads more like a product dashboard than a debug "
        "tool.");
    ImGui::PopTextWrapPos();

    ImGui::SeparatorText("Focus");
    ImGui::TextDisabled("Status");
    ImGui::Text("Preview stage");

    ImGui::SameLine(0.0f, 28.0f);

    ImGui::BeginGroup();
    ImGui::TextDisabled("Direction");
    ImGui::Text("UI shell and controls");
    ImGui::EndGroup();

    ImGui::End();
}

bool DrawTopToolbar(bool show_settings)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 panel_size(ToolbarWidth, ToolbarHeight);
    const ImVec2 panel_pos(
        viewport->WorkPos.x + viewport->WorkSize.x - panel_size.x - PageInset,
        viewport->WorkPos.y + PageInset);

    BeginCard("TopToolbar", panel_pos, panel_size);

    ImGui::TextDisabled("Workspace");
    ImGui::Text("Prototype");
    ImGui::SameLine();

    const float button_x = ImGui::GetWindowWidth() - ToolbarButtonWidth - 18.0f;
    ImGui::SetCursorPosX(button_x);
    const bool pressed = DrawButton(
        show_settings ? "Hide panel" : "Open panel",
        ImVec2(ToolbarButtonWidth, ToolbarButtonHeight),
        true);

    ImGui::End();
    return pressed;
}

void DrawSettingsPanel(SettingsState& state, bool* show_settings)
{
    if (show_settings == nullptr || !*show_settings) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 panel_size(
        SettingsPanelWidth,
        viewport->WorkSize.y - PageInset * 2.0f);
    const ImVec2 panel_pos(
        viewport->WorkPos.x + viewport->WorkSize.x - panel_size.x - PageInset,
        viewport->WorkPos.y + PageInset * 2.0f + ToolbarHeight);

    BeginCard("SettingsPanel", panel_pos, panel_size);

    ImGui::Text("Simulation settings");
    ImGui::SameLine();

    const float button_width = 96.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - button_width - 18.0f);
    if (DrawButton("Collapse", ImVec2(button_width, ToolbarButtonHeight), false)) {
        *show_settings = false;
    }

    ImGui::SeparatorText("Scene");
    ImGui::Checkbox("Show grid", &state.show_grid);
    ImGui::Checkbox("Show forces", &state.show_forces);

    ImGui::SeparatorText("Suspension");
    ImGui::SliderFloat("Spring k", &state.spring_k, 1000.0f, 50000.0f, "%.0f");
    ImGui::SliderFloat("Damper c", &state.damper_c, 10.0f, 10000.0f, "%.0f");
    ImGui::SliderFloat("Wheel mass", &state.wheel_mass, 1.0f, 100.0f, "%.1f kg");

    ImGui::SeparatorText("Road");
    const char* road_modes[] = {"Flat", "Sine", "Step", "Random"};
    ImGui::Combo("Road mode", &state.road_mode, road_modes, IM_ARRAYSIZE(road_modes));

    ImGui::End();
}

void DrawStatusPanel(const SettingsState& state)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 panel_size(StatusPanelWidth, StatusPanelHeight);
    const ImVec2 panel_pos(
        viewport->WorkPos.x + PageInset,
        viewport->WorkPos.y + viewport->WorkSize.y - panel_size.y - PageInset);

    BeginCard("StatusPanel", panel_pos, panel_size);

    ImGui::Text("Overview");
    ImGui::Separator();

    DrawMetric("Grid", state.show_grid ? "Visible" : "Hidden");
    ImGui::SameLine(0.0f, 28.0f);
    DrawMetric("Forces", state.show_forces ? "Visible" : "Hidden");

    ImGui::Spacing();
    DrawMetric("Road profile", RoadModeLabel(state.road_mode));

    char spring_value[32];
    char damper_value[32];
    std::snprintf(spring_value, sizeof(spring_value), "%.0f N/m", state.spring_k);
    std::snprintf(damper_value, sizeof(damper_value), "%.0f Ns/m", state.damper_c);

    ImGui::Spacing();
    DrawMetric("Spring", spring_value);
    ImGui::SameLine(0.0f, 28.0f);
    DrawMetric("Damper", damper_value);

    ImGui::End();
}

} // namespace whsim::ui
