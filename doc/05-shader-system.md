# Shader 系统详解

## 目录
1. [Shader 类解析](#shader-类解析)
2. [如何加载 Shader](#如何加载-shader)
3. [Uniform 管理](#uniform-管理)
4. [扩展 Shader 系统](#扩展-shader-系统)

---

## Shader 类解析

### 完整代码结构

文件：`src/graphics/Shader.hpp`

```cpp
#pragma once

#include <expected>
#include <string>
#include <string_view>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace fish::graphics {

class Shader {
public:
    /// @brief 从文件创建着色器
    /// @param vert_path 顶点着色器路径
    /// @param frag_path 片段着色器路径
    /// @return Shader 对象或错误信息
    static auto from_file(std::string_view vert_path,
                         std::string_view frag_path)
        -> std::expected<Shader, std::string>;

    /// @brief 从源码创建着色器
    /// @param vert_source 顶点着色器源码
    /// @param frag_source 片段着色器源码
    /// @return Shader 对象或错误信息
    static auto from_source(std::string_view vert_source,
                           std::string_view frag_source)
        -> std::expected<Shader, std::string>;

    ~Shader();

    // 禁止拷贝
    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;

    // 允许移动
    Shader(Shader&& other) noexcept;
    auto operator=(Shader&& other) noexcept -> Shader&;

    /// @brief 绑定/激活着色器
    void bind() const;

    /// @brief 解绑着色器
    static void unbind();

    // ============ Uniform 设置 ============

    void set_uniform(std::string_view name, int value) const;
    void set_uniform(std::string_view name, float value) const;
    void set_uniform(std::string_view name, bool value) const;
    void set_uniform(std::string_view name, const glm::vec2& value) const;
    void set_uniform(std::string_view name, const glm::vec3& value) const;
    void set_uniform(std::string_view name, const glm::vec4& value) const;
    void set_uniform(std::string_view name, const glm::mat2& value) const;
    void set_uniform(std::string_view name, const glm::mat3& value) const;
    void set_uniform(std::string_view name, const glm::mat4& value) const;

    [[nodiscard]] auto id() const -> GLuint { return m_program; }

private:
    Shader() = default;

    GLuint m_program = 0;
};

} // namespace fish::graphics
```

### RAII 设计

Shader 类使用 **RAII（资源获取即初始化）** 模式：

```cpp
// 构造时创建资源
Shader shader = std::move(*Shader::from_file(...));

// 析构时自动释放
// ~Shader() { glDeleteProgram(m_program); }
```

### 移动语义

```cpp
// 允许移动，不允许拷贝
Shader(Shader&& other) noexcept;
auto operator=(Shader&& other) noexcept -> Shader&;
```

使用 `std::expected` 和 `std::move` 安全返回：

```cpp
auto result = Shader::from_file("a.vert", "a.frag");
if (!result) {
    std::cerr << result.error() << std::endl;
    return;
}
Shader shader = std::move(*result);
```

---

## 如何加载 Shader

### 从文件加载（推荐）

```cpp
#include "graphics/Shader.hpp"

// 基本用法
auto shader_result = Shader::from_file(
    "assets/shaders/model.vert",
    "assets/shaders/model.frag"
);

if (!shader_result) {
    std::fprintf(stderr, "Failed to load shader: %s\n",
                 shader_result.error().c_str());
    return EXIT_FAILURE;
}

auto shader = std::move(*shader_result);
```

### 从字符串源码加载

```cpp
constexpr std::string_view vert_source = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

constexpr std::string_view frag_source = R"(
#version 460 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

auto shader_result = Shader::from_source(vert_source, frag_source);
```

### 完整的加载流程（内部实现）

文件：`src/graphics/Shader.cpp`

```cpp
// 1. 读取文件
std::string vert_source = read_file(vert_path);
std::string frag_source = read_file(frag_path);

// 2. 编译顶点着色器
GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
const char* vert_cstr = vert_source.c_str();
glShaderSource(vert_shader, 1, &vert_cstr, nullptr);
glCompileShader(vert_shader);

// 3. 检查编译错误
GLint success;
glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
if (!success) {
    GLint log_length;
    glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &log_length);
    std::vector<char> log(log_length);
    glGetShaderInfoLog(vert_shader, log_length, nullptr, log.data());
    return std::unexpected(std::string(log.data()));
}

// 4. 片段着色器同理...

// 5. 链接着色器程序
GLuint program = glCreateProgram();
glAttachShader(program, vert_shader);
glAttachShader(program, frag_shader);
glLinkProgram(program);

// 6. 检查链接错误
glGetProgramiv(program, GL_LINK_STATUS, &success);
if (!success) {
    // 获取错误日志...
}

// 7. 删除着色器对象（不再需要了）
glDeleteShader(vert_shader);
glDeleteShader(frag_shader);
```

---

## Uniform 管理

### 什么是 Uniform？

Uniform 是着色器中的**全局变量**，CPU 设置后在绘制调用中保持不变。

### 设置 Uniform 的方法

```cpp
shader.bind(); // 必须先绑定！

// 基础类型
shader.set_uniform("uIntValue", 42);
shader.set_uniform("uFloatValue", 3.14f);
shader.set_uniform("uBoolValue", true);

// GLM 向量
shader.set_uniform("uVec2", glm::vec2(1.0f, 2.0f));
shader.set_uniform("uVec3", glm::vec3(1.0f, 2.0f, 3.0f));
shader.set_uniform("uVec4", glm::vec4(1.0f, 2.0f, 3.0f, 4.0f));

// GLM 矩阵
shader.set_uniform("uMat2", glm::mat2(1.0f));
shader.set_uniform("uMat3", glm::mat3(1.0f));
shader.set_uniform("uMat4", glm::mat4(1.0f));
```

### Uniform 位置缓存优化

**当前实现的问题：**
```cpp
// 每次都调用 glGetUniformLocation
void set_uniform(std::string_view name, int value) const {
    GLint location = glGetUniformLocation(m_program, name.data());
    glUniform1i(location, value);
}
```

**可以优化为缓存位置：**
```cpp
class Shader {
private:
    std::unordered_map<std::string, GLint> m_uniform_cache;

public:
    void set_uniform(std::string_view name, int value) const {
        // 查找缓存
        auto it = m_uniform_cache.find(std::string(name));
        GLint location;
        if (it != m_uniform_cache.end()) {
            location = it->second;
        } else {
            location = glGetUniformLocation(m_program, name.data());
            m_uniform_cache[std::string(name)] = location;
        }
        glUniform1i(location, value);
    }
};
```

### 着色器中的 Uniform 声明

```glsl
#version 460 core

// 基础类型
uniform int uIntValue;
uniform float uFloatValue;
uniform bool uBoolValue;

// 向量
uniform vec2 uVec2;
uniform vec3 uVec3;
uniform vec4 uVec4;

// 矩阵
uniform mat2 uMat2;
uniform mat3 uMat3;
uniform mat4 uMat4;

// 数组
uniform vec3 uPositions[10];

// 结构体
uniform Light {
    vec3 color;
    vec3 direction;
    float intensity;
};
```

---

## 扩展 Shader 系统

### 添加几何着色器

```cpp
// Shader.hpp
static auto from_file(std::string_view vert_path,
                     std::string_view frag_path,
                     std::optional<std::string_view> geom_path)
    -> std::expected<Shader, std::string>;

// Shader.cpp
auto Shader::from_file(..., std::optional<std::string_view> geom_path) {
    // ... 编译 vert 和 frag ...

    GLuint geom_shader = 0;
    if (geom_path) {
        std::string geom_source = read_file(*geom_path);
        geom_shader = compile_shader(GL_GEOMETRY_SHADER, geom_source);
        glAttachShader(program, geom_shader);
    }

    glLinkProgram(program);

    if (geom_shader) {
        glDeleteShader(geom_shader);
    }

    // ...
}
```

### 添加 Uniform 缓冲区对象 (UBO)

```cpp
// 用于多个着色器共享的 Uniform
class UniformBuffer {
public:
    static auto create(size_t size, int binding_point)
        -> std::expected<UniformBuffer, std::string>;

    void set_data(const void* data, size_t size, size_t offset = 0);
    void bind() const;

private:
    GLuint m_buffer = 0;
    int m_binding_point = 0;
};

// 使用示例：
// 所有着色器共享的摄像机数据
struct CameraData {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
};

auto ubo = UniformBuffer::create(sizeof(CameraData), 0);
CameraData data;
data.view = camera.get_view_matrix();
data.projection = camera.get_projection_matrix();
data.position = camera.get_position();
ubo.set_data(&data, sizeof(data));

// 在着色器中：
layout (std140, binding = 0) uniform CameraData {
    mat4 uView;
    mat4 uProjection;
    vec3 uViewPos;
};
```

### Shader 管理器

```cpp
class ShaderManager {
public:
    auto get_or_load(const std::string& name,
                     const std::string& vert_path,
                     const std::string& frag_path)
        -> Shader* {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            return &it->second;
        }

        auto result = Shader::from_file(vert_path, frag_path);
        if (!result) {
            return nullptr;
        }

        auto [inserted, _] = m_shaders.emplace(name, std::move(*result));
        return &inserted->second;
    }

private:
    std::unordered_map<std::string, Shader> m_shaders;
};

// 使用：
ShaderManager shaders;
auto* model_shader = shaders.get_or_load(
    "model",
    "assets/shaders/model.vert",
    "assets/shaders/model.frag"
);
auto* shadow_shader = shaders.get_or_load(
    "shadow",
    "assets/shaders/shadow.vert",
    "assets/shaders/shadow.frag"
);
```

### 热重载（开发时有用）

```cpp
class Shader {
public:
    // 检查文件是否修改并重新加载
    auto reload() -> bool;

    // 获取文件路径（用于检查修改时间）
    [[nodiscard]] auto vert_path() const -> const std::string& { return m_vert_path; }
    [[nodiscard]] auto frag_path() const -> const std::string& { return m_frag_path; }

private:
    std::string m_vert_path;
    std::string m_frag_path;
    std::filesystem::file_time_type m_vert_last_write;
    std::filesystem::file_time_type m_frag_last_write;
};

// 在主循环中：
if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    shader.reload();
}
```

---

## 最佳实践

### 1. 着色器文件组织

```
assets/shaders/
├── model.vert          # 模型顶点着色器
├── model.frag          # 模型片段着色器
├── shadow.vert         # 阴影顶点着色器
├── shadow.frag         # 阴影片段着色器
├── common.glsl         # 通用函数（include）
└── lighting.glsl       # 光照计算
```

### 2. 使用 #include

```glsl
// common.glsl
#ifndef COMMON_GLSL
#define COMMON_GLSL

float PI = 3.14159265359;

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

#endif

// model.frag
#version 460 core

// 注意：需要用 glslang 或自定义预处理
#include "common.glsl"

void main() {
    float x = saturate(0.5);
}
```

### 3. Uniform 命名约定

```glsl
// 矩阵：u 前缀 + CamelCase
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpaceMatrix;

// 向量/颜色：u 前缀 + CamelCase
uniform vec3 uLightColor;
uniform vec3 uLightDir;
uniform vec3 uViewPos;

// 标量：u 前缀 + CamelCase
uniform float uMetallic;
uniform float uRoughness;
uniform int uShadowMap;
uniform bool uHasTexture;

// 纹理：u 前缀 + 纹理名
uniform sampler2D uBaseColorTexture;
```

### 4. 绑定纹理单元

```cpp
// 纹理单元 0：漫反射纹理
material.base_color_texture->bind(0);
shader.set_uniform("uBaseColorTexture", 0);

// 纹理单元 1：阴影贴图
shadow_fbo.bind_depth_texture(1);
shader.set_uniform("uShadowMap", 1);
```

### 5. 调试技巧

```cpp
// 检查 uniform 是否存在
GLint location = glGetUniformLocation(program, "uSomeUniform");
if (location == -1) {
    // Uniform 不存在，可能是被优化掉了
    // 或者名字拼写错了
    std::cerr << "Warning: uniform 'uSomeUniform' not found!" << std::endl;
}
```
