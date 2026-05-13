#pragma once

#include <vector>

#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace whsim {

struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 tex_coord{};
};

class Mesh final {
private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLsizei index_count_ = 0;

private:
    void Reset();
    void MoveFrom(Mesh& other) noexcept;

public:
    Mesh() = default;

    Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);

    Mesh(Mesh&& other) noexcept;
    Mesh(const Mesh&) = delete;
    ~Mesh();

    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&& other) noexcept;

    void Upload(
        const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);

    void Draw() const;
    void Draw(GLenum primitive) const;
};

} // namespace whsim
