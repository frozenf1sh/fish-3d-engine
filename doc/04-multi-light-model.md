# 多光源与多模型实现指南

## 目录
1. [当前架构分析](#当前架构分析)
2. [实现多光源](#实现多光源)
3. [实现多模型](#实现多模型)
4. [完整示例](#完整示例)

---

## 当前架构分析

### 当前限制

目前的 `RenderSystem` 只支持一个主光源：

```cpp
// RenderSystem.cpp
const LightComponent* main_light = get_main_light(registry);
if (main_light) {
    shader.set_uniform("uLightColor", main_light->color * main_light->intensity);
    shader.set_uniform("uLightDir", main_light->direction);
}
```

着色器中也只有一套光照 uniform：

```glsl
// model.frag
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uLightDir;
```

---

## 实现多光源

### 方案一： Forward Rendering（前向渲染）- 推荐用于多光源

#### 1. 修改着色器支持多光源

**文件：`assets/shaders/model.vert`**

无需修改，顶点着色器不需要知道有多少光源。

**文件：`assets/shaders/model.frag`**

```glsl
#version 460 core

// 最大光源数量
#define MAX_LIGHTS 8

struct Light {
    vec3 color;
    vec3 direction;
    float intensity;
};

// ... 其他 uniform ...

uniform int uLightCount;
uniform Light uLights[MAX_LIGHTS];

// ...

void main()
{
    // ...

    // 环境光
    float ambient_strength = 0.15;
    vec3 ambient = ambient_strength * base_color;

    // 所有光源的漫反射和镜面反射总和
    vec3 diffuse_total = vec3(0.0);
    vec3 specular_total = vec3(0.0);

    for (int i = 0; i < uLightCount; ++i) {
        Light light = uLights[i];

        // 漫反射
        vec3 light_dir = normalize(light.direction);
        float diff = max(dot(norm, light_dir), 0.0);
        diffuse_total += diff * light.color * light.intensity;

        // 镜面反射 (Blinn-Phong)
        float specular_strength = 0.5 * (1.0 - uRoughness);
        vec3 halfway_dir = normalize(light_dir + view_dir);
        float spec = pow(max(dot(norm, halfway_dir), 0.0), 32.0);
        specular_total += specular_strength * spec * light.color * light.intensity;
    }

    diffuse_total *= base_color;

    // 计算阴影（暂时只用第一个光源的阴影）
    float shadow = calculate_shadow_pcf(frag_pos_light_space, norm, uLights[0].direction);

    // 最终颜色：阴影部分只保留环境光
    vec3 result = ambient + (1.0 - shadow) * (diffuse_total + specular_total);

    // ... Gamma 校正 ...
}
```

#### 2. 修改 C++ 代码传递多光源

**文件：`src/scene/RenderSystem.cpp`**

```cpp
void RenderSystem::render(entt::registry& registry,
                          graphics::Shader& shader,
                          const graphics::Camera& camera,
                          float aspect_ratio)
{
    // ... 设置 VP 矩阵 ...

    // 获取所有光源
    auto light_view = registry.view<LightComponent>();
    int light_count = 0;

    for (auto entity : light_view) {
        if (light_count >= 8) break; // 最多 8 个光源

        const auto& light = light_view.get<LightComponent>(entity);

        // 设置光源数组元素
        std::string prefix = "uLights[" + std::to_string(light_count) + "]";
        shader.set_uniform(prefix + ".color", light.color);
        shader.set_uniform(prefix + ".direction", light.direction);
        shader.set_uniform(prefix + ".intensity", light.intensity);

        ++light_count;
    }

    // 设置光源数量
    shader.set_uniform("uLightCount", light_count);

    // ... 渲染实体 ...
}
```

#### 3. 创建多个光源的示例

```cpp
void create_multiple_lights(entt::registry& registry) {
    // 主光源（太阳光）
    auto sun = registry.create();
    registry.emplace<TagComponent>(sun, "Sun");
    registry.emplace<TransformComponent>(sun);
    registry.emplace<LightComponent>(sun,
        glm::vec3(1.0f, 0.95f, 0.9f),      // 颜色
        glm::normalize(glm::vec3(5.0f, 12.0f, 5.0f)), // 方向
        1.0f  // 强度
    );

    // 补光（蓝色）
    auto fill_light = registry.create();
    registry.emplace<TagComponent>(fill_light, "FillLight");
    registry.emplace<TransformComponent>(fill_light);
    registry.emplace<LightComponent>(fill_light,
        glm::vec3(0.3f, 0.4f, 0.6f),
        glm::normalize(glm::vec3(-5.0f, 3.0f, -5.0f)),
        0.3f
    );

    // 氛围光（紫色）
    auto ambient_light = registry.create();
    registry.emplace<TagComponent>(ambient_light, "AmbientLight");
    registry.emplace<TransformComponent>(ambient_light);
    registry.emplace<LightComponent>(ambient_light,
        glm::vec3(0.4f, 0.2f, 0.5f),
        glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)),
        0.2f
    );
}
```

### 方案二：延迟渲染（Deferred Rendering）- 高级方案

延迟渲染适合非常多的光源（几十上百个），但实现更复杂：

1. **G-Buffer Pass**：渲染位置、法线、颜色等信息到多个纹理
2. **Lighting Pass**：在屏幕空间计算光照

这是一个大的架构改动，适合后期扩展。

---

## 实现多模型

### 当前状态

多模型已经天然支持了！每个实体都可以有自己的 `MeshComponent`。

```cpp
// 加载不同的模型
auto fox_model = std::make_shared<Model>(std::move(*Model::from_file("assets/models/Fox.gltf")));
auto tree_model = std::make_shared<Model>(std::move(*Model::from_file("assets/models/Tree.gltf")));
auto rock_model = std::make_shared<Model>(std::move(*Model::from_file("assets/models/Rock.gltf")));

// 创建多个实体，每个有不同的模型
auto fox = registry.create();
registry.emplace<MeshComponent>(fox, fox_model, "assets/models/Fox.gltf");

auto tree = registry.create();
registry.emplace<MeshComponent>(tree, tree_model, "assets/models/Tree.gltf");

auto rock = registry.create();
registry.emplace<MeshComponent>(rock, rock_model, "assets/models/Rock.gltf");
```

### 模型资源管理

当前使用 `shared_ptr` 自动管理，已经很高效了。可以进一步封装：

```cpp
// 可以创建一个 ModelManager 来缓存模型
class ModelManager {
public:
    std::shared_ptr<Model> get_or_load(const std::string& path) {
        auto it = m_cache.find(path);
        if (it != m_cache.end()) {
            return it->second;
        }

        auto model_result = Model::from_file(path);
        if (!model_result) {
            return nullptr;
        }

        auto model = std::make_shared<Model>(std::move(*model_result));
        m_cache[path] = model;
        return model;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Model>> m_cache;
};
```

---

## 完整示例

### 一个完整的多光源多模型场景

```cpp
#include "scene/Scene.hpp"

void create_complex_scene(entt::registry& registry) {
    // ==========================================
    // 1. 加载所有模型
    // ==========================================

    ModelManager model_manager;

    auto fox_model = model_manager.get_or_load("assets/models/Fox.gltf");
    auto tree_model = model_manager.get_or_load("assets/models/Tree.gltf");
    auto rock_model = model_manager.get_or_load("assets/models/Rock.gltf");

    if (!fox_model || !tree_model || !rock_model) {
        std::fprintf(stderr, "Failed to load some models!\n");
        return;
    }

    // ==========================================
    // 2. 创建光源
    // ==========================================

    // 主光源（太阳光）- 暖白色
    auto sun = registry.create();
    registry.emplace<TagComponent>(sun, "Sun");
    registry.emplace<TransformComponent>(sun);
    registry.emplace<LightComponent>(sun,
        glm::vec3(1.0f, 0.95f, 0.8f),
        glm::normalize(glm::vec3(5.0f, 12.0f, 5.0f)),
        1.0f
    );

    // 补光（蓝光）- 从下方
    auto blue_light = registry.create();
    registry.emplace<TagComponent>(blue_light, "BlueLight");
    registry.emplace<TransformComponent>(blue_light);
    registry.emplace<LightComponent>(blue_light,
        glm::vec3(0.2f, 0.3f, 0.6f),
        glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)),
        0.4f
    );

    // ==========================================
    // 3. 创建物体
    // ==========================================

    // 主角狐狸
    auto fox = registry.create();
    registry.emplace<TagComponent>(fox, "PlayerFox");
    {
        auto& transform = registry.emplace<TransformComponent>(fox);
        transform.position = glm::vec3(0.0f, -0.45f, 0.0f);
        transform.scale = glm::vec3(0.01f);
    }
    registry.emplace<MeshComponent>(fox, fox_model, "assets/models/Fox.gltf");

    // 一圈树
    for (int i = 0; i < 8; ++i) {
        float angle = i * glm::two_pi<float>() / 8.0f;
        float radius = 5.0f;

        auto tree = registry.create();
        registry.emplace<TagComponent>(tree, "Tree");

        auto& transform = registry.emplace<TransformComponent>(tree);
        transform.position = glm::vec3(
            cos(angle) * radius,
            0.0f,
            sin(angle) * radius
        );
        transform.scale = glm::vec3(0.5f + (rand() % 100) / 200.0f);
        transform.set_rotation_euler(glm::vec3(0.0f, angle, 0.0f));

        registry.emplace<MeshComponent>(tree, tree_model, "assets/models/Tree.gltf");
    }

    // 一些随机放置的石头
    for (int i = 0; i < 15; ++i) {
        auto rock = registry.create();
        registry.emplace<TagComponent>(rock, "Rock");

        auto& transform = registry.emplace<TransformComponent>(rock);
        transform.position = glm::vec3(
            (rand() % 200 - 100) / 10.0f,
            0.0f,
            (rand() % 200 - 100) / 10.0f
        );
        transform.scale = glm::vec3(0.1f + (rand() % 100) / 200.0f);
        transform.set_rotation_euler(glm::vec3(
            glm::radians((float)(rand() % 360)),
            glm::radians((float)(rand() % 360)),
            glm::radians((float)(rand() % 360))
        ));

        registry.emplace<MeshComponent>(rock, rock_model, "assets/models/Rock.gltf");
    }
}
```

---

## 多阴影（进阶）

### 每个光源独立阴影

目前阴影只支持第一个光源。要支持多个光源的阴影：

1. **需要多个 Shadow Map**：每个光源一个 FBO
2. **需要多个光源空间矩阵**
3. **着色器中采样对应的 Shadow Map**

这会显著增加复杂度和显存使用。

### 简化方案：只让主光源投射阴影

这是最常见的做法：
- 只有最主要的光源（通常是太阳）投射阴影
- 其他光源只贡献光照，不投射阴影

这样在着色器中只需要：
```glsl
// 仍然只用 uLightSpaceMatrix 和 uShadowMap
float shadow = calculate_shadow_pcf(frag_pos_light_space, norm, uLights[0].direction);
```

---

## 性能考虑

### 多光源性能

- **前向渲染**：O(物体数量 × 光源数量)
  - 适合：少量光源（< 10）
  - 优点：简单，支持透明物体
  - 缺点：光源多了很慢

- **延迟渲染**：O(屏幕像素 × 光源数量)
  - 适合：大量光源（> 50）
  - 优点：光源多了也快
  - 缺点：复杂，不支持透明物体，显存占用大

### 多模型性能

当前架构已经很高效：
- 模型资源通过 `shared_ptr` 共享
- 相同模型只加载一次
- 遍历只访问有 MeshComponent 的实体

可以进一步优化：
- **视锥体剔除（Frustum Culling）**：不渲染摄像机看不到的物体
- **实例化渲染（Instancing）**：相同模型用一次绘制调用
