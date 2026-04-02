#include "SceneSerializer.hpp"
#include "Components.hpp"
#include "graphics/Model.hpp"

#include <fstream>
#include <iostream>

namespace fish::scene {

SceneSerializer::SceneSerializer(entt::registry& registry)
    : m_registry(registry)
{
}

auto SceneSerializer::serialize(const std::filesystem::path& file_path) const -> bool
{
    nlohmann::json json = serialize_to_json();

    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << file_path << std::endl;
        return false;
    }

    file << json.dump(4);
    return file.good();
}

auto SceneSerializer::deserialize(const std::filesystem::path& file_path) -> bool
{
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << file_path << std::endl;
        return false;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }

    deserialize_from_json(json);
    return true;
}

auto SceneSerializer::serialize_to_json() const -> nlohmann::json
{
    nlohmann::json json;
    json["entities"] = nlohmann::json::array();

    auto view = m_registry.view<entt::entity>();
    for (auto entity : view) {
        nlohmann::json entity_json;
        entity_json["id"] = static_cast<uint32_t>(entity);

        // TagComponent
        if (m_registry.all_of<TagComponent>(entity)) {
            const auto& tag = m_registry.get<TagComponent>(entity);
            entity_json["tag"] = tag.tag;
        }

        // TransformComponent
        if (m_registry.all_of<TransformComponent>(entity)) {
            const auto& transform = m_registry.get<TransformComponent>(entity);
            entity_json["transform"]["position"] = {
                {"x", transform.position.x},
                {"y", transform.position.y},
                {"z", transform.position.z}
            };
            entity_json["transform"]["rotation"] = {
                {"w", transform.rotation.w},
                {"x", transform.rotation.x},
                {"y", transform.rotation.y},
                {"z", transform.rotation.z}
            };
            entity_json["transform"]["scale"] = {
                {"x", transform.scale.x},
                {"y", transform.scale.y},
                {"z", transform.scale.z}
            };
        }

        // MeshComponent
        if (m_registry.all_of<MeshComponent>(entity)) {
            const auto& mesh = m_registry.get<MeshComponent>(entity);
            entity_json["mesh"]["model_path"] = mesh.model_path;
        }

        // LightComponent
        if (m_registry.all_of<LightComponent>(entity)) {
            const auto& light = m_registry.get<LightComponent>(entity);
            entity_json["light"]["color"] = {
                {"x", light.color.x},
                {"y", light.color.y},
                {"z", light.color.z}
            };
            entity_json["light"]["direction"] = {
                {"x", light.direction.x},
                {"y", light.direction.y},
                {"z", light.direction.z}
            };
            entity_json["light"]["intensity"] = light.intensity;
        }

        json["entities"].push_back(entity_json);
    }

    return json;
}

void SceneSerializer::deserialize_from_json(const nlohmann::json& json)
{
    // 清空现有场景
    m_registry.clear();

    if (!json.contains("entities")) {
        return;
    }

    for (const auto& entity_json : json["entities"]) {
        // 创建实体
        auto entity = m_registry.create();

        // TagComponent
        if (entity_json.contains("tag")) {
            m_registry.emplace<TagComponent>(entity, entity_json["tag"].get<std::string>());
        }

        // TransformComponent
        if (entity_json.contains("transform")) {
            const auto& transform_json = entity_json["transform"];
            TransformComponent transform;

            transform.position.x = transform_json["position"]["x"].get<float>();
            transform.position.y = transform_json["position"]["y"].get<float>();
            transform.position.z = transform_json["position"]["z"].get<float>();

            transform.rotation.w = transform_json["rotation"]["w"].get<float>();
            transform.rotation.x = transform_json["rotation"]["x"].get<float>();
            transform.rotation.y = transform_json["rotation"]["y"].get<float>();
            transform.rotation.z = transform_json["rotation"]["z"].get<float>();

            transform.scale.x = transform_json["scale"]["x"].get<float>();
            transform.scale.y = transform_json["scale"]["y"].get<float>();
            transform.scale.z = transform_json["scale"]["z"].get<float>();

            m_registry.emplace<TransformComponent>(entity, std::move(transform));
        } else {
            // 默认 Transform
            m_registry.emplace<TransformComponent>(entity);
        }

        // MeshComponent
        if (entity_json.contains("mesh")) {
            const auto& mesh_json = entity_json["mesh"];
            std::string model_path = mesh_json["model_path"].get<std::string>();

            // 尝试加载模型
            auto model_result = graphics::Model::from_file(model_path);
            if (model_result) {
                auto model = std::make_shared<graphics::Model>(std::move(*model_result));
                m_registry.emplace<MeshComponent>(entity, std::move(model), model_path);
            } else {
                std::cerr << "Warning: Failed to load model: " << model_path << std::endl;
            }
        }

        // LightComponent
        if (entity_json.contains("light")) {
            const auto& light_json = entity_json["light"];
            LightComponent light;

            light.color.x = light_json["color"]["x"].get<float>();
            light.color.y = light_json["color"]["y"].get<float>();
            light.color.z = light_json["color"]["z"].get<float>();

            light.direction.x = light_json["direction"]["x"].get<float>();
            light.direction.y = light_json["direction"]["y"].get<float>();
            light.direction.z = light_json["direction"]["z"].get<float>();

            light.intensity = light_json["intensity"].get<float>();

            m_registry.emplace<LightComponent>(entity, std::move(light));
        }
    }
}

} // namespace fish::scene
