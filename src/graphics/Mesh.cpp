#include "Mesh.hpp"

#include <glad/glad.h>

namespace fish::graphics {

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material material)
    : m_vertices(std::move(vertices))
    , m_indices(std::move(indices))
    , m_material(std::move(material))
{
    setup_mesh();
}

void Mesh::setup_mesh()
{
    m_vbo = Buffer::create(BufferType::Vertex, m_vertices.data(), m_vertices.size());
    m_ebo = Buffer::create(BufferType::Index, m_indices.data(), m_indices.size());

    constexpr GLsizei stride = sizeof(Vertex);
    m_vao.set_vertex_buffer(0, m_vbo, 0, stride);
    m_vao.set_index_buffer(m_ebo);

    // Position attribute
    m_vao.set_attribute(0, 0, AttributeType::Float, 3, GL_FALSE, offsetof(Vertex, position));
    m_vao.enable_attribute(0);

    // Normal attribute
    m_vao.set_attribute(1, 0, AttributeType::Float, 3, GL_FALSE, offsetof(Vertex, normal));
    m_vao.enable_attribute(1);

    // TexCoord attribute
    m_vao.set_attribute(2, 0, AttributeType::Float, 2, GL_FALSE, offsetof(Vertex, tex_coord));
    m_vao.enable_attribute(2);
}

void Mesh::draw() const
{
    m_vao.bind();

    if (m_material.base_color_texture) {
        m_material.base_color_texture->bind(0);
    }

    // 确保我们有索引数据
    if (!m_indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    } else {
        // 回退到无索引绘制
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    }
}

} // namespace fish::graphics
