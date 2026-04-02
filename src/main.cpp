#include <cstdio>
#include <cstdlib>
#include <array>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
  bool imgui_wants_mouse = false;
  bool imgui_wants_keyboard = false;
  bool game_running = false;  // 游戏是否在运行（鼠标捕获状态）
  bool scene_viewport_hovered = false;  // 鼠标是否悬停在视口上
  bool scene_viewport_focused = false;  // 视口是否有焦点
} g_context;

// ImGui 暗黑主题
void set_imgui_dark_theme() {
  ImGuiStyle& style = ImGui::GetStyle();

  // 窗口圆角
  style.WindowRounding = 8.0f;
  style.FrameRounding = 4.0f;
  style.GrabRounding = 4.0f;
  style.PopupRounding = 4.0f;
  style.ScrollbarRounding = 4.0f;
  style.TabRounding = 4.0f;

  // 颜色主题 - 现代暗黑
  ImVec4* colors = style.Colors;
  colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
  colors[ImGuiCol_ChildBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.00f);
  colors[ImGuiCol_PopupBg]                = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
  colors[ImGuiCol_Border]                 = ImVec4(0.28f, 0.28f, 0.28f, 0.30f);
  colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg]                = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_FrameBgActive]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
  colors[ImGuiCol_TitleBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
  colors[ImGuiCol_TitleBgActive]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed]        = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark]              = ImVec4(0.80f, 0.80f, 0.80f, 0.31f);
  colors[ImGuiCol_SliderGrab]             = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
  colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
  colors[ImGuiCol_Button]                 = ImVec4(0.30f, 0.30f, 0.30f, 0.80f);
  colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.40f, 0.40f, 0.80f);
  colors[ImGuiCol_ButtonActive]           = ImVec4(0.50f, 0.50f, 0.50f, 0.80f);
  colors[ImGuiCol_Header]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_HeaderHovered]          = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
  colors[ImGuiCol_HeaderActive]           = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
  colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.30f);
  colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.55f, 0.55f, 0.55f, 0.30f);
  colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.70f, 0.30f);
  colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.00f);
  colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.55f, 0.55f, 0.55f, 0.30f);
  colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.70f, 0.70f, 0.70f, 0.30f);
  colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.18f, 0.18f, 0.86f);
  colors[ImGuiCol_TabHovered]             = ImVec4(0.38f, 0.38f, 0.38f, 0.86f);
  colors[ImGuiCol_TabActive]              = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
  colors[ImGuiCol_TabUnfocused]           = ImVec4(0.12f, 0.12f, 0.12f, 0.97f);
  colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_DockingPreview]         = ImVec4(0.50f, 0.50f, 0.50f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
  colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
  colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
  colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
  colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
  colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

// 初始化 ImGui
void init_imgui(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // 设置现代暗黑主题
  set_imgui_dark_theme();

  // 初始化 ImGui GLFW 和 OpenGL3 绑定
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

// 关闭 ImGui
void shutdown_imgui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void glfw_error_callback(int error, const char *description) {
  std::fprintf(stderr, "GLFW Error [%d]: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
  glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int, int action, int) {
  // F1 和 ESC 始终响应，不受 ImGui 捕获限制
  if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
    g_context.game_running = !g_context.game_running;
    glfwSetInputMode(window, GLFW_CURSOR,
                     g_context.game_running ? GLFW_CURSOR_DISABLED
                                            : GLFW_CURSOR_NORMAL);
    g_context.first_mouse = true;  // 重置鼠标状态
  }

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    if (g_context.game_running) {
      // ESC 在游戏模式下释放鼠标
      g_context.game_running = false;
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      // 在编辑模式下退出程序
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  }

  // 其他按键只有在游戏运行且 ImGui 不捕获时才处理
  if (g_context.imgui_wants_keyboard || !g_context.game_running) {
    return;
  }
}

void mouse_callback(GLFWwindow*, double xpos_in, double ypos_in) {
  if (g_context.imgui_wants_mouse || !g_context.game_running) {
    return;
  }

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

void scroll_callback(GLFWwindow*, double, double yoffset) {
  if (g_context.imgui_wants_mouse || !g_context.game_running) {
    return;
  }
  g_context.camera.process_mouse_scroll(static_cast<float>(yoffset));
}

void process_input(GLFWwindow *window) {
  if (g_context.imgui_wants_keyboard || !g_context.game_running) {
    return;
  }

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

  // 默认显示鼠标（编辑模式）
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
    // 3.5. 创建场景渲染 FBO
    //======================================================================

    unsigned int scene_width = 1280;
    unsigned int scene_height = 720;
    auto scene_fbo = Framebuffer::create_color_depth(scene_width, scene_height);
    std::printf("Scene framebuffer created: %dx%d\n", scene_width, scene_height);

    //======================================================================
    // 4. 初始化 ImGui
    //======================================================================

    init_imgui(window);
    std::printf("ImGui initialized\n");

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
    std::printf("- PCF Soft Shadows\n");
    std::printf("- ImGui Editor\n\n");

    bool save_scene = false;
    bool load_scene = false;

    // 视口尺寸
    ImVec2 viewport_size(0, 0);
    bool viewport_size_changed = false;

    while (!glfwWindowShouldClose(window)) {
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
      // Pass 2: Scene Pass（渲染到场景 FBO）
      //==================================================================

      // 检查视口尺寸是否变化
      if (viewport_size_changed && viewport_size.x > 0 && viewport_size.y > 0) {
        scene_fbo.resize(static_cast<unsigned int>(viewport_size.x),
                        static_cast<unsigned int>(viewport_size.y));
        viewport_size_changed = false;
      }

      // 渲染到场景 FBO
      scene_fbo.bind();
      glViewport(0, 0, scene_fbo.width(), scene_fbo.height());
      glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader.bind();

      // 设置 VP 矩阵
      float scene_aspect_ratio = static_cast<float>(scene_fbo.width()) /
                                static_cast<float>(scene_fbo.height());
      glm::mat4 view_matrix = g_context.camera.get_view_matrix();
      glm::mat4 projection_matrix =
          g_context.camera.get_projection_matrix(scene_aspect_ratio);

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
      RenderSystem::render(registry, shader, g_context.camera, scene_aspect_ratio);

      // 绘制地板
      shader.set_uniform("uModel", floor_model_matrix);
      shader.set_uniform("uBaseColor", glm::vec3(0.6f, 0.6f, 0.6f));
      shader.set_uniform("uMetallic", 0.0f);
      shader.set_uniform("uRoughness", 1.0f);
      shader.set_uniform("uHasTexture", false);

      floor_vao.bind();
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(floor_indices.size()),
                     GL_UNSIGNED_INT, nullptr);

      scene_fbo.unbind();

      //==================================================================
      // Pass 3: ImGui Pass（编辑器界面）
      //==================================================================

      // 开始 ImGui 帧
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // 更新 ImGui 输入状态
      ImGuiIO& io = ImGui::GetIO();
      g_context.imgui_wants_mouse = io.WantCaptureMouse;
      g_context.imgui_wants_keyboard = io.WantCaptureKeyboard;

      // 创建全屏 DockSpace
      ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      ImGui::Begin("Fish Engine Editor DockSpace", nullptr, window_flags);
      ImGui::PopStyleVar(2);

      // 菜单栏
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("Save Scene", "F5")) {
            serializer.serialize("scene.json");
          }
          if (ImGui::MenuItem("Load Scene", "F9")) {
            if (std::filesystem::exists("scene.json")) {
              serializer.deserialize("scene.json");
            }
          }
          ImGui::Separator();
          if (ImGui::MenuItem("Exit", "ESC")) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

      // DockSpace
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

      ImGui::End();

      // 工具栏
      ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
      if (g_context.game_running) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Stop")) {
          g_context.game_running = false;
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        ImGui::PopStyleColor(2);
      } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        if (ImGui::Button("Play")) {
          g_context.game_running = true;
          g_context.first_mouse = true;
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        ImGui::PopStyleColor(2);
      }
      ImGui::SameLine();
      ImGui::Text(g_context.game_running ? "Mode: Game" : "Mode: Edit");
      ImGui::End();

      // Scene Viewport 窗口
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      ImGui::Begin("Scene Viewport");

      // 更新视口悬停和焦点状态
      g_context.scene_viewport_hovered = ImGui::IsWindowHovered();
      g_context.scene_viewport_focused = ImGui::IsWindowFocused();

      // 点击视口且不在游戏模式时进入游戏模式
      if (g_context.scene_viewport_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !g_context.game_running) {
        g_context.game_running = true;
        g_context.first_mouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      }

      // 获取窗口内容区域大小
      ImVec2 avail_size = ImGui::GetContentRegionAvail();
      if (avail_size.x != viewport_size.x || avail_size.y != viewport_size.y) {
        viewport_size = avail_size;
        viewport_size_changed = true;
      }

      // 显示场景纹理
      if (scene_fbo.has_color_attachment()) {
        ImTextureID tex_id = (ImTextureID)(intptr_t)scene_fbo.color_texture_id();
        // 翻转 Y 轴（OpenGL 纹理坐标与 ImGui 不同）
        ImGui::Image(tex_id, avail_size, ImVec2(0, 1), ImVec2(1, 0));
      }

      ImGui::End();
      ImGui::PopStyleVar();

      // Properties 窗口
      ImGui::Begin("Properties");
      ImGui::Text("Entities: %zu",
                  static_cast<size_t>(std::distance(registry.view<entt::entity>().begin(),
                                                    registry.view<entt::entity>().end())));
      ImGui::Separator();
      ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
                  g_context.camera.get_position().x,
                  g_context.camera.get_position().y,
                  g_context.camera.get_position().z);
      ImGui::Separator();
      ImGui::Text("Controls:");
      ImGui::BulletText("Click Play or click viewport to enter game mode");
      ImGui::BulletText("F1: Toggle game/edit mode");
      ImGui::BulletText("ESC: Exit game mode / Quit");
      ImGui::BulletText("WASD: Move camera");
      ImGui::BulletText("Mouse: Look around");
      ImGui::End();

      // Render ImGui
      ImGui::Render();
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      // 更新和渲染额外的视口
      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
      }

      // 交换缓冲区
      glfwSwapBuffers(window);
    }

    // 清理 ImGui
    shutdown_imgui();
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
