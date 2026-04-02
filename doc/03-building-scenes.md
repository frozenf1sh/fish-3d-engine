# 场景搭建指南

## 目录
1. [快速入门](#快速入门)
2. [创建实体](#创建实体)
3. [添加组件](#添加组件)
4. [完整示例](#完整示例)
5. [场景序列化](#场景序列化)

---

## 快速入门

### 最简单的场景

```cpp
// 1. 创建注册表
entt::registry registry;

// 2. 创建一个实体
auto entity = registry.create();

// 3. 添加组件
registry.emplace<TransformComponent>(entity);
registry.emplace<TagComponent>(entity, "MyObject");
```

---

## 创建实体

### 基本方式

```cpp
// 创建一个空实体
entt::entity entity = registry.create();

// 创建实体时可以直接设置版本号（高级用法）
auto entity = registry.create(entt::entity{42});
```

### 批量创建

```cpp
// 一次创建多个实体
std::vector<entt::entity> entities(100);
registry.create(entities.begin(), entities.end());
```

### 销毁实体

```cpp
// 销毁单个实体
registry.destroy(entity);

// 检查实体是否有效
if (registry.valid(entity)) {
    // 实体还存在
}

// 销毁所有实体
registry.clear();
```

---

## 添加组件

### TransformComponent（变换组件）

#### 基础设置

```cpp
auto entity = registry.create();
auto& transform = registry.emplace<TransformComponent>(entity);

// 设置位置
transform.position = glm::vec3(0.0f, 1.0f, 0.0f);

// 设置缩放
transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);

// 设置旋转（四元数）
transform.rotation = glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));

// 或者使用欧拉角辅助函数
transform.set_rotation_euler(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
```

#### 获取模型矩阵

```cpp
// 获取用于渲染的模型矩阵
glm::mat4 model_matrix = transform.get_model_matrix();
shader.set_uniform("uModel", model_matrix);
```

#### 修改已有组件

```cpp
// 检查是否有组件
if (registry.all_of<TransformComponent>(entity)) {
    // 获取引用并修改
    auto& transform = registry.get<TransformComponent>(entity);
    transform.position.x += 1.0f;
}

// 或者尝试获取（如果不存在则添加）
auto& transform = registry.get_or_emplace<TransformComponent>(entity);
```

### MeshComponent（网格组件）

#### 从文件加载

```cpp
// 加载模型
std::printf("Loading model...\n");
auto model_result = Model::from_file("assets/models/Fox.gltf");
if (!model_result) {
    std::fprintf(stderr, "%s\n", model_result.error().c_str());
    return;
}

// 创建 shared_ptr
auto model = std::make_shared<Model>(std::move(*model_result));

// 添加到实体
auto entity = registry.create();
registry.emplace<TransformComponent>(entity);
registry.emplace<MeshComponent>(entity, std::move(model), "assets/models/Fox.gltf");
```

#### 资源共享

多个实体可以共享同一个模型资源：

```cpp
// 只加载一次模型
auto tree_model = std::make_shared<Model>(std::move(*Model::from_file("assets/models/Tree.gltf")));

// 创建 10 棵树，都使用同一个模型
for (int i = 0; i < 10; ++i) {
    auto entity = registry.create();

    // 每棵树有不同的位置
    auto& transform = registry.emplace<TransformComponent>(entity);
    transform.position = glm::vec3(i * 5.0f, 0.0f, 0.0f);

    // 但共享同一个模型
    registry.emplace<MeshComponent>(entity, tree_model, "assets/models/Tree.gltf");
}
```

### LightComponent（光源组件）

#### 创建方向光

```cpp
auto entity = registry.create();
registry.emplace<TagComponent>(entity, "MainLight");

// 从上方照射的白光
glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 light_dir = glm::normalize(glm::vec3(5.0f, 12.0f, 5.0f));

registry.emplace<LightComponent>(entity, light_color, light_dir, 1.0f);
```

#### 修改光源

```cpp
auto& light = registry.get<LightComponent>(entity);

// 改变颜色为暖色调
light.color = glm::vec3(1.0f, 0.9f, 0.7f);

// 改变方向
light.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));

// 调整强度
light.intensity = 1.5f;
```

### TagComponent（标签组件）

```cpp
auto entity = registry.create();

// 添加标签
registry.emplace<TagComponent>(entity, "Player");

// 获取标签
auto& tag = registry.get<TagComponent>(entity);
std::cout << "Entity tag: " << tag.tag << std::endl;
```

---

## 完整示例

### 创建一个带狐狸的场景

```cpp
#include "scene/Scene.hpp"

void create_default_scene(entt::registry& registry) {
    // 加载狐狸模型
    auto fox_model_result = Model::from_file("assets/models/Fox.gltf");
    if (!fox_model_result) {
        return;
    }
    auto fox_model = std::make_shared<Model>(std::move(*fox_model_result));

    // 创建狐狸实体
    auto fox = registry.create();
    registry.emplace<TagComponent>(fox, "Fox");

    auto& fox_transform = registry.emplace<TransformComponent>(fox);
    fox_transform.position = glm::vec3(0.0f, -0.45f, 0.0f);
    fox_transform.scale = glm::vec3(0.01f);

    registry.emplace<MeshComponent>(fox, fox_model, "assets/models/Fox.gltf");

    // 创建光源
    auto light = registry.create();
    registry.emplace<TagComponent>(light, "DirectionalLight");
    registry.emplace<TransformComponent>(light);

    glm::vec3 light_pos = glm::vec3(5.0f, 12.0f, 5.0f);
    glm::vec3 light_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 light_dir = glm::normalize(light_pos - light_target);

    registry.emplace<LightComponent>(light, glm::vec3(1.0f), light_dir, 1.0f);
}
```

### 创建多个模型

```cpp
void create_forest_scene(entt::registry& registry) {
    // 加载树模型
    auto tree_model_result = Model::from_file("assets/models/Tree.gltf");
    if (!tree_model_result) return;
    auto tree_model = std::make_shared<Model>(std::move(*tree_model_result));

    // 创建 5x5 的树林
    for (int x = -2; x <= 2; ++x) {
        for (int z = -2; z <= 2; ++z) {
            auto entity = registry.create();

            auto& transform = registry.emplace<TransformComponent>(entity);
            transform.position = glm::vec3(x * 3.0f, 0.0f, z * 3.0f);
            transform.scale = glm::vec3(0.5f + (rand() % 100) / 200.0f); // 随机大小
            transform.set_rotation_euler(glm::vec3(0.0f, glm::radians(rand() % 360), 0.0f));

            registry.emplace<MeshComponent>(entity, tree_model, "assets/models/Tree.gltf");
        }
    }
}
```

---

## 场景序列化

### 保存场景

```cpp
SceneSerializer serializer(registry);

// 保存到文件
if (serializer.serialize("scene.json")) {
    std::printf("Scene saved successfully!\n");
} else {
    std::printf("Failed to save scene!\n");
}
```

### 加载场景

```cpp
SceneSerializer serializer(registry);

// 检查文件是否存在
if (std::filesystem::exists("scene.json")) {
    if (serializer.deserialize("scene.json")) {
        std::printf("Scene loaded successfully!\n");
    } else {
        std::printf("Failed to load scene!\n");
    }
} else {
    std::printf("Scene file not found, creating default scene...\n");
    create_default_scene(registry);
}
```

### 生成的 scene.json 格式

```json
{
  "entities": [
    {
      "id": 0,
      "tag": "Fox",
      "transform": {
        "position": {"x": 0.0, "y": -0.45, "z": 0.0},
        "rotation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
        "scale": {"x": 0.01, "y": 0.01, "z": 0.01}
      },
      "mesh": {
        "model_path": "assets/models/Fox.gltf"
      }
    },
    {
      "id": 1,
      "tag": "DirectionalLight",
      "transform": {
        "position": {"x": 0.0, "y": 0.0, "z": 0.0},
        "rotation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
        "scale": {"x": 1.0, "y": 1.0, "z": 1.0}
      },
      "light": {
        "color": {"x": 1.0, "y": 1.0, "z": 1.0},
        "direction": {"x": 0.377, "y": 0.904, "z": 0.377},
        "intensity": 1.0
      }
    }
  ]
}
```

---

## 在主循环中使用

### 基本渲染流程

```cpp
int main() {
    // ... 初始化 ...

    entt::registry registry;
    create_default_scene(registry);
    SceneSerializer serializer(registry);

    bool save_scene = false;
    bool load_scene = false;

    while (!glfwWindowShouldClose(window)) {
        // ... 处理输入 ...

        // F5: 保存场景
        if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !save_scene) {
            save_scene = true;
            serializer.serialize("scene.json");
        }
        if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE) {
            save_scene = false;
        }

        // F9: 加载场景
        if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS && !load_scene) {
            load_scene = true;
            if (std::filesystem::exists("scene.json")) {
                serializer.deserialize("scene.json");
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_RELEASE) {
            load_scene = false;
        }

        // ... 阴影 Pass ...

        // 渲染场景
        RenderSystem::render(registry, shader, camera, aspect_ratio);

        // ...
    }
}
```

---

## 组件操作技巧

### 检查和获取组件

```cpp
// 检查实体是否有组件
if (registry.all_of<TransformComponent>(entity)) {
    // 有这个组件
}

// 检查是否有任意一个组件
if (registry.any_of<TransformComponent, MeshComponent>(entity)) {
    // 至少有一个
}

// 获取组件（如果确定存在）
auto& transform = registry.get<TransformComponent>(entity);

// 尝试获取组件（可能返回 null）
auto* transform_ptr = registry.try_get<TransformComponent>(entity);
if (transform_ptr) {
    // 存在
}
```

### 移除组件

```cpp
// 移除单个组件
registry.remove<MeshComponent>(entity);

// 移除多个组件
registry.remove<TransformComponent, MeshComponent>(entity);

// 移除但不析构（高级用法）
registry.erase<MeshComponent>(entity);
```

### 替换或添加

```cpp
// 如果有就替换，没有就添加
auto& transform = registry.emplace_or_replace<TransformComponent>(entity);
transform.position = glm::vec3(1.0f);

// 如果没有就添加，有就返回现有
auto& transform = registry.get_or_emplace<TransformComponent>(entity);
```

---

## 调试技巧

### 遍历所有实体

```cpp
auto view = registry.view<entt::entity>();
for (auto entity : view) {
    std::printf("Entity: %u\n", static_cast<uint32_t>(entity));

    // 检查有哪些组件
    if (registry.all_of<TagComponent>(entity)) {
        std::printf("  Tag: %s\n", registry.get<TagComponent>(entity).tag.c_str());
    }
    if (registry.all_of<TransformComponent>(entity)) {
        auto& t = registry.get<TransformComponent>(entity);
        std::printf("  Position: (%.2f, %.2f, %.2f)\n",
                    t.position.x, t.position.y, t.position.z);
    }
}
```
