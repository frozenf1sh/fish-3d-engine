#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "graphics/Shader.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Camera.hpp"
#include "Components.hpp"

namespace fish::scene {

/// @brief 渲染系统 - 负责场景渲染
class RenderSystem {
public:
    /// @brief 渲染场景（光照 Pass）
    /// @param registry ECS 注册表
    /// @param shader 主着色器
    /// @param camera 摄像机
    /// @param aspect_ratio 宽高比
    static void render(entt::registry& registry,
                      graphics::Shader& shader,
                      const graphics::Camera& camera,
                      float aspect_ratio);

    /// @brief 渲染阴影 Pass
    /// @param registry ECS 注册表
    /// @param shadow_shader 阴影着色器
    /// @param light_space_matrix 光源空间矩阵
    /// @param shadow_fbo 阴影 FBO
    static void render_shadow_pass(entt::registry& registry,
                                   graphics::Shader& shadow_shader,
                                   const glm::mat4& light_space_matrix,
                                   graphics::Framebuffer& shadow_fbo);

    /// @brief 获取场景中的主光源
    /// @param registry ECS 注册表
    /// @return 光源组件指针（如果没有返回 nullptr）
    [[nodiscard]] static auto get_main_light(entt::registry& registry)
        -> const LightComponent*;
};

} // namespace fish::scene
