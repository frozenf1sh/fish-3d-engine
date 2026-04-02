#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "graphics/Mesh.hpp"
#include "graphics/Model.hpp"

namespace fish::scene {

/// @brief 变换组件 - 包含位置、旋转、缩放
struct TransformComponent {
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f}; // w, x, y, z
    glm::vec3 scale{1.0f};

    [[nodiscard]] auto get_model_matrix() const -> glm::mat4 {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotation_mat = glm::mat4_cast(rotation);
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);
        return translation * rotation_mat * scaling;
    }

    /// @brief 设置欧拉角旋转（弧度）
    void set_rotation_euler(const glm::vec3& euler) {
        rotation = glm::quat(euler);
    }

    /// @brief 获取欧拉角旋转（弧度）
    [[nodiscard]] auto get_rotation_euler() const -> glm::vec3 {
        return glm::eulerAngles(rotation);
    }
};

/// @brief 网格组件 - 指向模型资源
struct MeshComponent {
    std::shared_ptr<Model> model;
    std::string model_path; // 用于序列化

    MeshComponent() = default;

    explicit MeshComponent(std::shared_ptr<Model> m, std::string path = "")
        : model(std::move(m)), model_path(std::move(path)) {}
};

/// @brief 光源组件 - 方向光
struct LightComponent {
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f}; // 默认向下照射
    float intensity{1.0f};

    LightComponent() = default;

    LightComponent(const glm::vec3& color_, const glm::vec3& direction_, float intensity_ = 1.0f)
        : color(color_), direction(glm::normalize(direction_)), intensity(intensity_) {}
};

/// @brief 标签组件 - 用于标识实体
struct TagComponent {
    std::string tag;

    TagComponent() = default;
    explicit TagComponent(std::string t) : tag(std::move(t)) {}
};

} // namespace fish::scene
