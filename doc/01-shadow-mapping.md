# 阴影映射 (Shadow Mapping) 详解

## 目录
1. [阴影原理概述](#阴影原理概述)
2. [阴影映射算法](#阴影映射算法)
3. [代码实现详解](#代码实现详解)
4. [常见问题与解决方案](#常见问题与解决方案)

---

## 阴影原理概述

### 什么是阴影？

阴影是物体遮挡了光线，使得其他物体无法接收到光照的区域。在 3D 渲染中，阴影对于增强场景的真实感至关重要。

### 阴影的两个关键问题

1. **判断某个点是否在阴影中**
   - 从该点向光源看去，如果中间有其他物体，则该点在阴影中

2. **如何高效计算**
   - 直接计算：对每个像素检查所有物体，效率极低
   - 阴影映射：预处理 + 纹理查询，效率高

---

## 阴影映射算法

### 核心思想

阴影映射是一个**两阶段渲染算法**：

#### 第一阶段：Shadow Pass（阴影渲染）

1. **从光源的视角渲染场景**
   - 只关心深度信息，不关心颜色
   - 记录每个点到光源的距离

2. **存储深度信息到纹理**
   - 这个纹理称为 **Shadow Map（阴影贴图）**
   - 本质是一个深度纹理

#### 第二阶段：Lighting Pass（光照渲染）

1. **从摄像机视角渲染场景**
   - 正常的颜色渲染

2. **对于每个像素**
   - 将像素位置变换到光源空间
   - 查询 Shadow Map
   - 比较深度，判断是否在阴影中

---

## 代码实现详解

### 1. 阴影贴图 FBO 的创建

文件：`src/graphics/Framebuffer.hpp/cpp`

```cpp
class Framebuffer {
public:
    static auto create_depth(unsigned int width, unsigned int height)
        -> Framebuffer;
    // ...
};
```

**关键点：**
- 使用 `glCreateFramebuffers` 创建 DSA 风格的 FBO
- 创建深度纹理作为附着
- 不使用颜色附着（只需要深度）

```cpp
// 创建深度纹理
glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_texture);
glTextureStorage2D(m_depth_texture, 1, GL_DEPTH_COMPONENT24,
                   width, height);

// 附加到 FBO
glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT,
                          m_depth_texture, 0);
```

### 2. 阴影 Pass 着色器

文件：`assets/shaders/shadow.vert`

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uLightSpaceMatrix;

void main()
{
    gl_Position = uLightSpaceMatrix * uModel * vec4(aPos, 1.0);
}
```

**关键概念：**
- `uLightSpaceMatrix` = 光源投影 × 光源观察矩阵
- 将顶点从世界空间变换到光源的裁剪空间

### 3. 主着色器中的阴影计算

文件：`assets/shaders/model.frag`

#### 3.1 光源空间变换

```glsl
// 将片段位置变换到光源空间
vec4 frag_pos_light_space = uLightSpaceMatrix * vec4(FragPos, 1.0);
```

#### 3.2 透视除法和坐标变换

```glsl
// 1. 透视除法：将裁剪空间坐标转换到 NDC [-1, 1]
vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;

// 2. 转换到纹理坐标空间 [0, 1]
// NDC 的 [-1, 1] 范围要变成纹理的 [0, 1] 范围
proj_coords = proj_coords * 0.5 + 0.5;
```

#### 3.3 Shadow Bias（阴影偏移）

**问题：Shadow Acne（阴影痤疮）**

阴影痤疮表现为物体表面出现条纹状的阴影，原因是：

1. Shadow Map 的分辨率有限
2. 多个像素对应 Shadow Map 中的同一个纹素
3. 自遮挡问题

**解决方案：Shadow Bias**

```glsl
float calculate_shadow_bias(vec3 normal, vec3 light_dir)
{
    float bias_min = 0.005;
    float bias_max = 0.05;
    float cos_theta = max(dot(normal, light_dir), 0.0);
    return max(bias_max * (1.0 - cos_theta), bias_min);
}
```

**原理：**
- 基于法线和光线方向的夹角动态调整偏移
- 夹角越大（表面越倾斜），需要的偏移越大
- `bias_max * (1.0 - cos_theta)` 实现这一点

#### 3.4 PCF (Percentage-Closer Filtering) 软阴影

```glsl
float calculate_shadow_pcf(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir)
{
    // ... 坐标变换 ...

    // PCF 采样：4x4 网格，共 16 个采样点
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(uShadowMap, 0);

    // 采样从 -2 到 +1，这样中心点周围有更平衡的分布
    for (int x = -2; x <= 1; ++x)
    {
        for (int y = -2; y <= 1; ++y)
        {
            float pcf_depth = texture(uShadowMap, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += (proj_coords.z - bias > pcf_depth) ? 1.0 : 0.0;
        }
    }

    // 取平均值：16个采样点 / 16
    shadow /= 16.0;

    return shadow;
}
```

**原理：**
- 不只是比较单个深度值，而是比较周围的多个采样点
- 统计有多少比例的采样点在阴影中
- 结果是 0.0（完全照亮）到 1.0（完全阴影）之间的值

### 4. C++ 中的两阶段渲染

文件：`src/main.cpp`

#### 4.1 光源矩阵设置

```cpp
// 方向光使用正交投影
float ortho_left = -10.0f;
float ortho_right = 10.0f;
float ortho_bottom = -10.0f;
float ortho_top = 10.0f;
float ortho_near = 1.0f;
float ortho_far = 20.0f;

glm::mat4 light_projection = glm::ortho(
    ortho_left, ortho_right,
    ortho_bottom, ortho_top,
    ortho_near, ortho_far);

glm::mat4 light_view = glm::lookAt(
    light_pos, light_target,
    glm::vec3(0.0f, 1.0f, 0.0f));

glm::mat4 light_space_matrix = light_projection * light_view;
```

**为什么方向光用正交投影？**
- 方向光的光线是平行的
- 透视投影会有近大远小，不适合方向光
- 正交投影保持平行关系

#### 4.2 Shadow Pass

```cpp
// 绑定阴影 FBO
shadow_fbo.bind();
glViewport(0, 0, shadow_fbo.width(), shadow_fbo.height());
glClear(GL_DEPTH_BUFFER_BIT);

// 使用阴影着色器
shadow_shader.bind();
shadow_shader.set_uniform("uLightSpaceMatrix", light_space_matrix);

// 绘制场景到阴影贴图
// ...
```

#### 4.3 Lighting Pass

```cpp
// 恢复默认视口
shadow_fbo.unbind();
glfwGetFramebufferSize(window, &width, &height);
glViewport(0, 0, width, height);

// 清除颜色和深度缓冲
glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 使用主着色器
shader.bind();

// 设置阴影相关参数
shader.set_uniform("uLightSpaceMatrix", light_space_matrix);
shadow_fbo.bind_depth_texture(1);
shader.set_uniform("uShadowMap", 1);

// 绘制场景
// ...
```

---

## 常见问题与解决方案

### 问题 1：阴影痤疮 (Shadow Acne)

**症状：** 物体表面出现条纹状的错误阴影

**原因：**
- Shadow Map 分辨率有限
- 自遮挡
- 浮点精度问题

**解决方案：**
- 使用 Shadow Bias（已实现）
- 增加 Shadow Map 分辨率
- 渲染背面到 Shadow Map（正面剔除）

### 问题 2：Peter Panning（悬浮）

**症状：** 物体看起来像是悬浮在地面上

**原因：** Shadow Bias 过大

**解决方案：** 调整 `bias_min` 和 `bias_max` 参数

### 问题 3：阴影锯齿严重

**症状：** 阴影边缘有明显的锯齿

**原因：** Shadow Map 分辨率不足

**解决方案：**
- 使用 PCF（已实现 4x4）
- 增加 Shadow Map 分辨率
- 使用更高级的滤波（VSM、ESM 等）

### 问题 4：远处没有阴影

**症状：** 距离光源远处的物体没有阴影

**原因：** 正交投影的范围太小

**解决方案：** 增大 `ortho_left/right/bottom/top` 的值

---

## 进阶阅读

- **Variance Shadow Maps (VSM)** - 使用深度方差，支持更好的滤波
- **Exponential Shadow Maps (ESM)** - 指数深度表示
- **Cascaded Shadow Maps (CSM)** - 多级阴影贴图，解决大场景问题
- **Screen-Space Soft Shadows (SSSS)** - 屏幕空间软阴影
