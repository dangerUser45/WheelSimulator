#pragma once

#include <cstddef>
#include <vector>

#include "config/simulation_settings.hpp"

namespace whsim {

class TerrainGrid final {
private:
    TerrainSettings settings_{};
    std::vector<float> heights_{};
    int origin_cell_x_ = 0;
    int origin_cell_y_ = 0;
    unsigned int revision_ = 0;

private:
    void RebuildHeights();
    [[nodiscard]] float HeightAtCell(int cell_x, int cell_y) const;
    [[nodiscard]] std::size_t Index(int x, int y) const noexcept;

public:
    explicit TerrainGrid(const TerrainSettings& settings);

    [[nodiscard]] bool CenterAround(float world_x, float world_y);

    [[nodiscard]] int SamplesX() const noexcept;
    [[nodiscard]] int SamplesY() const noexcept;
    [[nodiscard]] float CellSize() const noexcept;
    [[nodiscard]] float OriginX() const noexcept;
    [[nodiscard]] float OriginY() const noexcept;
    [[nodiscard]] float CenterX() const noexcept;
    [[nodiscard]] float CenterY() const noexcept;
    [[nodiscard]] unsigned int Revision() const noexcept;
    [[nodiscard]] const std::vector<float>& Heights() const noexcept;
    [[nodiscard]] float* HeightsData() noexcept;
};

} // namespace whsim
