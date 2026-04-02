#include "Model.hpp"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <filesystem>

namespace fish::graphics {

namespace {

auto load_textures(std::string_view directory, const tinygltf::Model& gltf_model)
    -> std::unordered_map<int, std::shared_ptr<Texture2D>>
{
    std::unordered_map<int, std::shared_ptr<Texture2D>> textures;

    for (size_t i = 0; i < gltf_model.textures.size(); ++i) {
        const auto& texture = gltf_model.textures[i];
        const auto& image = gltf_model.images[texture.source];

        std::filesystem::path tex_path = std::filesystem::path(directory) / image.uri;

        auto tex_result = Texture2D::from_file(tex_path.string(), true);
        if (tex_result) {
            textures[static_cast<int>(i)] = std::make_shared<Texture2D>(std::move(*tex_result));
        } else {
            std::fprintf(stderr, "Warning: Failed to load texture %s: %s\n",
                        tex_path.c_str(), tex_result.error().c_str());
        }
    }

    return textures;
}

} // anonymous namespace

auto Model::from_file(std::string_view path)
    -> std::expected<Model, std::string>
{
    Model model;

    std::filesystem::path model_path(path);
    model.m_directory = model_path.parent_path();

    tinygltf::TinyGLTF loader;
    tinygltf::Model gltf_model;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, path.data());

    if (!warn.empty()) {
        std::fprintf(stderr, "Warning: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        return std::unexpected(std::string("Error loading glTF: ") + err);
    }

    if (!ret) {
        return std::unexpected("Failed to load glTF model");
    }

    // Load textures first
    model.m_textures = load_textures(model.m_directory.string(), gltf_model);

    // Process each mesh
    for (const auto& gltf_mesh : gltf_model.meshes) {
        for (const auto& primitive : gltf_mesh.primitives) {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            Material material;

            // Get positions
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const auto& accessor = gltf_model.accessors[primitive.attributes.at("POSITION")];
                const auto& buffer_view = gltf_model.bufferViews[accessor.bufferView];
                const auto& buffer = gltf_model.buffers[buffer_view.buffer];

                const float* data = reinterpret_cast<const float*>(
                    &buffer.data[buffer_view.byteOffset + accessor.byteOffset]
                );

                vertices.resize(accessor.count);
                for (size_t i = 0; i < accessor.count; ++i) {
                    vertices[i].position = glm::vec3(
                        data[i * 3 + 0],
                        data[i * 3 + 1],
                        data[i * 3 + 2]
                    );
                }
            }

            // Get normals
            bool has_normals = (primitive.attributes.find("NORMAL") != primitive.attributes.end());
            std::printf("  Primitive has normals: %s\n", has_normals ? "yes" : "no");
            if (has_normals) {
                const auto& accessor = gltf_model.accessors[primitive.attributes.at("NORMAL")];
                const auto& buffer_view = gltf_model.bufferViews[accessor.bufferView];
                const auto& buffer = gltf_model.buffers[buffer_view.buffer];

                const float* data = reinterpret_cast<const float*>(
                    &buffer.data[buffer_view.byteOffset + accessor.byteOffset]
                );

                for (size_t i = 0; i < accessor.count; ++i) {
                    vertices[i].normal = glm::vec3(
                        data[i * 3 + 0],
                        data[i * 3 + 1],
                        data[i * 3 + 2]
                    );
                }
                if (accessor.count > 0) {
                    std::printf("  First normal: (%.2f, %.2f, %.2f)\n",
                               vertices[0].normal.x, vertices[0].normal.y, vertices[0].normal.z);
                }
            } else {
                // 如果没有法线，设为默认值
                for (size_t i = 0; i < vertices.size(); ++i) {
                    vertices[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
            }

            // Get texture coordinates
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const auto& accessor = gltf_model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& buffer_view = gltf_model.bufferViews[accessor.bufferView];
                const auto& buffer = gltf_model.buffers[buffer_view.buffer];

                const float* data = reinterpret_cast<const float*>(
                    &buffer.data[buffer_view.byteOffset + accessor.byteOffset]
                );

                for (size_t i = 0; i < accessor.count; ++i) {
                    vertices[i].tex_coord = glm::vec2(
                        data[i * 2 + 0],
                        data[i * 2 + 1]
                    );
                }
            }

            // Get indices
            if (primitive.indices >= 0) {
                const auto& accessor = gltf_model.accessors[primitive.indices];
                const auto& buffer_view = gltf_model.bufferViews[accessor.bufferView];
                const auto& buffer = gltf_model.buffers[buffer_view.buffer];

                indices.resize(accessor.count);

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const unsigned short* data = reinterpret_cast<const unsigned short*>(
                        &buffer.data[buffer_view.byteOffset + accessor.byteOffset]
                    );
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = static_cast<unsigned int>(data[i]);
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const unsigned int* data = reinterpret_cast<const unsigned int*>(
                        &buffer.data[buffer_view.byteOffset + accessor.byteOffset]
                    );
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = data[i];
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    const unsigned char* data = reinterpret_cast<const unsigned char*>(
                        &buffer.data[buffer_view.byteOffset + accessor.byteOffset]
                    );
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = static_cast<unsigned int>(data[i]);
                    }
                }
            }

            // Get material
            if (primitive.material >= 0) {
                const auto& gltf_material = gltf_model.materials[primitive.material];

                // Base color factor
                if (gltf_material.pbrMetallicRoughness.baseColorFactor.size() >= 3) {
                    material.base_color = glm::vec3(
                        gltf_material.pbrMetallicRoughness.baseColorFactor[0],
                        gltf_material.pbrMetallicRoughness.baseColorFactor[1],
                        gltf_material.pbrMetallicRoughness.baseColorFactor[2]
                    );
                }

                material.metallic = gltf_material.pbrMetallicRoughness.metallicFactor;
                material.roughness = gltf_material.pbrMetallicRoughness.roughnessFactor;

                // Base color texture
                if (gltf_material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    int tex_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
                    if (model.m_textures.find(tex_index) != model.m_textures.end()) {
                        material.base_color_texture = model.m_textures[tex_index];
                    }
                }
            }

            // 如果没有法线，手动计算（假设是三角形列表）
            if (!has_normals && vertices.size() >= 3) {
                std::printf("  Calculating normals...\n");
                // 先清零
                for (auto& v : vertices) {
                    v.normal = glm::vec3(0.0f);
                }
                // 计算每个三角形的法线，并累加到顶点
                for (size_t i = 0; i < vertices.size(); i += 3) {
                    if (i + 2 >= vertices.size()) break;
                    glm::vec3 v0 = vertices[i + 0].position;
                    glm::vec3 v1 = vertices[i + 1].position;
                    glm::vec3 v2 = vertices[i + 2].position;
                    // 计算三角形法线（叉乘）
                    glm::vec3 edge1 = v1 - v0;
                    glm::vec3 edge2 = v2 - v0;
                    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
                    // 累加到三个顶点
                    vertices[i + 0].normal += normal;
                    vertices[i + 1].normal += normal;
                    vertices[i + 2].normal += normal;
                }
                // 归一化每个顶点的法线
                for (auto& v : vertices) {
                    if (glm::length(v.normal) > 0.0001f) {
                        v.normal = glm::normalize(v.normal);
                    } else {
                        v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
                if (!vertices.empty()) {
                    std::printf("  First calculated normal: (%.2f, %.2f, %.2f)\n",
                               vertices[0].normal.x, vertices[0].normal.y, vertices[0].normal.z);
                }
            }

            model.m_meshes.emplace_back(std::move(vertices), std::move(indices), std::move(material));
        }
    }

    return model;
}

void Model::draw() const
{
    for (const auto& mesh : m_meshes) {
        mesh.draw();
    }
}

} // namespace fish::graphics
