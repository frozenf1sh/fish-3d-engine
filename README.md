# Fish Engine

一个现代 C++ 3D 图形引擎，使用 OpenGL 4.6 核心模式。

## 功能特性

- **ECS 架构** (使用 [Entt](https://github.com/skypjack/entt))
  - 组件：TransformComponent, MeshComponent, LightComponent, TagComponent
  - 系统：RenderSystem（渲染系统）
  - 场景序列化：JSON 格式保存/加载
- **方向光阴影映射 (Directional Shadow Mapping)**
  - PCF (Percentage-Closer Filtering) 软阴影（4x4 采样）
  - Shadow Bias 防止阴影痤疮
- **glTF 模型加载** (借助 tinygltf)
- **自动法线计算**（如果模型没有法线）
- **Blinn-Phong 光照模型**
- **FPS 风格摄像机**
- **纹理支持**
- **场景序列化**（F5 保存，F9 加载）

## 文档

详细文档请查看 [doc/](doc/) 目录：

- [阴影映射详解](doc/01-shadow-mapping.md) - 阴影映射原理与实现
- [ECS 架构详解](doc/02-ecs-architecture.md) - Entt ECS 架构介绍
- [场景搭建指南](doc/03-building-scenes.md) - 如何使用 ECS 搭建场景
- [多光源与多模型](doc/04-multi-light-model.md) - 实现多光源和多模型
- [Shader 系统详解](doc/05-shader-system.md) - Shader 类的使用与扩展

## 依赖项

- GLFW3 - 窗口和输入
- GLAD - OpenGL 函数加载
- GLM - 数学库
- tinygltf - glTF 模型加载
- stb - 图像加载
- **Entt** - ECS 库
- **nlohmann-json** - JSON 库

使用 [vcpkg](https://vcpkg.io) 管理依赖。

## 构建

确保已设置 `VCPKG_ROOT` 环境变量：

```bash
export VCPKG_ROOT=/path/to/vcpkg
```

使用提供的构建脚本：

```bash
# Debug 构建
./build.sh

# Release 构建
./build.sh -r

# 清理并构建
./build.sh -c

# 构建并运行
./build.sh --run
```

或手动使用 CMake：

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## 控制

- **WASD** - 移动
- **空格/Shift** - 上升/下降
- **鼠标** - 视角
- **滚轮** - 缩放 FOV
- **F1** - 切换鼠标捕获
- **F5** - 保存场景到 scene.json
- **F9** - 从 scene.json 加载场景
- **ESC** - 退出

## 项目结构

```
fish-engine/
├── src/
│   ├── main.cpp
│   ├── graphics/
│   │   ├── Shader.hpp/cpp
│   │   ├── Buffer.hpp/cpp
│   │   ├── VertexArray.hpp/cpp
│   │   ├── Camera.hpp/cpp
│   │   ├── Texture2D.hpp/cpp
│   │   ├── Mesh.hpp/cpp
│   │   ├── Model.hpp/cpp
│   │   └── Framebuffer.hpp/cpp
│   └── scene/
│       ├── Components.hpp
│       ├── RenderSystem.hpp/cpp
│       ├── SceneSerializer.hpp/cpp
│       └── Scene.hpp
├── assets/
│   ├── shaders/
│   │   ├── model.vert/frag
│   │   └── shadow.vert/frag
│   └── models/
│       └── Fox.gltf
├── doc/
│   └── ... (详细文档)
├── CMakeLists.txt
├── build.sh
└── vcpkg.json
```

## 技术细节

- OpenGL 4.6 Core Profile
- C++23
- 使用 Direct State Access (DSA)
- 两阶段渲染（Shadow Pass + Lighting Pass）
- ECS 架构

## License

MIT
