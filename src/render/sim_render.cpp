#include "render/sim_render.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "physics/phys_obj_view.hpp"
#include "physics/terrain_grid.hpp"
#include "render/mesh.hpp"

namespace whsim {

namespace {

constexpr float PI = 3.14159265358979323846f;

constexpr glm::vec3 LIGHT_DIRECTION{0.25f, 0.45f, 0.88f};
constexpr glm::vec3 SKY_COLOR{0.45f, 0.78f, 1.0f};

constexpr glm::vec3 TERRAIN_COLOR{0.38f, 0.39f, 0.40f};
constexpr glm::vec3 TERRAIN_GRID_COLOR{0.74f, 0.75f, 0.76f};
constexpr glm::vec3 WHEEL_COLOR{0.95f, 0.28f, 0.08f};
constexpr glm::vec3 SUSPENSION_COLOR{0.95f, 0.82f, 0.28f};
constexpr glm::vec3 CAR_BODY_COLOR{0.06f, 0.36f, 0.95f};

constexpr float TERRAIN_GRID_Z_OFFSET = 0.01f;
constexpr float WHEEL_HUB_RADIUS_FACTOR = 0.22f;
constexpr float WHEEL_RIM_INNER_RADIUS_FACTOR = 0.76f;
constexpr float WHEEL_SPOKE_HALF_WIDTH_FACTOR = 0.035f;
constexpr int WHEEL_SEGMENTS = 64;
constexpr int WHEEL_SPOKES = 8;

std::string ReadTextFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string ShaderTypeName(GLenum type)
{
    switch (type) {
    case GL_VERTEX_SHADER:
        return "vertex";
    case GL_FRAGMENT_SHADER:
        return "fragment";
    default:
        return "unknown";
    }
}

std::string GetShaderInfoLog(GLuint shader)
{
    GLint log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    std::string log(
        static_cast<std::size_t>(std::max(log_length, 1)),
        '\0'
    );
    glGetShaderInfoLog(shader, log_length, nullptr, log.data());
    return log;
}

std::string GetProgramInfoLog(GLuint program)
{
    GLint log_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

    std::string log(
        static_cast<std::size_t>(std::max(log_length, 1)),
        '\0'
    );
    glGetProgramInfoLog(program, log_length, nullptr, log.data());
    return log;
}

GLuint CompileShader(GLenum type, const std::string& source)
{
    const GLuint shader = glCreateShader(type);
    const char* source_ptr = source.c_str();

    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_TRUE) {
        return shader;
    }

    const std::string log = GetShaderInfoLog(shader);
    glDeleteShader(shader);

    throw std::runtime_error(
        "Failed to compile " + ShaderTypeName(type) + " shader: " + log
    );
}

void CheckProgramLinkStatus(GLuint program)
{
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_TRUE) {
        return;
    }

    const std::string log = GetProgramInfoLog(program);
    glDeleteProgram(program);
    throw std::runtime_error("Failed to link simulation shader program: " + log);
}

GLuint LinkShaderProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    const GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    CheckProgramLinkStatus(program);
    return program;
}

GLuint CreateShaderProgramFromFiles(
    const std::string& vertex_shader_path,
    const std::string& fragment_shader_path)
{
    const std::string vertex_source = ReadTextFile(vertex_shader_path);
    const std::string fragment_source = ReadTextFile(fragment_shader_path);

    const GLuint vertex_shader =
        CompileShader(GL_VERTEX_SHADER, vertex_source);
    const GLuint fragment_shader =
        CompileShader(GL_FRAGMENT_SHADER, fragment_source);

    return LinkShaderProgram(vertex_shader, fragment_shader);
}

GLint GetUniformLocationChecked(GLuint program, const char* name)
{
    const GLint location = glGetUniformLocation(program, name);
    if (location == -1) {
        throw std::runtime_error(
            std::string("Uniform not found or optimized out: ") + name
        );
    }

    return location;
}

void SetMat4(GLint location, const glm::mat4& matrix)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void SetVec3(GLint location, const glm::vec3& value)
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void SetFloat(GLint location, float value)
{
    glUniform1f(location, value);
}

glm::mat4 MatrixFromPhysObj(const PhysObj& object)
{
    glm::mat4 matrix{1.0f};

    for (std::size_t column = 0; column < 4; ++column) {
        for (std::size_t row = 0; row < 4; ++row) {
            matrix[column][row] =
                static_cast<float>(object.obj_matrix[column * 4 + row]);
        }
    }

    return matrix;
}

glm::vec3 TerrainPosition(const TerrainGrid& terrain, int x, int y, float z_offset)
{
    const auto height_index =
        static_cast<std::size_t>(y) * static_cast<std::size_t>(terrain.SamplesX()) +
        static_cast<std::size_t>(x);

    const float world_x =
        terrain.OriginX() + static_cast<float>(x) * terrain.CellSize();
    const float world_y =
        terrain.OriginY() + static_cast<float>(y) * terrain.CellSize();

    return glm::vec3{
        world_x - terrain.CenterX(),
        world_y - terrain.CenterY(),
        terrain.Heights()[height_index] + z_offset
    };
}

glm::vec3 TerrainNormal(const TerrainGrid& terrain, int x, int y)
{
    const int left = std::max(0, x - 1);
    const int right = std::min(terrain.SamplesX() - 1, x + 1);
    const int down = std::max(0, y - 1);
    const int up = std::min(terrain.SamplesY() - 1, y + 1);

    const glm::vec3 dx =
        TerrainPosition(terrain, right, y, 0.0f) -
        TerrainPosition(terrain, left, y, 0.0f);
    const glm::vec3 dy =
        TerrainPosition(terrain, x, up, 0.0f) -
        TerrainPosition(terrain, x, down, 0.0f);

    return glm::normalize(glm::cross(dx, dy));
}

Mesh CreateTerrainMesh(const TerrainGrid& terrain)
{
    std::vector<Vertex> vertices;
    vertices.reserve(
        static_cast<std::size_t>(terrain.SamplesX()) *
        static_cast<std::size_t>(terrain.SamplesY())
    );

    for (int y = 0; y < terrain.SamplesY(); ++y) {
        for (int x = 0; x < terrain.SamplesX(); ++x) {
            vertices.push_back(Vertex{
                TerrainPosition(terrain, x, y, 0.0f),
                TerrainNormal(terrain, x, y),
                glm::vec2{
                    static_cast<float>(x) / static_cast<float>(terrain.SamplesX() - 1),
                    static_cast<float>(y) / static_cast<float>(terrain.SamplesY() - 1)
                }
            });
        }
    }

    std::vector<unsigned int> indices;
    indices.reserve(
        static_cast<std::size_t>(terrain.SamplesX() - 1) *
        static_cast<std::size_t>(terrain.SamplesY() - 1) * 6U
    );

    for (int y = 0; y < terrain.SamplesY() - 1; ++y) {
        for (int x = 0; x < terrain.SamplesX() - 1; ++x) {
            const auto current = static_cast<unsigned int>(y * terrain.SamplesX() + x);
            const auto right = current + 1U;
            const auto up = static_cast<unsigned int>((y + 1) * terrain.SamplesX() + x);
            const auto up_right = up + 1U;

            indices.push_back(current);
            indices.push_back(right);
            indices.push_back(up);
            indices.push_back(right);
            indices.push_back(up_right);
            indices.push_back(up);
        }
    }

    return Mesh(vertices, indices);
}

Mesh CreateTerrainGridMesh(const TerrainGrid& terrain)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(
        static_cast<std::size_t>(terrain.SamplesX()) *
        static_cast<std::size_t>(terrain.SamplesY())
    );

    for (int y = 0; y < terrain.SamplesY(); ++y) {
        for (int x = 0; x < terrain.SamplesX(); ++x) {
            vertices.push_back(Vertex{
                TerrainPosition(terrain, x, y, TERRAIN_GRID_Z_OFFSET),
                TerrainNormal(terrain, x, y),
                glm::vec2{}
            });
        }
    }

    for (int y = 0; y < terrain.SamplesY(); ++y) {
        for (int x = 0; x < terrain.SamplesX() - 1; ++x) {
            const auto current = static_cast<unsigned int>(y * terrain.SamplesX() + x);
            indices.push_back(current);
            indices.push_back(current + 1U);
        }
    }

    for (int x = 0; x < terrain.SamplesX(); ++x) {
        for (int y = 0; y < terrain.SamplesY() - 1; ++y) {
            const auto current = static_cast<unsigned int>(y * terrain.SamplesX() + x);
            indices.push_back(current);
            indices.push_back(current + static_cast<unsigned int>(terrain.SamplesX()));
        }
    }

    return Mesh(vertices, indices);
}

float WheelAngleForSegment(int segment, int segment_count)
{
    return 2.0f * PI * static_cast<float>(segment) /
        static_cast<float>(segment_count);
}

glm::vec3 WheelRingPosition(float x, float radius, float angle)
{
    return glm::vec3{x, std::cos(angle) * radius, std::sin(angle) * radius};
}

glm::vec3 WheelSideNormal(float radius, float angle)
{
    const float y = std::cos(angle) * radius;
    const float z = std::sin(angle) * radius;
    return glm::normalize(glm::vec3{0.0f, y, z});
}

glm::vec2 WheelSideTexCoord(float u, int segment, int segment_count)
{
    return glm::vec2{
        u,
        static_cast<float>(segment) / static_cast<float>(segment_count)
    };
}

glm::vec2 WheelCapTexCoord(float y, float z, float radius)
{
    return glm::vec2{
        y / radius * 0.5f + 0.5f,
        z / radius * 0.5f + 0.5f
    };
}

void AddWheelOuterSideVertices(
    std::vector<Vertex>& vertices,
    float radius,
    float half_width,
    int segments)
{
    for (int i = 0; i < segments; ++i) {
        const float angle = WheelAngleForSegment(i, segments);
        const glm::vec3 normal = WheelSideNormal(radius, angle);

        vertices.push_back(Vertex{
            WheelRingPosition(-half_width, radius, angle),
            normal,
            WheelSideTexCoord(0.0f, i, segments)
        });
        vertices.push_back(Vertex{
            WheelRingPosition(half_width, radius, angle),
            normal,
            WheelSideTexCoord(1.0f, i, segments)
        });
    }
}

void AddWheelOuterSideIndices(std::vector<unsigned int>& indices, int segments)
{
    for (int i = 0; i < segments; ++i) {
        const auto left = static_cast<unsigned int>(i * 2);
        const auto right = static_cast<unsigned int>(i * 2 + 1);
        const auto next_left = static_cast<unsigned int>(((i + 1) % segments) * 2);
        const auto next_right = static_cast<unsigned int>(((i + 1) % segments) * 2 + 1);

        indices.push_back(left);
        indices.push_back(right);
        indices.push_back(next_left);
        indices.push_back(right);
        indices.push_back(next_right);
        indices.push_back(next_left);
    }
}

unsigned int AddWheelCapRing(
    std::vector<Vertex>& vertices,
    float x,
    float radius,
    int segments,
    const glm::vec3& normal)
{
    const auto ring_start = static_cast<unsigned int>(vertices.size());

    for (int i = 0; i < segments; ++i) {
        const float angle = WheelAngleForSegment(i, segments);
        const float y = std::cos(angle) * radius;
        const float z = std::sin(angle) * radius;

        vertices.push_back(Vertex{
            glm::vec3{x, y, z},
            normal,
            WheelCapTexCoord(y, z, radius)
        });
    }

    return ring_start;
}

void AddWheelAnnulusIndices(
    std::vector<unsigned int>& indices,
    unsigned int outer_start,
    unsigned int inner_start,
    int segments,
    bool left_side)
{
    for (int i = 0; i < segments; ++i) {
        const auto outer_current = static_cast<unsigned int>(outer_start + i);
        const auto outer_next = static_cast<unsigned int>(outer_start + ((i + 1) % segments));
        const auto inner_current = static_cast<unsigned int>(inner_start + i);
        const auto inner_next = static_cast<unsigned int>(inner_start + ((i + 1) % segments));

        if (left_side) {
            indices.push_back(outer_current);
            indices.push_back(outer_next);
            indices.push_back(inner_current);
            indices.push_back(outer_next);
            indices.push_back(inner_next);
            indices.push_back(inner_current);
        } else {
            indices.push_back(outer_current);
            indices.push_back(inner_current);
            indices.push_back(outer_next);
            indices.push_back(outer_next);
            indices.push_back(inner_current);
            indices.push_back(inner_next);
        }
    }
}

void AddWheelDisc(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    float x,
    float radius,
    int segments,
    const glm::vec3& normal,
    bool left_side)
{
    const float inner_radius = radius * WHEEL_RIM_INNER_RADIUS_FACTOR;
    const unsigned int outer_start = AddWheelCapRing(vertices, x, radius, segments, normal);
    const unsigned int inner_start = AddWheelCapRing(vertices, x, inner_radius, segments, normal);

    AddWheelAnnulusIndices(indices, outer_start, inner_start, segments, left_side);
}

void AddWheelHub(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    float half_width,
    float radius,
    int segments)
{
    const float hub_radius = radius * WHEEL_HUB_RADIUS_FACTOR;
    const unsigned int left_start = AddWheelCapRing(
        vertices,
        -half_width,
        hub_radius,
        segments,
        glm::vec3{-1.0f, 0.0f, 0.0f}
    );
    const unsigned int right_start = AddWheelCapRing(
        vertices,
        half_width,
        hub_radius,
        segments,
        glm::vec3{1.0f, 0.0f, 0.0f}
    );

    AddWheelAnnulusIndices(indices, left_start, right_start, segments, false);
}

void AddQuad(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& d,
    const glm::vec3& normal)
{
    const auto start = static_cast<unsigned int>(vertices.size());

    vertices.push_back(Vertex{a, normal, glm::vec2{0.0f, 0.0f}});
    vertices.push_back(Vertex{b, normal, glm::vec2{1.0f, 0.0f}});
    vertices.push_back(Vertex{c, normal, glm::vec2{1.0f, 1.0f}});
    vertices.push_back(Vertex{d, normal, glm::vec2{0.0f, 1.0f}});

    indices.push_back(start);
    indices.push_back(start + 1U);
    indices.push_back(start + 2U);
    indices.push_back(start);
    indices.push_back(start + 2U);
    indices.push_back(start + 3U);
}

void AddWheelSpoke(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    float x,
    float radius,
    float angle,
    const glm::vec3& normal)
{
    const float hub_radius = radius * WHEEL_HUB_RADIUS_FACTOR;
    const float rim_radius = radius * WHEEL_RIM_INNER_RADIUS_FACTOR;
    const float half_spoke_width = radius * WHEEL_SPOKE_HALF_WIDTH_FACTOR;

    const glm::vec3 radial{0.0f, std::cos(angle), std::sin(angle)};
    const glm::vec3 tangent{0.0f, -std::sin(angle), std::cos(angle)};

    const glm::vec3 inner_a = glm::vec3{x, 0.0f, 0.0f} + radial * hub_radius - tangent * half_spoke_width;
    const glm::vec3 inner_b = glm::vec3{x, 0.0f, 0.0f} + radial * hub_radius + tangent * half_spoke_width;
    const glm::vec3 outer_a = glm::vec3{x, 0.0f, 0.0f} + radial * rim_radius - tangent * half_spoke_width;
    const glm::vec3 outer_b = glm::vec3{x, 0.0f, 0.0f} + radial * rim_radius + tangent * half_spoke_width;

    AddQuad(vertices, indices, inner_a, outer_a, outer_b, inner_b, normal);
}

void AddWheelSpokes(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    float half_width,
    float radius)
{
    for (int i = 0; i < WHEEL_SPOKES; ++i) {
        const float angle = WheelAngleForSegment(i, WHEEL_SPOKES);
        AddWheelSpoke(
            vertices,
            indices,
            -half_width,
            radius,
            angle,
            glm::vec3{-1.0f, 0.0f, 0.0f}
        );
        AddWheelSpoke(
            vertices,
            indices,
            half_width,
            radius,
            angle,
            glm::vec3{1.0f, 0.0f, 0.0f}
        );
    }
}

Mesh CreateWheelMesh(const WheelSettings& settings)
{
    const float radius = settings.radius;
    const float half_width = settings.width * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(static_cast<std::size_t>(WHEEL_SEGMENTS * 8));
    indices.reserve(static_cast<std::size_t>(WHEEL_SEGMENTS * 24));

    AddWheelOuterSideVertices(vertices, radius, half_width, WHEEL_SEGMENTS);
    AddWheelOuterSideIndices(indices, WHEEL_SEGMENTS);

    AddWheelDisc(
        vertices,
        indices,
        -half_width,
        radius,
        WHEEL_SEGMENTS,
        glm::vec3{-1.0f, 0.0f, 0.0f},
        true
    );
    AddWheelDisc(
        vertices,
        indices,
        half_width,
        radius,
        WHEEL_SEGMENTS,
        glm::vec3{1.0f, 0.0f, 0.0f},
        false
    );

    AddWheelHub(vertices, indices, half_width, radius, WHEEL_SEGMENTS);
    AddWheelSpokes(vertices, indices, half_width, radius);

    return Mesh(vertices, indices);
}

std::vector<Vertex> CreateBoxVertices()
{
    constexpr float x = 0.8f;
    constexpr float y = 0.45f;
    constexpr float z = 0.2f;

    return {
        {glm::vec3{-x, -y, -z}, glm::vec3{0.0f, 0.0f, -1.0f}, {}},
        {glm::vec3{ x, -y, -z}, glm::vec3{0.0f, 0.0f, -1.0f}, {}},
        {glm::vec3{ x,  y, -z}, glm::vec3{0.0f, 0.0f, -1.0f}, {}},
        {glm::vec3{-x,  y, -z}, glm::vec3{0.0f, 0.0f, -1.0f}, {}},
        {glm::vec3{-x, -y,  z}, glm::vec3{0.0f, 0.0f,  1.0f}, {}},
        {glm::vec3{ x, -y,  z}, glm::vec3{0.0f, 0.0f,  1.0f}, {}},
        {glm::vec3{ x,  y,  z}, glm::vec3{0.0f, 0.0f,  1.0f}, {}},
        {glm::vec3{-x,  y,  z}, glm::vec3{0.0f, 0.0f,  1.0f}, {}},
        {glm::vec3{-x, -y, -z}, glm::vec3{-1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{-x,  y, -z}, glm::vec3{-1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{-x,  y,  z}, glm::vec3{-1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{-x, -y,  z}, glm::vec3{-1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{ x, -y, -z}, glm::vec3{1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{ x,  y, -z}, glm::vec3{1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{ x,  y,  z}, glm::vec3{1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{ x, -y,  z}, glm::vec3{1.0f, 0.0f, 0.0f}, {}},
        {glm::vec3{-x, -y, -z}, glm::vec3{0.0f, -1.0f, 0.0f}, {}},
        {glm::vec3{ x, -y, -z}, glm::vec3{0.0f, -1.0f, 0.0f}, {}},
        {glm::vec3{ x, -y,  z}, glm::vec3{0.0f, -1.0f, 0.0f}, {}},
        {glm::vec3{-x, -y,  z}, glm::vec3{0.0f, -1.0f, 0.0f}, {}},
        {glm::vec3{-x,  y, -z}, glm::vec3{0.0f, 1.0f, 0.0f}, {}},
        {glm::vec3{ x,  y, -z}, glm::vec3{0.0f, 1.0f, 0.0f}, {}},
        {glm::vec3{ x,  y,  z}, glm::vec3{0.0f, 1.0f, 0.0f}, {}},
        {glm::vec3{-x,  y,  z}, glm::vec3{0.0f, 1.0f, 0.0f}, {}}
    };
}

std::vector<unsigned int> CreateBoxIndices()
{
    return {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        8, 9, 10, 8, 10, 11,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 21, 22, 20, 22, 23
    };
}

Mesh CreateBoxMesh()
{
    return Mesh(CreateBoxVertices(), CreateBoxIndices());
}

const Mesh& MeshForKind(
    PhysObjKind kind,
    const Mesh& terrain,
    const Mesh& wheel,
    const Mesh& box)
{
    switch (kind) {
    case PhysObjKind::Terrain:
        return terrain;
    case PhysObjKind::Wheel:
        return wheel;
    case PhysObjKind::Suspension:
    case PhysObjKind::CarBody:
        return box;
    }

    return box;
}

glm::vec3 ColorForKind(PhysObjKind kind)
{
    switch (kind) {
    case PhysObjKind::Terrain:
        return TERRAIN_COLOR;
    case PhysObjKind::Wheel:
        return WHEEL_COLOR;
    case PhysObjKind::Suspension:
        return SUSPENSION_COLOR;
    case PhysObjKind::CarBody:
        return CAR_BODY_COLOR;
    }

    return glm::vec3{1.0f};
}

float SpecularStrengthForKind(PhysObjKind kind)
{
    switch (kind) {
    case PhysObjKind::Terrain:
        return 0.12f;
    case PhysObjKind::Wheel:
        return 0.48f;
    case PhysObjKind::Suspension:
        return 0.35f;
    case PhysObjKind::CarBody:
        return 0.42f;
    }

    return 0.2f;
}

float ShininessForKind(PhysObjKind kind)
{
    switch (kind) {
    case PhysObjKind::Terrain:
        return 18.0f;
    case PhysObjKind::Wheel:
        return 36.0f;
    case PhysObjKind::Suspension:
        return 28.0f;
    case PhysObjKind::CarBody:
        return 40.0f;
    }

    return 24.0f;
}

} // namespace

SimRender::SimRender() = default;

SimRender::~SimRender()
{
    terrain_grid_mesh_.reset();
    terrain_mesh_.reset();
    wheel_mesh_.reset();
    box_mesh_.reset();

    DestroyFramebufferResources();

    if (shader_program_ != 0) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
}

void SimRender::Initialize(
    const SimulationSettings& settings,
    const TerrainGrid& terrain_grid)
{
    if (initialized_) {
        return;
    }

    InitializeShader();
    InitializeMeshes(settings, terrain_grid);

    settings_ = settings;
    terrain_revision_ = terrain_grid.Revision();
    camera_.ApplySettings(settings_.camera);
    initialized_ = true;
}

void SimRender::InitializeShader()
{
    std::string vertex_shader_path = std::string(SHADERS_PATH) + "vertex.glsl";
    std::string fragment_shader_path = std::string(SHADERS_PATH) + "fragment.glsl";

    shader_program_ = CreateShaderProgramFromFiles(
        vertex_shader_path,
        fragment_shader_path
    );

    InitializeUniformLocations();
}

void SimRender::InitializeUniformLocations()
{
    u_model_location_ = GetUniformLocationChecked(shader_program_, "uModel");
    u_view_location_ = GetUniformLocationChecked(shader_program_, "uView");
    u_projection_location_ = GetUniformLocationChecked(shader_program_, "uProjection");
    u_color_location_ = GetUniformLocationChecked(shader_program_, "uColor");
    u_light_dir_location_ = GetUniformLocationChecked(shader_program_, "uLightDir");
    u_view_pos_location_ = GetUniformLocationChecked(shader_program_, "uViewPos");
    u_specular_strength_location_ = GetUniformLocationChecked(shader_program_, "uSpecularStrength");
    u_shininess_location_ = GetUniformLocationChecked(shader_program_, "uShininess");
}

void SimRender::InitializeMeshes(
    const SimulationSettings& settings,
    const TerrainGrid& terrain_grid)
{
    terrain_mesh_ = std::make_unique<Mesh>(CreateTerrainMesh(terrain_grid));
    terrain_grid_mesh_ = std::make_unique<Mesh>(CreateTerrainGridMesh(terrain_grid));
    wheel_mesh_ = std::make_unique<Mesh>(CreateWheelMesh(settings.wheel));
    box_mesh_ = std::make_unique<Mesh>(CreateBoxMesh());
}

void SimRender::RefreshSettings(
    const SimulationSettings& settings,
    const TerrainGrid& terrain_grid)
{
    camera_.ApplySettings(settings.camera);

    if (terrain_revision_ != terrain_grid.Revision()) {
        terrain_mesh_ = std::make_unique<Mesh>(CreateTerrainMesh(terrain_grid));
        terrain_grid_mesh_ = std::make_unique<Mesh>(CreateTerrainGridMesh(terrain_grid));
        terrain_revision_ = terrain_grid.Revision();
    }

    if (WheelMeshSettingsChanged(settings_, settings)) {
        wheel_mesh_ = std::make_unique<Mesh>(CreateWheelMesh(settings.wheel));
    }

    settings_ = settings;
}

void SimRender::DestroyFramebufferResources()
{
    if (depth_renderbuffer_ != 0) {
        glDeleteRenderbuffers(1, &depth_renderbuffer_);
        depth_renderbuffer_ = 0;
    }

    if (sim_texture_ != 0) {
        glDeleteTextures(1, &sim_texture_);
        sim_texture_ = 0;
    }

    if (framebuffer_ != 0) {
        glDeleteFramebuffers(1, &framebuffer_);
        framebuffer_ = 0;
    }

    texture_width_ = 0;
    texture_height_ = 0;
}

void SimRender::EnsureFramebuffer(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    const bool framebuffer_is_valid =
        framebuffer_ != 0 &&
        sim_texture_ != 0 &&
        depth_renderbuffer_ != 0 &&
        texture_width_ == width &&
        texture_height_ == height;

    if (framebuffer_is_valid) {
        return;
    }

    DestroyFramebufferResources();
    CreateFramebufferResources(width, height);
}

void SimRender::CreateFramebufferResources(int width, int height)
{
    texture_width_ = width;
    texture_height_ = height;

    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

    CreateColorTexture(width, height);
    CreateDepthRenderbuffer(width, height);
    AttachFramebufferResources();
    ValidateFramebuffer();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SimRender::CreateColorTexture(int width, int height)
{
    glGenTextures(1, &sim_texture_);
    glBindTexture(GL_TEXTURE_2D, sim_texture_);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void SimRender::CreateDepthRenderbuffer(int width, int height)
{
    glGenRenderbuffers(1, &depth_renderbuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

void SimRender::AttachFramebufferResources()
{
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        sim_texture_,
        0
    );
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        depth_renderbuffer_
    );
}

void SimRender::ValidateFramebuffer() const
{
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    throw std::runtime_error("Failed to create simulation framebuffer");
}

void SimRender::BeginRenderToTexture(int width, int height) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SimRender::EndRenderToTexture() const
{
    glUseProgram(0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int SimRender::Render(
    GLFWwindow* window,
    const std::vector<PhysObj>& objects,
    const TerrainGrid& terrain_grid,
    int width,
    int height,
    float dt,
    bool camera_input_enabled,
    const SimulationSettings& settings)
{
    if (width <= 0 || height <= 0) {
        return 0;
    }

    Initialize(settings, terrain_grid);
    RefreshSettings(settings, terrain_grid);
    camera_.Update(window, objects, dt, camera_input_enabled);
    EnsureFramebuffer(width, height);

    BeginRenderToTexture(width, height);
    DrawObjects(objects, width, height);
    EndRenderToTexture();

    return sim_texture_;
}

void SimRender::DrawObjects(
    const std::vector<PhysObj>& objects,
    int width,
    int height) const
{
    if (!terrain_mesh_ || !terrain_grid_mesh_ || !wheel_mesh_ || !box_mesh_) {
        return;
    }

    const glm::mat4 projection = camera_.ProjectionMatrix(width, height);
    const glm::mat4 view = camera_.ViewMatrix();

    glUseProgram(shader_program_);
    SetMat4(u_view_location_, view);
    SetMat4(u_projection_location_, projection);
    SetVec3(u_light_dir_location_, LIGHT_DIRECTION);

    const glm::mat4 inverse_view = glm::inverse(view);
    SetVec3(u_view_pos_location_, glm::vec3{inverse_view[3]});

    for (const PhysObj& object : objects) {
        DrawObject(object);
    }
}

void SimRender::DrawObject(const PhysObj& object) const
{
    if (object.kind == PhysObjKind::Terrain) {
        DrawTerrain(object);
        return;
    }

    const Mesh& mesh = MeshForKind(
        object.kind,
        *terrain_mesh_,
        *wheel_mesh_,
        *box_mesh_
    );

    const glm::mat4 model = MatrixFromPhysObj(object);
    const glm::vec3 color = ColorForKind(object.kind);

    SetMat4(u_model_location_, model);
    SetVec3(u_color_location_, color);
    SetFloat(u_specular_strength_location_, SpecularStrengthForKind(object.kind));
    SetFloat(u_shininess_location_, ShininessForKind(object.kind));

    mesh.Draw();
}

void SimRender::DrawTerrain(const PhysObj& object) const
{
    const glm::mat4 model = MatrixFromPhysObj(object);

    SetMat4(u_model_location_, model);
    SetVec3(u_color_location_, TERRAIN_COLOR);
    SetFloat(u_specular_strength_location_, SpecularStrengthForKind(PhysObjKind::Terrain));
    SetFloat(u_shininess_location_, ShininessForKind(PhysObjKind::Terrain));
    terrain_mesh_->Draw();

    glLineWidth(1.4f);
    SetVec3(u_color_location_, TERRAIN_GRID_COLOR);
    SetFloat(u_specular_strength_location_, 0.0f);
    SetFloat(u_shininess_location_, 1.0f);
    terrain_grid_mesh_->Draw(GL_LINES);
    glLineWidth(1.0f);
}

unsigned int SimRender::SimTexture() const noexcept
{
    return sim_texture_;
}

CameraMode SimRender::CameraViewMode() const noexcept
{
    return camera_.Mode();
}

} // namespace whsim
