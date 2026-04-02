#include <cstdio>
#include <cstdlib>
#include <array>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <entt/entt.hpp>

#include "graphics/Shader.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Model.hpp"
#include "graphics/Texture2D.hpp"
#include "graphics/Framebuffer.hpp"

#include "scene/Scene.hpp"

using namespace fish::graphics;
using namespace fish::scene;

// 全局状态
struct Context {
  Camera camera{glm::vec3(0.0f, 2.0f, 6.0f)};
  bool first_mouse = true;
  float last_x = 400.0f;
  float last_y = 300.0f;
  float delta_time = 0.0f;
  float last_frame = 0.0f;
  GLFWwindow *window = nullptr;
} g_context;

void glfw_error_callback(int error, const char *description) {
  std::fprintf(stderr, "GLFW Error [%d]: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
    static bool cursor_enabled = false;
    cursor_enabled = !cursor_enabled;
    glfwSetInputMode(window, GLFW_CURSOR,
                     cursor_enabled ? GLFW_CURSOR_NORMAL
                                    : GLFW_CURSOR_DISABLED);
  }

  // F5: 保存场景
  if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
    std::printf("Saving scene to scene.json...\n");
  }

  // F9: 加载场景
  if (key == GLFW_KEY_F9 && action == GLFW_PRESS) {
    std::printf("Loading scene from scene.json...\n");
  }
}

void mouse_callback(GLFWwindow *window, double xpos_in, double ypos_in) {
  float xpos = static_cast<float>(xpos_in);
  float ypos = static_cast<float>(ypos_in);

  if (g_context.first_mouse) {
    g_context.last_x = xpos;
    g_context.last_y = ypos;
    g_context.first_mouse = false;
  }

  float x_offset = xpos - g_context.last_x;
  float y_offset = g_context.last_y - ypos;

  g_context.last_x = xpos;
  g_context.last_y = ypos;

  g_context.camera.process_mouse_movement(x_offset, y_offset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  g_context.camera.process_mouse_scroll(static_cast<float>(yoffset));
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    g_context.camera.process_keyboard(GLFW_KEY_W, g_context.delta_time);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    g_context.camera.process_keyboard(GLFW_KEY_S, g_context.delta_time);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    g_context.camera.process_keyboard(GLFW_KEY_A, g_context.delta_time);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    g_context.camera.process_keyboard(GLFW_KEY_D, g_context.delta_time);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    g_context.camera.process_keyboard(GLFW_KEY_SPACE, g_context.delta_time);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    g_context.camera.process_keyboard(GLFW_KEY_LEFT_SHIFT,
                                      g_context.delta_time);
}

/// @brief 创建地板实体
void create_floor_entity(entt::registry& registry) {
  auto entity = registry.create();
  registry.emplace<TagComponent>(entity, "Floor");

  // 地板位置
  auto& transform = registry.emplace<TransformComponent>(entity);
  transform.position = glm::vec3(0.0f, -0.5f, 0.0f);

  // 创建地板模型（简单的平面）
  // 注意：这里我们用一个空模型，在 main 循环中单独处理地板
  // 或者可以创建一个真正的地板 Model
}

/// @brief 创建狐狸实体
auto create_fox_entity(entt::registry& registry, std::shared_ptr<Model> model)
    -> entt::entity
{
  auto entity = registry.create();
  registry.emplace<TagComponent>(entity, "Fox");

  auto& transform = registry.emplace<TransformComponent>(entity);
  transform.position = glm::vec3(0.0f, -0.45f, 0.0f);
  transform.scale = glm::vec3(0.01f);

  registry.emplace<MeshComponent>(entity, std::move(model), "assets/models/Fox.gltf");

  return entity;
}

/// @brief 创建光源实体
auto create_light_entity(entt::registry& registry,
                         const glm::vec3& color,
                         const glm::vec3& direction)
    -> entt::entity
{
  auto entity = registry.create();
  registry.emplace<TagComponent>(entity, "DirectionalLight");
  registry.emplace<TransformComponent>(entity);
  registry.emplace<LightComponent>(entity, color, direction, 1.0f);
  return entity;
}

int main() {
  std::printf("Initializing Fish Engine...\n");

  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit()) {
    std::fprintf(stderr, "Failed to initialize GLFW!\n");
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  GLFWwindow *window = glfwCreateWindow(
      800, 600, "Fish Engine - ECS Architecture", nullptr, nullptr);

  if (!window) {
    std::fprintf(stderr, "Failed to create GLFW window!\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  g_context.window = window;

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // 隐藏并捕获鼠标
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::fprintf(stderr, "Failed to initialize GLAD!\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // 设置初始视口
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  std::printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
  std::printf("Renderer: %s\n", glGetString(GL_RENDERER));
  std::printf("Vendor: %s\n", glGetString(GL_VENDOR));
  std::printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  // 开启深度测试
  glEnable(GL_DEPTH_TEST);

  {
    // 作用域块：所有 OpenGL 资源在此作用域内创建和销毁

    //======================================================================
    // 1. 创建 ECS 注册表
    //======================================================================
    entt::registry registry;
    SceneSerializer serializer(registry);

    //======================================================================
    // 2. 创建着色器
    //======================================================================

    // 主着色器（带阴影计算）
    auto shader_result = Shader::from_file("assets/shaders/model.vert",
                                           "assets/shaders/model.frag");
    if (!shader_result) {
      std::fprintf(stderr, "%s\n", shader_result.error().c_str());
      return EXIT_FAILURE;
    }
    auto shader = std::move(*shader_result);

    // 阴影着色器（只输出深度）
    auto shadow_shader_result = Shader::from_file("assets/shaders/shadow.vert",
                                                  "assets/shaders/shadow.frag");
    if (!shadow_shader_result) {
      std::fprintf(stderr, "%s\n", shadow_shader_result.error().c_str());
      return EXIT_FAILURE;
    }
    auto shadow_shader = std::move(*shadow_shader_result);

    //======================================================================
    // 3. 创建阴影贴图 FBO
    //======================================================================

    const unsigned int SHADOW_WIDTH = 2048;
    const unsigned int SHADOW_HEIGHT = 2048;
    auto shadow_fbo = Framebuffer::create_depth(SHADOW_WIDTH, SHADOW_HEIGHT);
    std::printf("Shadow map created: %dx%d\n", SHADOW_WIDTH, SHADOW_HEIGHT);

    //======================================================================
    // 4. 加载模型和创建场景
    //======================================================================

    // 加载狐狸模型
    std::printf("Loading model...\n");
    auto model_result = Model::from_file("assets/models/Fox.gltf");
    if (!model_result) {
      std::fprintf(stderr, "%s\n", model_result.error().c_str());
      return EXIT_FAILURE;
    }
    auto fox_model = std::make_shared<Model>(std::move(*model_result));
    std::printf("Model loaded! %zu meshes\n", fox_model->get_meshes().size());

    // 创建场景实体
    create_fox_entity(registry, fox_model);

    // 方向光 - 从上方打下来
    glm::vec3 light_pos = glm::vec3(5.0f, 12.0f, 5.0f);
    glm::vec3 light_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 light_dir = glm::normalize(light_pos - light_target);
    glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);

    create_light_entity(registry, light_color, light_dir);

    //======================================================================
    // 5. 创建地板（保持独立，因为它是简单的几何体）
    //======================================================================

    struct FloorVertex {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 tex_coord;
    };

    const std::array floor_vertices = {
        FloorVertex{{-10.0f, 0.0f, -10.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        FloorVertex{{10.0f, 0.0f, -10.0f}, {0.0f, 1.0f, 0.0f}, {10.0f, 0.0f}},
        FloorVertex{{10.0f, 0.0f, 10.0f}, {0.0f, 1.0f, 0.0f}, {10.0f, 10.0f}},
        FloorVertex{{-10.0f, 0.0f, 10.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 10.0f}},
    };

    const std::array<unsigned int, 6> floor_indices = {0, 1, 2, 2, 3, 0};

    auto floor_vbo = Buffer::create(BufferType::Vertex, floor_vertices.data(),
                                    floor_vertices.size());
    auto floor_ebo = Buffer::create(BufferType::Index, floor_indices.data(),
                                    floor_indices.size());

    VertexArray floor_vao;
    constexpr GLsizei floor_stride = sizeof(FloorVertex);
    floor_vao.set_vertex_buffer(0, floor_vbo, 0, floor_stride);
    floor_vao.set_index_buffer(floor_ebo);

    floor_vao.set_attribute(0, 0, AttributeType::Float, 3, GL_FALSE,
                            offsetof(FloorVertex, position));
    floor_vao.enable_attribute(0);

    floor_vao.set_attribute(1, 0, AttributeType::Float, 3, GL_FALSE,
                            offsetof(FloorVertex, normal));
    floor_vao.enable_attribute(1);

    floor_vao.set_attribute(2, 0, AttributeType::Float, 2, GL_FALSE,
                            offsetof(FloorVertex, tex_coord));
    floor_vao.enable_attribute(2);

    glm::mat4 floor_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));

    //======================================================================
    // 6. 主循环
    //======================================================================

    std::printf("\n=== 控制说明 ===\n");
    std::printf("WASD: 移动\n");
    std::printf("空格/Shift: 上升/下降\n");
    std::printf("鼠标: 视角\n");
    std::printf("滚轮: 缩放 FOV\n");
    std::printf("F1: 切换鼠标捕获\n");
    std::printf("F5: 保存场景\n");
    std::printf("F9: 加载场景\n");
    std::printf("ESC: 退出\n\n");
    std::printf("=== ECS 架构特性 ===\n");
    std::printf("- Entt ECS 系统\n");
    std::printf("- Scene Serialization (JSON)\n");
    std::printf("- Directional Shadow Mapping\n");
    std::printf("- PCF Soft Shadows\n\n");

    bool save_scene = false;
    bool load_scene = false;

    while (!glfwSetWindowShouldClose(window, GLFW_FALSE),
           !glfwWindowShouldClose(window)) {
      // 计算 delta time
      float current_frame = static_cast<float>(glfwGetTime());
      g_context.delta_time = current_frame - g_context.last_frame;
      g_context.last_frame = current_frame;

      process_input(window);
      glfwPollEvents();

      // 检查场景保存/加载
      if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !save_scene) {
        save_scene = true;
        std::printf("Saving scene to scene.json...\n");
        if (serializer.serialize("scene.json")) {
          std::printf("Scene saved successfully!\n");
        }
      }
      if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE) {
        save_scene = false;
      }

      if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS && !load_scene) {
        load_scene = true;
        std::printf("Loading scene from scene.json...\n");
        if (std::filesystem::exists("scene.json")) {
          if (serializer.deserialize("scene.json")) {
            std::printf("Scene loaded successfully!\n");
          }
        } else {
          std::printf("scene.json not found, creating default scene...\n");
          // 重新创建默认场景
          registry.clear();
          create_fox_entity(registry, fox_model);
          create_light_entity(registry, light_color, light_dir);
        }
      }
      if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_RELEASE) {
        load_scene = false;
      }

      //==================================================================
      // Pass 1: Shadow Pass（阴影渲染 Pass）
      //==================================================================

      // 设置光源的正交投影矩阵
      float ortho_left = -10.0f;
      float ortho_right = 10.0f;
      float ortho_bottom = -10.0f;
      float ortho_top = 10.0f;
      float ortho_near = 1.0f;
      float ortho_far = 20.0f;

      glm::mat4 light_projection =
          glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top,
                     ortho_near, ortho_far);

      glm::mat4 light_view =
          glm::lookAt(light_pos, light_target, glm::vec3(0.0f, 1.0f, 0.0f));

      glm::mat4 light_space_matrix = light_projection * light_view;

      // 阴影 Pass - 先渲染 ECS 中的实体
      shadow_fbo.bind();
      glViewport(0, 0, shadow_fbo.width(), shadow_fbo.height());
      glClear(GL_DEPTH_BUFFER_BIT);

      shadow_shader.bind();
      shadow_shader.set_uniform("uLightSpaceMatrix", light_space_matrix);

      // 渲染 ECS 实体到阴影贴图
      auto shadow_view = registry.view<TransformComponent, MeshComponent>();
      for (auto entity : shadow_view) {
        auto& transform = shadow_view.get<TransformComponent>(entity);
        auto& mesh_comp = shadow_view.get<MeshComponent>(entity);

        if (!mesh_comp.model) continue;

        shadow_shader.set_uniform("uModel", transform.get_model_matrix());
        for (const auto& mesh : mesh_comp.model->get_meshes()) {
          mesh.draw();
        }
      }

      // 渲染地板到阴影贴图
      shadow_shader.set_uniform("uModel", floor_model_matrix);
      floor_vao.bind();
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(floor_indices.size()),
                     GL_UNSIGNED_INT, nullptr);

      shadow_fbo.unbind();

      //==================================================================
      // Pass 2: Lighting Pass（光照渲染 Pass）
      //==================================================================

      glfwGetFramebufferSize(window, &width, &height);
      glViewport(0, 0, width, height);

      glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader.bind();

      // 设置 VP 矩阵
      float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
      glm::mat4 view_matrix = g_context.camera.get_view_matrix();
      glm::mat4 projection_matrix = g_context.camera.get_projection_matrix(aspect_ratio);

      shader.set_uniform("uView", view_matrix);
      shader.set_uniform("uProjection", projection_matrix);
      shader.set_uniform("uViewPos", g_context.camera.get_position());

      // 设置光源参数
      shader.set_uniform("uLightPos", light_pos);
      shader.set_uniform("uLightColor", light_color);
      shader.set_uniform("uLightDir", light_dir);

      // 设置阴影相关参数
      shader.set_uniform("uLightSpaceMatrix", light_space_matrix);
      shadow_fbo.bind_depth_texture(1);
      shader.set_uniform("uShadowMap", 1);

      // 使用 RenderSystem 渲染 ECS 实体
      RenderSystem::render(registry, shader, g_context.camera, aspect_ratio);

      // 绘制地板
      shader.set_uniform("uModel", floor_model_matrix);
      shader.set_uniform("uBaseColor", glm::vec3(0.6f, 0.6f, 0.6f));
      shader.set_uniform("uMetallic", 0.0f);
      shader.set_uniform("uRoughness", 1.0f);
      shader.set_uniform("uHasTexture", false);

      floor_vao.bind();
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(floor_indices.size()),
                     GL_UNSIGNED_INT, nullptr);

      // 交换缓冲区
      glfwSwapBuffers(window);
    }
  }

  // 清理回调以防止退出时段错误
  glfwSetKeyCallback(window, nullptr);
  glfwSetCursorPosCallback(window, nullptr);
  glfwSetScrollCallback(window, nullptr);
  glfwSetFramebufferSizeCallback(window, nullptr);

  glfwDestroyWindow(window);
  glfwTerminate();

  std::printf("Engine shutdown complete.\n");
  return EXIT_SUCCESS;
}
