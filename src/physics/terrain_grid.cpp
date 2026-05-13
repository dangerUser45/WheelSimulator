#include "physics/terrain_grid.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace whsim {

namespace {

constexpr float HASH_SCALE = 43758.5453123f;
constexpr float HASH_X = 12.9898f;
constexpr float HASH_Y = 78.233f;
constexpr float SECOND_OCTAVE_SCALE = 2.1f;
constexpr float THIRD_OCTAVE_SCALE = 4.3f;
constexpr float SECOND_OCTAVE_WEIGHT = 0.5f;
constexpr float THIRD_OCTAVE_WEIGHT = 0.25f;
constexpr float TOTAL_WEIGHT =
    1.0f + SECOND_OCTAVE_WEIGHT + THIRD_OCTAVE_WEIGHT;

float Fract(float value)
{
    return value - std::floor(value);
}

float Smooth(float value)
{
    return value * value * (3.0f - 2.0f * value);
}

float Lerp(float lhs, float rhs, float amount)
{
    return lhs + (rhs - lhs) * amount;
}

float Hash(int x, int y)
{
    const float value =
        std::sin(static_cast<float>(x) * HASH_X +
                 static_cast<float>(y) * HASH_Y) *
        HASH_SCALE;

    return Fract(value) * 2.0f - 1.0f;
}

float ValueNoise(float x, float y)
{
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;

    const float tx = Smooth(Fract(x));
    const float ty = Smooth(Fract(y));

    const float a = Hash(x0, y0);
    const float b = Hash(x1, y0);
    const float c = Hash(x0, y1);
    const float d = Hash(x1, y1);

    return Lerp(Lerp(a, b, tx), Lerp(c, d, tx), ty);
}

int ClampedSamples(int samples)
{
    constexpr int min_samples = 16;
    return std::max(min_samples, samples);
}

} // namespace

TerrainGrid::TerrainGrid(const TerrainSettings& settings) :
    settings_(settings),
    heights_{},
    origin_cell_x_(-ClampedSamples(settings.samples_x) / 2),
    origin_cell_y_(-ClampedSamples(settings.samples_y) / 2),
    revision_(0)
{
    heights_.resize(
        static_cast<std::size_t>(SamplesX()) *
        static_cast<std::size_t>(SamplesY()));

    RebuildHeights();
}

bool TerrainGrid::CenterAround(float world_x, float world_y)
{
    const int center_cell_x =
        static_cast<int>(std::floor(world_x / settings_.cell_size));

    const int center_cell_y =
        static_cast<int>(std::floor(world_y / settings_.cell_size));

    const int next_origin_x = center_cell_x - SamplesX() / 2;
    const int next_origin_y = center_cell_y - SamplesY() / 2;

    if (next_origin_x == origin_cell_x_ &&
        next_origin_y == origin_cell_y_) {
        return false;
    }

    origin_cell_x_ = next_origin_x;
    origin_cell_y_ = next_origin_y;

    RebuildHeights();
    return true;
}

int TerrainGrid::SamplesX() const noexcept
{
    return ClampedSamples(settings_.samples_x);
}

int TerrainGrid::SamplesY() const noexcept
{
    return ClampedSamples(settings_.samples_y);
}

float TerrainGrid::CellSize() const noexcept
{
    return settings_.cell_size;
}

float TerrainGrid::OriginX() const noexcept
{
    return static_cast<float>(origin_cell_x_) * settings_.cell_size;
}

float TerrainGrid::OriginY() const noexcept
{
    return static_cast<float>(origin_cell_y_) * settings_.cell_size;
}

float TerrainGrid::CenterX() const noexcept
{
    return OriginX() +
        static_cast<float>(SamplesX() - 1) * settings_.cell_size * 0.5f;
}

float TerrainGrid::CenterY() const noexcept
{
    return OriginY() +
        static_cast<float>(SamplesY() - 1) * settings_.cell_size * 0.5f;
}

unsigned int TerrainGrid::Revision() const noexcept
{
    return revision_;
}

const std::vector<float>& TerrainGrid::Heights() const noexcept
{
    return heights_;
}

float* TerrainGrid::HeightsData() noexcept
{
    return heights_.data();
}

void TerrainGrid::RebuildHeights()
{
    for (int y = 0; y < SamplesY(); ++y) {
        for (int x = 0; x < SamplesX(); ++x) {
            heights_[Index(x, y)] =
                HeightAtCell(origin_cell_x_ + x, origin_cell_y_ + y);
        }
    }

    ++revision_;
}

float TerrainGrid::HeightAtCell(int cell_x, int cell_y) const
{
    const float x =
        static_cast<float>(cell_x) * settings_.cell_size *
        settings_.noise_frequency;

    const float y =
        static_cast<float>(cell_y) * settings_.cell_size *
        settings_.noise_frequency;

    const float noise =
        ValueNoise(x, y) +
        ValueNoise(x * SECOND_OCTAVE_SCALE, y * SECOND_OCTAVE_SCALE) *
            SECOND_OCTAVE_WEIGHT +
        ValueNoise(x * THIRD_OCTAVE_SCALE, y * THIRD_OCTAVE_SCALE) *
            THIRD_OCTAVE_WEIGHT;

    const float height =
        settings_.height_amplitude * noise / TOTAL_WEIGHT;

    return std::clamp(height, settings_.min_height, settings_.max_height);
}

std::size_t TerrainGrid::Index(int x, int y) const noexcept
{
    return static_cast<std::size_t>(y) *
        static_cast<std::size_t>(SamplesX()) +
        static_cast<std::size_t>(x);
}

} // namespace whsim
