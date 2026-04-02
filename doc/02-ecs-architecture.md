# ECS 架构详解

## 目录
1. [什么是 ECS](#什么是-ecs)
2. [为什么使用 ECS](#为什么使用-ecs)
3. [项目中的 ECS 实现](#项目中的-ecs-实现)
4. [核心组件详解](#核心组件详解)
5. [系统详解](#系统详解)

---

## 什么是 ECS？

### ECS 的定义

**ECS** 是 **Entity-Component-System**（实体-组件-系统）的缩写，是一种软件架构模式，特别适合游戏开发和仿真系统。

### 三个核心概念

#### 1. Entity（实体）
- **唯一标识符**：一个 ID，用来表示游戏世界中的"事物"
- **本身不包含数据或逻辑**：只是一个"容器"的引用
- 在 Entt 中：`entt::entity` 本质上是一个整数 ID

```cpp
// 创建一个实体
entt::entity entity = registry.create();

// 销毁实体
registry.destroy(entity);
```

#### 2. Component（组件）
- **纯数据**：不包含任何逻辑（行为）
- **C-style struct**：通常只是一些字段的集合
- **附加到实体上**：一个实体可以有多个组件

```cpp
// 组件示例
struct TransformComponent {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};
```

#### 3. System（系统）
- **纯逻辑**：不存储数据，只处理数据
- **处理具有特定组件的实体**
- **遍历和更新**：每帧调用一次

```cpp
// 系统示例
class RenderSystem {
public:
    static void render(entt::registry& registry, ...) {
        // 遍历所有有 Transform 和 Mesh 的实体
        auto view = registry.view<TransformComponent, MeshComponent>();
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);
            auto& mesh = view.get<MeshComponent>(entity);
            // 渲染逻辑...
        }
    }
};
```

---

## 为什么使用 ECS？

### 1. 组合优于继承

**传统 OOP 继承的问题：**
```
GameObject
├── Character
│   ├── Player
│   └── Enemy
└── Prop
    ├── Tree
    └── Crate
```

- 如果想要一个既是 Enemy 又是 Prop 的东西？
- 继承层级深了之后难以维护
- "钻石问题"

**ECS 的组合方式：**
```
实体：
├─ 实体1: [Transform, Mesh, Physics, PlayerController] (玩家)
├─ 实体2: [Transform, Mesh, Physics, AI] (敌人)
└─ 实体3: [Transform, Mesh] (道具)
```

- 自由组合需要的功能
- 没有继承的限制

### 2. 缓存友好（Cache-friendly）

**内存布局：**
```
Entt 中的存储方式（dense sets）：

组件数据在内存中是连续存储的：
[Transform0][Transform1][Transform2][Transform3]...
[Mesh0][Mesh1][Mesh2][Mesh3]...
```

**好处：**
- CPU 缓存命中率高
- 预取（Prefetching）效果好
- 遍历速度快

### 3. 更好的性能

- **按需处理**：只处理有特定组件的实体
- **并行友好**：不同系统可以并行执行
- **零开销抽象**：Entt 是 header-only，编译期优化

---

## 项目中的 ECS 实现

### 目录结构

```
src/scene/
├── Components.hpp       # 所有组件定义
├── RenderSystem.hpp/cpp # 渲染系统
├── SceneSerializer.hpp/cpp # 场景序列化
└── Scene.hpp            # 统一引入头文件
```

### Registry（注册表）

`entt::registry` 是 ECS 的核心，负责：
- 存储所有实体
- 存储所有组件数据
- 管理实体和组件的生命周期

```cpp
// 创建注册表
entt::registry registry;

// 创建实体并添加组件
auto entity = registry.create();
registry.emplace<TransformComponent>(entity);
registry.emplace<MeshComponent>(entity, model, path);
```

---

## 核心组件详解

### 1. TransformComponent（变换组件）

文件：`src/scene/Components.hpp`

```cpp
struct TransformComponent {
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    // 获取模型矩阵
    [[nodiscard]] auto get_model_matrix() const -> glm::mat4 {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotation_mat = glm::mat4_cast(rotation);
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);
        return translation * rotation_mat * scaling;
    }

    // 欧拉角辅助函数
    void set_rotation_euler(const glm::vec3& euler);
    [[nodiscard]] auto get_rotation_euler() const -> glm::vec3;
};
```

**为什么用四元数？**
- 避免万向锁（Gimbal Lock）
- 插值更平滑
- 存储更高效（4 个浮点数 vs 9 个）

**使用示例：**
```cpp
auto& transform = registry.emplace<TransformComponent>(entity);
transform.position = glm::vec3(0.0f, 1.0f, 0.0f);
transform.set_rotation_euler(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
transform.scale = glm::vec3(2.0f);
```

### 2. MeshComponent（网格组件）

```cpp
struct MeshComponent {
    std::shared_ptr<graphics::Model> model;
    std::string model_path; // 用于序列化

    MeshComponent() = default;

    MeshComponent(std::shared_ptr<graphics::Model> m, std::string path)
        : model(std::move(m)), model_path(std::move(path)) {}
};
```

**设计说明：**
- `model`：指向共享的模型资源（多个实体可以共用一个模型）
- `model_path`：保存模型文件路径，用于序列化和重新加载
- 使用 `shared_ptr` 管理资源生命周期

**使用示例：**
```cpp
auto model_result = Model::from_file("assets/models/Fox.gltf");
if (model_result) {
    auto model = std::make_shared<Model>(std::move(*model_result));
    registry.emplace<MeshComponent>(entity, std::move(model), "assets/models/Fox.gltf");
}
```

### 3. LightComponent（光源组件）

```cpp
struct LightComponent {
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    float intensity{1.0f};

    LightComponent() = default;

    LightComponent(const glm::vec3& color_, const glm::vec3& direction_, float intensity_ = 1.0f)
        : color(color_), direction(glm::normalize(direction_)), intensity(intensity_) {}
};
```

**当前实现：方向光**
- 只有方向，没有位置
- 所有地方的光照强度相同
- 适合太阳光等平行光源

**未来扩展：**
```cpp
// 点光源
struct PointLightComponent {
    glm::vec3 color;
    glm::vec3 position;
    float intensity;
    float radius;
};

// 聚光灯
struct SpotLightComponent {
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 direction;
    float intensity;
    float inner_angle;
    float outer_angle;
};
```

### 4. TagComponent（标签组件）

```cpp
struct TagComponent {
    std::string tag;

    TagComponent() = default;
    explicit TagComponent(std::string t) : tag(std::move(t)) {}
};
```

**用途：**
- 给实体一个可读的名字
- 调试时方便识别
- 可以按标签查找实体

**使用示例：**
```cpp
registry.emplace<TagComponent>(entity, "Player");
registry.emplace<TagComponent>(entity, "MainCamera");
```

---

## 系统详解

### RenderSystem（渲染系统）

文件：`src/scene/RenderSystem.hpp/cpp`

```cpp
class RenderSystem {
public:
    // 渲染光照 Pass
    static void render(entt::registry& registry,
                      graphics::Shader& shader,
                      const graphics::Camera& camera,
                      float aspect_ratio);

    // 渲染阴影 Pass
    static void render_shadow_pass(entt::registry& registry,
                                   graphics::Shader& shadow_shader,
                                   const glm::mat4& light_space_matrix,
                                   graphics::Framebuffer& shadow_fbo);

    // 获取主光源
    [[nodiscard]] static auto get_main_light(entt::registry& registry)
        -> const LightComponent*;
};
```

#### View（视图）的使用

Entt 的 `view` 是查询具有特定组件的实体的方式：

```cpp
// 获取所有同时有 Transform 和 Mesh 的实体
auto view = registry.view<TransformComponent, MeshComponent>();

// 遍历
for (auto entity : view) {
    auto& transform = view.get<TransformComponent>(entity);
    auto& mesh = view.get<MeshComponent>(entity);
    // ...
}
```

**高级用法：**
```cpp
// 排除某些组件
auto view = registry.view<TransformComponent, MeshComponent>(entt::exclude<LightComponent>);

// 只获取其中一个组件（不构造临时对象）
auto view = registry.view<TransformComponent>();
for (auto [entity, transform] : view.each()) {
    // ...
}
```

#### 渲染流程

```cpp
void RenderSystem::render(entt::registry& registry, ...)
{
    // 1. 设置 VP 矩阵和摄像机
    shader.set_uniform("uView", view_matrix);
    shader.set_uniform("uProjection", projection_matrix);

    // 2. 设置光源
    const LightComponent* main_light = get_main_light(registry);
    if (main_light) {
        shader.set_uniform("uLightColor", main_light->color * main_light->intensity);
        shader.set_uniform("uLightDir", main_light->direction);
    }

    // 3. 遍历并渲染所有实体
    auto view = registry.view<TransformComponent, MeshComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& mesh_comp = view.get<MeshComponent>(entity);

        shader.set_uniform("uModel", transform.get_model_matrix());

        // 绘制模型
        for (const auto& mesh : mesh_comp.model->get_meshes()) {
            // 设置材质...
            mesh.draw();
        }
    }
}
```

---

## 更多系统设计思路

### 未来可以添加的系统

#### 1. PhysicsSystem（物理系统）

```cpp
class PhysicsSystem {
public:
    static void update(entt::registry& registry, float delta_time) {
        auto view = registry.view<TransformComponent, PhysicsComponent>();
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);
            auto& physics = view.get<PhysicsComponent>(entity);

            // 应用重力
            physics.velocity += glm::vec3(0.0f, -9.8f, 0.0f) * delta_time;

            // 更新位置
            transform.position += physics.velocity * delta_time;
        }
    }
};
```

#### 2. InputSystem（输入系统）

```cpp
class InputSystem {
public:
    static void update(entt::registry& registry, GLFWwindow* window) {
        auto view = registry.view<TransformComponent, PlayerControllerComponent>();
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                transform.position += glm::vec3(0.0f, 0.0f, -1.0f) * speed;
            }
            // ...
        }
    }
};
```

---

## Entt 的更多功能

### 1. Group（组）

对于频繁访问的组件组合，可以使用 `group`：

```cpp
// 创建一个组，内部优化了存储
auto group = registry.group<TransformComponent>(entt::get<MeshComponent>);

// 使用方式和 view 类似
for (auto entity : group) {
    auto& transform = group.get<TransformComponent>(entity);
    auto& mesh = group.get<MeshComponent>(entity);
}
```

### 2. Snapshot（快照）

用于序列化或保存状态：

```cpp
// 创建快照
auto snapshot = registry.snapshot();
snapshot.entities(entity_list);
snapshot.components<TransformComponent>(transform_list);
```

### 3. Signal（信号）

组件添加/移除时的回调：

```cpp
// 监听组件添加
registry.on_construct<MeshComponent>().connect<&on_mesh_added>();

void on_mesh_added(entt::registry& registry, entt::entity entity) {
    std::cout << "Mesh added to entity " << static_cast<uint32_t>(entity) << std::endl;
}
```

---

## 最佳实践

### 1. 组件设计
- **保持简单**：组件应该是纯数据
- **避免逻辑**：不要在组件中放方法（除了简单的辅助函数）
- **粒度适中**：不要太大也不要太小

### 2. 系统设计
- **无状态**：系统不应该存储状态
- **专注单一职责**：每个系统只做一件事
- **按功能划分**：渲染、物理、输入等分开

### 3. 性能考虑
- 使用 `view` 而不是手动遍历所有实体
- 频繁访问的组合用 `group`
- 组件数据在内存中连续存储，利用缓存局部性
