#include "render/mesh.hpp"

#include <cstddef>

namespace whsim {

namespace {

void UploadArrayBuffer(const std::vector<Vertex>& vertices)
{
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW
    );
}

void UploadElementBuffer(const std::vector<unsigned int>& indices)
{
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
        indices.data(),
        GL_STATIC_DRAW
    );
}

void ConfigurePositionAttribute()
{
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, position))
    );
}

void ConfigureNormalAttribute()
{
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, normal))
    );
}

void ConfigureTexCoordAttribute()
{
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, tex_coord))
    );
}

void ConfigureVertexAttributes()
{
    ConfigurePositionAttribute();
    ConfigureNormalAttribute();
    ConfigureTexCoordAttribute();
}

} // namespace

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices)
{
    Upload(vertices, indices);
}

Mesh::Mesh(Mesh&& other) noexcept
{
    MoveFrom(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other) {
        Reset();
        MoveFrom(other);
    }

    return *this;
}

Mesh::~Mesh()
{
    Reset();
}

void Mesh::Reset()
{
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
    }

    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }

    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }

    vao_ = 0;
    vbo_ = 0;
    ebo_ = 0;
    index_count_ = 0;
}

void Mesh::MoveFrom(Mesh& other) noexcept
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

void Mesh::Upload(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices)
{
    Reset();

    index_count_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    UploadArrayBuffer(vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    UploadElementBuffer(indices);

    ConfigureVertexAttributes();

    glBindVertexArray(0);
}

void Mesh::Draw() const
{
    if (vao_ == 0 || index_count_ == 0) {
        return;
    }

    glBindVertexArray(vao_);

    glDrawElements(
        GL_TRIANGLES,
        index_count_,
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(0);
}

} // namespace whsim
