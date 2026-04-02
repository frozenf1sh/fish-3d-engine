# Fish Engine 文档目录

## 文档列表

1. **[阴影映射详解](01-shadow-mapping.md)** - 阴影映射原理与实现
2. **[ECS 架构详解](02-ecs-architecture.md)** - Entt ECS 架构介绍
3. **[场景搭建指南](03-building-scenes.md)** - 如何使用 ECS 搭建场景
4. **[多光源与多模型](04-multi-light-model.md)** - 实现多光源和多模型
5. **[Shader 系统详解](05-shader-system.md)** - Shader 类的使用与扩展

## 快速开始

### 第一次看？

如果你是第一次接触这个项目，推荐按以下顺序阅读：

1. **[ECS 架构详解](02-ecs-architecture.md)** - 了解项目的核心架构
2. **[场景搭建指南](03-building-scenes.md)** - 学习如何创建实体和组件
3. **[阴影映射详解](01-shadow-mapping.md)** - 如果你想了解渲染细节
4. **[Shader 系统详解](05-shader-system.md)** - 如果你想修改或添加着色器
5. **[多光源与多模型](04-multi-light-model.md)** - 如果你想扩展功能

### 我想...

- **了解阴影是怎么工作的** → [阴影映射详解](01-shadow-mapping.md)
- **学习 ECS 架构** → [ECS 架构详解](02-ecs-architecture.md)
- **创建一个场景** → [场景搭建指南](03-building-scenes.md)
- **添加更多光源** → [多光源与多模型](04-multi-light-model.md)
- **修改或添加着色器** → [Shader 系统详解](05-shader-system.md)

## 项目结构速览

```
src/
├── main.cpp                    # 主程序入口
├── graphics/                   # 底层渲染模块
│   ├── Shader.hpp/cpp          # 着色器类
│   ├── Buffer.hpp/cpp          # 缓冲对象
│   ├── VertexArray.hpp/cpp     # VAO
│   ├── Camera.hpp/cpp          # 摄像机
│   ├── Texture2D.hpp/cpp       # 2D 纹理
│   ├── Mesh.hpp/cpp            # 网格
│   ├── Model.hpp/cpp           # 模型加载
│   └── Framebuffer.hpp/cpp     # FBO
└── scene/                      # ECS 场景模块
    ├── Components.hpp           # 所有组件定义
    ├── RenderSystem.hpp/cpp     # 渲染系统
    ├── SceneSerializer.hpp/cpp  # 场景序列化
    └── Scene.hpp                # 统一引入头文件

assets/
├── shaders/                    # GLSL 着色器
│   ├── model.vert/frag         # 模型着色器（带阴影）
│   └── shadow.vert/frag        # 阴影 Pass 着色器
└── models/                     # 模型文件

doc/                            # 你正在这里！
```

## 快捷键

- **WASD** - 移动
- **空格/Shift** - 上升/下降
- **鼠标** - 视角
- **滚轮** - 缩放 FOV
- **F1** - 切换鼠标捕获
- **F5** - 保存场景到 scene.json
- **F9** - 从 scene.json 加载场景
- **ESC** - 退出

## 技术栈

- **OpenGL 4.6 Core** - 图形 API
- **C++23** - 编程语言
- **Entt** - ECS 库
- **nlohmann-json** - JSON 库
- **GLFW3** - 窗口和输入
- **GLAD** - OpenGL 加载
- **GLM** - 数学库
- **tinygltf** - glTF 模型加载
- **stb** - 图像加载
