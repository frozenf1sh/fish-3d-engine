#include "RenderSystem.hpp"
#include "Components.hpp"

namespace fish::scene {

void RenderSystem::render(entt::registry& registry,
                          graphics::Shader& shader,
                          const graphics::Camera& camera,
                          float aspect_ratio)
{
    // 设置 VP 矩阵
    glm::mat4 view_matrix = camera.get_view_matrix();
    glm::mat4 projection_matrix = camera.get_projection_matrix(aspect_ratio);

    shader.set_uniform("uView", view_matrix);
    shader.set_uniform("uProjection", projection_matrix);
    shader.set_uniform("uViewPos", camera.get_position());

    // 设置光源
    const LightComponent* main_light = get_main_light(registry);
    if (main_light) {
        shader.set_uniform("uLightColor", main_light->color * main_light->intensity);
        shader.set_uniform("uLightDir", main_light->direction);
    }

    // 渲染所有带 MeshComponent 和 TransformComponent 的实体
    auto view = registry.view<TransformComponent, MeshComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& mesh_comp = view.get<MeshComponent>(entity);

        if (!mesh_comp.model) {
            continue;
        }

        shader.set_uniform("uModel", transform.get_model_matrix());

        // 绘制模型的每个网格
        for (const auto& mesh : mesh_comp.model->get_meshes()) {
            const auto& material = mesh.get_material();

            shader.set_uniform("uBaseColor", material.base_color);
            shader.set_uniform("uMetallic", material.metallic);
            shader.set_uniform("uRoughness", material.roughness);
            shader.set_uniform("uHasTexture", material.base_color_texture != nullptr);

            if (material.base_color_texture) {
                material.base_color_texture->bind(0);
            }
            shader.set_uniform("uBaseColorTexture", 0);

            mesh.draw();
        }
    }
}

void RenderSystem::render_shadow_pass(entt::registry& registry,
                                       graphics::Shader& shadow_shader,
                                       const glm::mat4& light_space_matrix,
                                       graphics::Framebuffer& shadow_fbo)
{
    shadow_fbo.bind();
    glViewport(0, 0, shadow_fbo.width(), shadow_fbo.height());
    glClear(GL_DEPTH_BUFFER_BIT);

    shadow_shader.bind();
    shadow_shader.set_uniform("uLightSpaceMatrix", light_space_matrix);

    // 渲染所有带 MeshComponent 和 TransformComponent 的实体
    auto view = registry.view<TransformComponent, MeshComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& mesh_comp = view.get<MeshComponent>(entity);

        if (!mesh_comp.model) {
            continue;
        }

        shadow_shader.set_uniform("uModel", transform.get_model_matrix());

        for (const auto& mesh : mesh_comp.model->get_meshes()) {
            mesh.draw();
        }
    }

    shadow_fbo.unbind();
}

auto RenderSystem::get_main_light(entt::registry& registry)
    -> const LightComponent*
{
    auto view = registry.view<LightComponent>();
    if (!view.empty()) {
        return &view.get<LightComponent>(view.front());
    }
    return nullptr;
}

} // namespace fish::scene
