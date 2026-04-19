#pragma once

#include <glad/gl.h>

#include <imgui.h>

#include "view.hpp"

namespace whsim::ui {

void ConfigureImGui();
void ConfigureImGuiFont();
void DrawPreviewImage(const PreviewImage& preview_image);
void DrawMenu(MenuCond& menu_cond, GLuint& texture);
void DrawSimulation();
void DrawSettings();
void DrawGraphics();

inline constexpr int WINDOW_WIDTH      = 1280;
inline constexpr int WINDOW_HEIGTH     = 720;
inline constexpr int MIN_WINDOW_WIDTH  = 800;
inline constexpr int MIN_WINDOW_HEIGTH = 600;

inline constexpr float CLEAR_COLOR_RED   = 29.0f / 255.0f;
inline constexpr float CLEAR_COLOR_GREEN = 32.0f / 255.0f;
inline constexpr float CLEAR_COLOR_BLUE  = 37.0f / 255.0f;
inline constexpr float CLEAR_COLOR_ALPHA = 1.0f;

inline constexpr float PREVIEW_SCALE = 0.5f;

inline constexpr size_t NUM_BUTTONS = 3;

inline constexpr float  LEFT_PANEL_WIDTH = 150.0f;
inline constexpr ImVec2 LEFT_PANEL_PADDING {20.0f, 0.0f};
inline constexpr ImVec2 LEFT_PANEL_ITEM_SPACING {0.0f, 12.0f};
// inline constexpr float  LEFT_PANEL_TOP_SPACER = 300.0f;

inline constexpr ImVec4 LEFT_PANEL_BG_COLOR     {17.0f / 255.0f, 17.0f / 255.0f, 19.0f / 255.0f, 1.00f};
inline constexpr ImVec4 LEFT_PANEL_BORDER_COLOR {36.0f / 255.0f, 38.0f / 255.0f, 43.0f / 255.0f, 1.00f};
inline constexpr ImVec4 CONTENT_BG_COLOR        {29.0f / 255.0f, 32.0f / 255.0f, 37.0f / 255.0f, 1.00f};

inline constexpr ImU32 BACKGROUND_OVERLAY_PACKED = IM_COL32(20, 22, 26, 214);
inline constexpr ImU32 MENU_HEADER_TEXT_COLOR    = IM_COL32(255, 255, 255, 255);

inline constexpr ImU32 MENU_BUTTON_HOVER_BG_PACKED    = IM_COL32(31, 34, 39, 255);
inline constexpr ImU32 MENU_BUTTON_TEXT_PACKED        = IM_COL32(219, 223, 230, 255);
inline constexpr ImU32 MENU_BUTTON_ACTIVE_TEXT_PACKED = IM_COL32(0, 144, 255, 255);
inline constexpr ImU32 MENU_BUTTON_ACTIVE_INDICATOR_PACKED = IM_COL32(0, 144, 255, 255);

inline constexpr ImVec2 MENU_BUTTON_SIZE {LEFT_PANEL_WIDTH, 52.0f};
inline constexpr float MENU_BUTTON_TEXT_PADDING_X  = 28.0f;
inline constexpr float MENU_BUTTON_INDICATOR_WIDTH = 3.0f;

inline constexpr float BASE_FONT_SIZE        = 20.0f;
inline constexpr float SIZE_FONT_HEADER_MENU = 50.0f;

inline constexpr float EDGE_TO_EDGE_WINDOW_ROUNDING    = 0.0f;
inline constexpr float EDGE_TO_EDGE_WINDOW_BORDER_SIZE = 0.0f;

inline constexpr float CONTENT_PANEL_MARGIN     = 32.0f;
inline constexpr float SIMULATION_PANEL_MARGIN  = 120.0f;
inline constexpr float SIMULATION_BUTTON_MARGIN = 50.0f;
inline constexpr float MENU_HEADER_TEXT_MARGIN  = 50.0f;

inline constexpr float SIM_RECT_ROUNDING = 10.0f;


inline constexpr ImGuiWindowFlags IMGUI_FIXED_WINDOW_FLAGS =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoSavedSettings;

inline constexpr ImGuiWindowFlags IMGUI_BACKGROUND_WINDOW_FLAGS =
    IMGUI_FIXED_WINDOW_FLAGS |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoInputs;

inline constexpr ImGuiWindowFlags IMGUI_CONTENT_WINDOW_FLAGS =
    IMGUI_FIXED_WINDOW_FLAGS |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;

} // namespace whsim::ui
