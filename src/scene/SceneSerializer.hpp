#pragma once

#include <string>
#include <filesystem>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace fish::scene {

/// @brief 场景序列化器 - 负责保存和加载场景
class SceneSerializer {
public:
    /// @brief 构造函数
    /// @param registry ECS 注册表引用
    explicit SceneSerializer(entt::registry& registry);

    /// @brief 序列化场景到 JSON 文件
    /// @param file_path 保存路径
    /// @return 是否成功
    auto serialize(const std::filesystem::path& file_path) const -> bool;

    /// @brief 从 JSON 文件反序列化场景
    /// @param file_path 加载路径
    /// @return 是否成功
    auto deserialize(const std::filesystem::path& file_path) -> bool;

    /// @brief 序列化为 JSON 对象
    /// @return JSON 对象
    [[nodiscard]] auto serialize_to_json() const -> nlohmann::json;

    /// @brief 从 JSON 对象反序列化
    /// @param json JSON 对象
    void deserialize_from_json(const nlohmann::json& json);

private:
    entt::registry& m_registry;
};

} // namespace fish::scene
