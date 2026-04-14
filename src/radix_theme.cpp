#include <array>
#include <filesystem>

#include <imgui.h>

#include "radix_theme.hpp"

namespace whsim::radix {

namespace {

constexpr ImVec4 Accent         {0.17f, 0.49f, 0.88f, 1.00f};
constexpr ImVec4 AccentHover    {0.22f, 0.56f, 0.95f, 1.00f};
constexpr ImVec4 AccentActive   {0.14f, 0.42f, 0.79f, 1.00f};
constexpr ImVec4 WindowBg       {0.07f, 0.07f, 0.08f, 0.94f};
constexpr ImVec4 PanelBg        {0.10f, 0.11f, 0.12f, 0.98f};
constexpr ImVec4 PanelBgHover   {0.13f, 0.14f, 0.16f, 1.00f};
constexpr ImVec4 PanelBgActive  {0.16f, 0.18f, 0.20f, 1.00f};
constexpr ImVec4 Border         {0.25f, 0.27f, 0.30f, 0.68f};
constexpr ImVec4 Text           {0.94f, 0.95f, 0.96f, 1.00f};
constexpr ImVec4 TextMuted      {0.60f, 0.64f, 0.69f, 1.00f};
constexpr ImVec4 SurfaceAlt     {0.08f, 0.08f, 0.10f, 1.00f};
constexpr float  BaseFontSize = 17.0f;

bool TryLoadPreferredSansFont(ImGuiIO& io, const ImFontConfig& config, const ImWchar* glyph_ranges)
{
    constexpr std::array kFontCandidates{
        "/usr/share/fonts/Adwaita/AdwaitaSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    };

    for (const char* path : kFontCandidates) {
        if (!std::filesystem::exists(path)) {
            continue;
        }

        if (io.Fonts->AddFontFromFileTTF(path, BaseFontSize, &config, glyph_ranges) != nullptr) {
            return true;
        }
    }

    return false;
}

} // namespace

void ConfigureImGuiFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigDpiScaleFonts = true;
    io.Fonts->Clear();

    ImFontConfig config{};
    config.SizePixels = BaseFontSize;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = false;

    const ImWchar* glyph_ranges = io.Fonts->GetGlyphRangesCyrillic();
    if (!TryLoadPreferredSansFont(io, config, glyph_ranges)) {
        io.Fonts->AddFontDefaultVector(&config);
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = BaseFontSize;
}

void ApplyImGuiTheme()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                  = Text;
    colors[ImGuiCol_TextDisabled]          = TextMuted;
    colors[ImGuiCol_WindowBg]              = WindowBg;
    colors[ImGuiCol_ChildBg]               = PanelBg;
    colors[ImGuiCol_PopupBg]               = PanelBg;
    colors[ImGuiCol_Border]                = Border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg]               = PanelBg;
    colors[ImGuiCol_FrameBgHovered]        = PanelBgHover;
    colors[ImGuiCol_FrameBgActive]         = PanelBgActive;
    colors[ImGuiCol_TitleBg]               = WindowBg;
    colors[ImGuiCol_TitleBgActive]         = WindowBg;
    colors[ImGuiCol_TitleBgCollapsed]      = WindowBg;
    colors[ImGuiCol_MenuBarBg]             = PanelBg;
    colors[ImGuiCol_ScrollbarBg]           = SurfaceAlt;
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.27f, 0.30f, 0.34f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.39f, 0.43f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.42f, 0.46f, 0.51f, 1.0f);
    colors[ImGuiCol_CheckMark]             = Accent;
    colors[ImGuiCol_SliderGrab]            = Accent;
    colors[ImGuiCol_SliderGrabActive]      = AccentHover;
    colors[ImGuiCol_Button]                = Accent;
    colors[ImGuiCol_ButtonHovered]         = AccentHover;
    colors[ImGuiCol_ButtonActive]          = AccentActive;
    colors[ImGuiCol_Header]                = PanelBg;
    colors[ImGuiCol_HeaderHovered]         = PanelBgHover;
    colors[ImGuiCol_HeaderActive]          = PanelBgActive;
    colors[ImGuiCol_Separator]             = Border;
    colors[ImGuiCol_SeparatorHovered]      = AccentHover;
    colors[ImGuiCol_SeparatorActive]       = Accent;
    colors[ImGuiCol_ResizeGrip]            = ImVec4(Accent.x, Accent.y, Accent.z, 0.35f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(AccentHover.x, AccentHover.y, AccentHover.z, 0.72f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(AccentActive.x, AccentActive.y, AccentActive.z, 0.92f);
    colors[ImGuiCol_Tab]                   = PanelBg;
    colors[ImGuiCol_TabHovered]            = PanelBgHover;
    colors[ImGuiCol_TabActive]             = PanelBgActive;
    colors[ImGuiCol_TabUnfocused]          = PanelBg;
    colors[ImGuiCol_TabUnfocusedActive]    = PanelBgActive;
    colors[ImGuiCol_DockingPreview]        = ImVec4(Accent.x, Accent.y, Accent.z, 0.30f);
    colors[ImGuiCol_DockingEmptyBg]        = SurfaceAlt;
    colors[ImGuiCol_PlotLines]             = AccentHover;
    colors[ImGuiCol_PlotLinesHovered]      = Accent;
    colors[ImGuiCol_TableHeaderBg]         = PanelBg;
    colors[ImGuiCol_TableBorderStrong]     = Border;
    colors[ImGuiCol_TableBorderLight]      = Border;
    colors[ImGuiCol_TableRowBgAlt]         = SurfaceAlt;
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(Accent.x, Accent.y, Accent.z, 0.28f);
    colors[ImGuiCol_DragDropTarget]        = AccentHover;
    colors[ImGuiCol_NavHighlight]          = AccentHover;
    colors[ImGuiCol_NavWindowingHighlight] = AccentHover;

    style.Alpha             = 1.0f;
    style.DisabledAlpha     = 0.6f;
    style.WindowPadding     = ImVec2(18.0f, 18.0f);
    style.FramePadding      = ImVec2(14.0f, 10.0f);
    style.CellPadding       = ImVec2(12.0f, 8.0f);
    style.ItemSpacing       = ImVec2(12.0f, 12.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 8.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing     = 16.0f;
    style.ScrollbarSize     = 10.0f;
    style.GrabMinSize        = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize  = 1.0f;
    style.PopupBorderSize  = 1.0f;
    style.FrameBorderSize  = 1.0f;
    style.TabBorderSize    = 0.0f;

    style.WindowRounding    = 18.0f;
    style.ChildRounding     = 16.0f;
    style.FrameRounding     = 12.0f;
    style.PopupRounding     = 16.0f;
    style.ScrollbarRounding = 999.0f;
    style.GrabRounding      = 999.0f;
    style.TabRounding       = 12.0f;

    style.WindowTitleAlign        = ImVec2(0.0f, 0.5f);
    style.ButtonTextAlign         = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign     = ImVec2(0.0f, 0.5f);
    style.SeparatorTextBorderSize = 0.0f;
    style.SeparatorTextAlign      = ImVec2(0.0f, 0.5f);
    style.SeparatorTextPadding    = ImVec2(0.0f, 10.0f);
}

} // namespace whsim::radix
