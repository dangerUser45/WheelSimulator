#pragma once

namespace whsim::ui {

struct SettingsState final {
    bool show_grid = true;
    bool show_forces = false;

    float spring_k = 15000.0f;
    float damper_c = 1200.0f;
    float wheel_mass = 35.0f;

    int road_mode = 0;
};

void DrawHeroPanel();
bool DrawTopToolbar(bool show_settings);
void DrawSettingsPanel(SettingsState& state, bool* show_settings);
void DrawStatusPanel(const SettingsState& state);

} // namespace whsim::ui
