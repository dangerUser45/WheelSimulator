#include "render/sim_render.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
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
#include "render/mesh.hpp"

namespace whsim {

namespace {

constexpr float PI = 3.14159265358979323846f;

constexpr glm::vec3 LIGHT_DIRECTION{0.35f, 0.75f, 0.55f};

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
    glUniformMatrix4fv(
        location,
        1,
        GL_FALSE,
        glm::value_ptr(matrix)
    );
}

void SetVec3(GLint location, const glm::vec3& value)
{
    glUniform3fv(
        location,
        1,
        glm::value_ptr(value)
    );
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

Mesh CreateTerrainMesh()
{
    constexpr float half_size = 12.8f;

    std::vector<Vertex> vertices{
        {
            glm::vec3{-half_size, -half_size, 0.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec2{0.0f, 0.0f}
        },
        {
            glm::vec3{half_size, -half_size, 0.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec2{1.0f, 0.0f}
        },
        {
            glm::vec3{half_size, half_size, 0.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec2{1.0f, 1.0f}
        },
        {
            glm::vec3{-half_size, half_size, 0.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec2{0.0f, 1.0f}
        }
    };

    std::vector<unsigned int> indices{
        0, 1, 2,
        0, 2, 3
    };

    return Mesh(vertices, indices);
}

float WheelAngleForSegment(int segment, int segment_count)
{
    return 2.0f * PI *
        static_cast<float>(segment) /
        static_cast<float>(segment_count);
}

glm::vec3 WheelRingPosition(float x, float radius, float angle)
{
    return glm::vec3{
        x,
        std::cos(angle) * radius,
        std::sin(angle) * radius
    };
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

void AddWheelSideVertices(
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

void AddWheelSideIndices(
    std::vector<unsigned int>& indices,
    int segments)
{
    for (int i = 0; i < segments; ++i) {
        const auto left = static_cast<unsigned int>(i * 2);
        const auto right = static_cast<unsigned int>(i * 2 + 1);

        const auto next_left =
            static_cast<unsigned int>(((i + 1) % segments) * 2);

        const auto next_right =
            static_cast<unsigned int>(((i + 1) % segments) * 2 + 1);

        indices.push_back(left);
        indices.push_back(right);
        indices.push_back(next_left);

        indices.push_back(right);
        indices.push_back(next_right);
        indices.push_back(next_left);
    }
}

unsigned int AddWheelCapCenter(
    std::vector<Vertex>& vertices,
    float x,
    const glm::vec3& normal)
{
    const auto center_index = static_cast<unsigned int>(vertices.size());

    vertices.push_back(Vertex{
        glm::vec3{x, 0.0f, 0.0f},
        normal,
        glm::vec2{0.5f, 0.5f}
    });

    return center_index;
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

void AddLeftWheelCapIndices(
    std::vector<unsigned int>& indices,
    unsigned int center,
    unsigned int ring_start,
    int segments)
{
    for (int i = 0; i < segments; ++i) {
        const auto current =
            static_cast<unsigned int>(ring_start + i);

        const auto next =
            static_cast<unsigned int>(ring_start + ((i + 1) % segments));

        indices.push_back(center);
        indices.push_back(next);
        indices.push_back(current);
    }
}

void AddRightWheelCapIndices(
    std::vector<unsigned int>& indices,
    unsigned int center,
    unsigned int ring_start,
    int segments)
{
    for (int i = 0; i < segments; ++i) {
        const auto current =
            static_cast<unsigned int>(ring_start + i);

        const auto next =
            static_cast<unsigned int>(ring_start + ((i + 1) % segments));

        indices.push_back(center);
        indices.push_back(current);
        indices.push_back(next);
    }
}

void AddWheelCaps(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    float radius,
    float half_width,
    int segments)
{
    const unsigned int left_center = AddWheelCapCenter(
        vertices,
        -half_width,
        glm::vec3{-1.0f, 0.0f, 0.0f}
    );

    const unsigned int right_center = AddWheelCapCenter(
        vertices,
        half_width,
        glm::vec3{1.0f, 0.0f, 0.0f}
    );

    const unsigned int left_ring_start = AddWheelCapRing(
        vertices,
        -half_width,
        radius,
        segments,
        glm::vec3{-1.0f, 0.0f, 0.0f}
    );

    const unsigned int right_ring_start = AddWheelCapRing(
        vertices,
        half_width,
        radius,
        segments,
        glm::vec3{1.0f, 0.0f, 0.0f}
    );

    AddLeftWheelCapIndices(
        indices,
        left_center,
        left_ring_start,
        segments
    );

    AddRightWheelCapIndices(
        indices,
        right_center,
        right_ring_start,
        segments
    );
}

Mesh CreateWheelMesh(const WheelSettings& settings)
{
    const float radius = settings.radius;
    const float width = settings.width;
    constexpr int segments = 48;
    const float half_width = width * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(static_cast<std::size_t>(segments * 4 + 2));
    indices.reserve(static_cast<std::size_t>(segments * 12));

    AddWheelSideVertices(vertices, radius, half_width, segments);
    AddWheelSideIndices(indices, segments);
    AddWheelCaps(vertices, indices, radius, half_width, segments);

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

        {glm::vec3{-x, -y,  z}, glm::vec3{0.0f, 0.0f, 1.0f}, {}},
        {glm::vec3{ x, -y,  z}, glm::vec3{0.0f, 0.0f, 1.0f}, {}},
        {glm::vec3{ x,  y,  z}, glm::vec3{0.0f, 0.0f, 1.0f}, {}},
        {glm::vec3{-x,  y,  z}, glm::vec3{0.0f, 0.0f, 1.0f}, {}},

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
    return Mesh(
        CreateBoxVertices(),
        CreateBoxIndices()
    );
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
            return glm::vec3{0.25f, 0.45f, 0.28f};

        case PhysObjKind::Wheel:
            return glm::vec3{0.08f, 0.09f, 0.10f};

        case PhysObjKind::Suspension:
            return glm::vec3{0.78f, 0.72f, 0.62f};

        case PhysObjKind::CarBody:
            return glm::vec3{0.14f, 0.42f, 0.72f};
    }

    return glm::vec3{1.0f};
}

} // namespace

SimRender::SimRender() = default;

SimRender::~SimRender()
{
    terrain_mesh_.reset();
    wheel_mesh_.reset();
    box_mesh_.reset();

    DestroyFramebufferResources();

    if (shader_program_ != 0) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
}

void SimRender::Initialize(const SimulationSettings& settings)
{
    if (initialized_) {
        return;
    }

    InitializeShader();
    InitializeMeshes(settings);
    settings_ = settings;
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
    u_model_location_ =
        GetUniformLocationChecked(shader_program_, "uModel");

    u_view_location_ =
        GetUniformLocationChecked(shader_program_, "uView");

    u_projection_location_ =
        GetUniformLocationChecked(shader_program_, "uProjection");

    u_color_location_ =
        GetUniformLocationChecked(shader_program_, "uColor");

    u_light_dir_location_ =
        GetUniformLocationChecked(shader_program_, "uLightDir");
}

void SimRender::InitializeMeshes(const SimulationSettings& settings)
{
    terrain_mesh_ = std::make_unique<Mesh>(CreateTerrainMesh());
    wheel_mesh_ = std::make_unique<Mesh>(CreateWheelMesh(settings.wheel));
    box_mesh_ = std::make_unique<Mesh>(CreateBoxMesh());
}

void SimRender::RefreshSettings(const SimulationSettings& settings)
{
    camera_.ApplySettings(settings.camera);

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

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );
}

void SimRender::CreateDepthRenderbuffer(int width, int height)
{
    glGenRenderbuffers(1, &depth_renderbuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        width,
        height
    );
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

    glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
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
    int width,
    int height,
    float dt,
    bool camera_input_enabled,
    const SimulationSettings& settings)
{
    if (width <= 0 || height <= 0) {
        return 0;
    }

    Initialize(settings);
    RefreshSettings(settings);
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
    if (!terrain_mesh_ || !wheel_mesh_ || !box_mesh_) {
        return;
    }

    const glm::mat4 projection = camera_.ProjectionMatrix(width, height);
    const glm::mat4 view = camera_.ViewMatrix();

    glUseProgram(shader_program_);

    SetMat4(u_view_location_, view);
    SetMat4(u_projection_location_, projection);
    SetVec3(u_light_dir_location_, LIGHT_DIRECTION);

    for (const PhysObj& object : objects) {
        DrawObject(object);
    }
}

void SimRender::DrawObject(const PhysObj& object) const
{
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

    mesh.Draw();
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
