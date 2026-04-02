#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Mesh.hpp"
#include "Texture2D.hpp"

namespace fish::graphics {

class Model {
public:
    static auto from_file(std::string_view path)
        -> std::expected<Model, std::string>;

    void draw() const;

    [[nodiscard]] auto get_meshes() const -> const std::vector<Mesh>& { return m_meshes; }
    [[nodiscard]] auto get_directory() const -> const std::filesystem::path& { return m_directory; }

private:
    Model() = default;

    static auto load_textures(std::string_view directory, const tinygltf::Model& model)
        -> std::unordered_map<int, std::shared_ptr<Texture2D>>;

    std::vector<Mesh> m_meshes;
    std::unordered_map<int, std::shared_ptr<Texture2D>> m_textures;
    std::filesystem::path m_directory;
};

} // namespace fish::graphics
