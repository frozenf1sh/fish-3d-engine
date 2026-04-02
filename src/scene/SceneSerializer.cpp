#include "SceneSerializer.hpp"
#include "Components.hpp"
#include "graphics/Model.hpp"

#include <fstream>
#include <iostream>

namespace fish::scene {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    TransformComponent,
    position,
    rotation,
    scale
)

// glm::quat 的序列化支持
void to_json(nlohmann::json& j, const glm::quat& q) {
    j = nlohmann::json{{"w", q.w}, {"x", q.x}, {"y", q.y}, {"z", q.z}};
}

void from_json(const nlohmann::json& j, glm::quat& q) {
    j.at("w").get_to(q.w);
    j.at("x").get_to(q.x);
    j.at("y").get_to(q.y);
    j.at("z").get_to(q.z);
}

// glm::vec3 的序列化支持
void to_json(nlohmann::json& j, const glm::vec3& v) {
    j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

void from_json(const nlohmann::json& j, glm::vec3& v) {
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
}

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
            entity_json["transform"] = transform;
        }

        // MeshComponent
        if (m_registry.all_of<MeshComponent>(entity)) {
            const auto& mesh = m_registry.get<MeshComponent>(entity);
            entity_json["mesh"]["model_path"] = mesh.model_path;
        }

        // LightComponent
        if (m_registry.all_of<LightComponent>(entity)) {
            const auto& light = m_registry.get<LightComponent>(entity);
            entity_json["light"]["color"] = light.color;
            entity_json["light"]["direction"] = light.direction;
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
            auto transform = entity_json["transform"].get<TransformComponent>();
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
            light_json.at("color").get_to(light.color);
            light_json.at("direction").get_to(light.direction);
            light_json.at("intensity").get_to(light.intensity);
            m_registry.emplace<LightComponent>(entity, std::move(light));
        }
    }
}

} // namespace fish::scene
