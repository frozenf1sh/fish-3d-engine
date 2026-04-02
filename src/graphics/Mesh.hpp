#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "Texture2D.hpp"

namespace fish::graphics {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

struct Material {
    glm::vec3 base_color = {1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 1.0f;
    std::shared_ptr<Texture2D> base_color_texture;
};

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material material);
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    auto operator=(const Mesh&) -> Mesh& = delete;

    Mesh(Mesh&&) noexcept = default;
    auto operator=(Mesh&&) noexcept -> Mesh& = default;

    void draw() const;

    [[nodiscard]] auto get_material() const -> const Material& { return m_material; }
    [[nodiscard]] auto get_vertex_count() const -> size_t { return m_vertices.size(); }
    [[nodiscard]] auto get_index_count() const -> size_t { return m_indices.size(); }

private:
    void setup_mesh();

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    Material m_material;

    Buffer m_vbo;
    Buffer m_ebo;
    VertexArray m_vao;
};

} // namespace fish::graphics
