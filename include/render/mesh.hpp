#pragma once

#include <cstddef>
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

    void Reset()
    {
        if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);

        vao_ = 0;
        vbo_ = 0;
        ebo_ = 0;
        index_count_ = 0;
    }

    void MoveFrom(Mesh& other) noexcept
    {
        vao_ = other.vao_;
        vbo_ = other.vbo_;
        ebo_ = other.ebo_;
        index_count_ = other.index_count_;

        other.vao_ = 0;
        other.vbo_ = 0;
        other.ebo_ = 0;
        other.index_count_ = 0;
    }

public:
    Mesh() = default;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    {
        Upload(vertices, indices);
    }

    ~Mesh() { Reset(); }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept { MoveFrom(other); }

    Mesh& operator=(Mesh&& other) noexcept
    {
        if (this != &other) {
            Reset();
            MoveFrom(other);
        }
        return *this;
    }

    void Upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    {
        Reset();

        index_count_ = static_cast<GLsizei>(indices.size());

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
            vertices.data(),
            GL_STATIC_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
            indices.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, position))
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, normal))
        );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, tex_coord))
        );

        glBindVertexArray(0);
    }

    void Draw() const
    {
        if (vao_ == 0 || index_count_ == 0) return;

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
};

} // namespace whsim
