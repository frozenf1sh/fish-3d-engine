#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <array>
#include <filesystem>
#include <format>
#include <deque>
#include <string>

#include <portable-file-dialogs.h>

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

// 窗口状态
struct WindowState {
  int x = 100, y = 100;
  int width = 1600, height = 900;
  bool is_fullscreen = false;
} g_window_state;

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
} g_context;

// 日志消息
struct LogMessage {
  enum class Type { Info, Warning, Error };
  Type type;
  std::string message;
};

// 编辑器状态
struct EditorContext {
  entt::entity selected_entity = entt::null;
  std::deque<LogMessage> log_messages;
  bool show_error_popup = false;
  std::string error_message;

  void log(LogMessage::Type type, const std::string& msg) {
    log_messages.push_back({type, msg});
    if (log_messages.size() > 100) {
      log_messages.pop_front();
    }
  }

  void log_info(const std::string& msg) { log(LogMessage::Type::Info, msg); }
  void log_warning(const std::string& msg) { log(LogMessage::Type::Warning, msg); }
  void log_error(const std::string& msg) {
    log(LogMessage::Type::Error, msg);
    show_error_popup = true;
    error_message = msg;
  }
} g_editor_context;

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
  // 禁用独立视口，让所有窗口都在主窗口内
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // 大幅增加字体大小 - 使用默认字体并缩放
  io.Fonts->AddFontDefault();
  io.FontGlobalScale = 2.0f;

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
  // F1, F11 和 ESC 始终响应，不受 ImGui 捕获限制
  if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
    g_context.game_running = !g_context.game_running;
    glfwSetInputMode(window, GLFW_CURSOR,
                     g_context.game_running ? GLFW_CURSOR_DISABLED
                                            : GLFW_CURSOR_NORMAL);
    g_context.first_mouse = true;  // 重置鼠标状态
  }

  if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
    // 切换全屏/窗口模式
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (g_window_state.is_fullscreen) {
      // 切换回窗口模式
      glfwSetWindowMonitor(window, nullptr,
                           g_window_state.x, g_window_state.y,
                           g_window_state.width, g_window_state.height,
                           GLFW_DONT_CARE);
      g_window_state.is_fullscreen = false;
    } else {
      // 保存当前窗口位置和大小
      glfwGetWindowPos(window, &g_window_state.x, &g_window_state.y);
      glfwGetWindowSize(window, &g_window_state.width, &g_window_state.height);
      // 切换到全屏
      glfwSetWindowMonitor(window, monitor,
                           0, 0, mode->width, mode->height,
                           mode->refreshRate);
      g_window_state.is_fullscreen = true;
    }
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

  // 其他按键只有在游戏运行时才处理
  if (!g_context.game_running) {
    return;
  }
}

void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in) {
  // 只有在游戏模式下才处理鼠标移动
  if (!g_context.game_running) {
    return;
  }

  float xpos = static_cast<float>(xpos_in);
  float ypos = static_cast<float>(ypos_in);

  if (g_context.first_mouse) {
    g_context.last_x = xpos;
    g_context.last_y = ypos;
    g_context.first_mouse = false;
    // 第一次回调后重置到中心
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);
    double center_x = static_cast<double>(win_width) / 2.0;
    double center_y = static_cast<double>(win_height) / 2.0;
    glfwSetCursorPos(window, center_x, center_y);
    g_context.last_x = static_cast<float>(center_x);
    g_context.last_y = static_cast<float>(center_y);
    return;
  }

  // 计算鼠标移动偏移
  float x_offset = xpos - g_context.last_x;
  float y_offset = g_context.last_y - ypos;

  // 处理相机移动
  if (std::abs(x_offset) > 0.001f || std::abs(y_offset) > 0.001f) {
    g_context.camera.process_mouse_movement(x_offset, y_offset);
  }

  // 重置鼠标到中心
  int win_width, win_height;
  glfwGetWindowSize(window, &win_width, &win_height);
  double center_x = static_cast<double>(win_width) / 2.0;
  double center_y = static_cast<double>(win_height) / 2.0;
  glfwSetCursorPos(window, center_x, center_y);
  g_context.last_x = static_cast<float>(center_x);
  g_context.last_y = static_cast<float>(center_y);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
  // 只有在游戏模式下才处理滚轮
  if (!g_context.game_running) {
    return;
  }
  g_context.camera.process_mouse_scroll(static_cast<float>(yoffset));
}

void process_input(GLFWwindow *window) {
  // 只有在游戏模式下才处理游戏输入，忽略 ImGui 的捕获
  if (!g_context.game_running) {
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
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);  // 启动时最大化

  // 创建窗口（窗口化全屏）
  GLFWwindow *window = glfwCreateWindow(
      1600, 900, "Fish Engine - ECS Architecture", nullptr, nullptr);

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

    // 是否第一次运行
    bool first_run = true;

    while (!glfwWindowShouldClose(window)) {
      // 计算 delta time
      float current_frame = static_cast<float>(glfwGetTime());
      g_context.delta_time = current_frame - g_context.last_frame;
      g_context.last_frame = current_frame;

      // 处理输入（先轮询事件）
      glfwPollEvents();
      process_input(window);

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
          if (ImGui::MenuItem("Save Scene...")) {
            auto selection = pfd::save_file("Save Scene", "", {"JSON Files", "*.json"}).result();
            if (!selection.empty()) {
              if (serializer.serialize(selection)) {
                g_editor_context.log_info(std::format("Saved scene to: {}", selection));
              } else {
                g_editor_context.log_error(std::format("Failed to save scene to: {}", selection));
              }
            }
          }
          if (ImGui::MenuItem("Load Scene...")) {
            auto selection = pfd::open_file("Load Scene", "", {"JSON Files", "*.json"}).result();
            if (!selection.empty()) {
              std::string path = selection[0];
              if (std::filesystem::exists(path)) {
                if (serializer.deserialize(path)) {
                  g_editor_context.log_info(std::format("Loaded scene from: {}", path));
                } else {
                  g_editor_context.log_error(std::format("Failed to load scene from: {}", path));
                }
              } else {
                g_editor_context.log_error(std::format("File not found: {}", path));
              }
            }
          }
          ImGui::Separator();
          if (ImGui::MenuItem("Import Model...")) {
            auto selection = pfd::open_file("Import Model", "", {"GLTF Files", "*.gltf *.glb"}).result();
            if (!selection.empty()) {
              std::string path = selection[0];
              try {
                auto model_result = Model::from_file(path);
                if (model_result) {
                  auto new_model = std::make_shared<Model>(std::move(*model_result));
                  auto new_entity = registry.create();

                  // 提取文件名作为标签
                  std::filesystem::path fs_path(path);
                  std::string name = fs_path.stem().string();

                  registry.emplace<TagComponent>(new_entity, name);
                  registry.emplace<TransformComponent>(new_entity);
                  registry.emplace<MeshComponent>(new_entity, new_model, path);

                  g_editor_context.log_info(std::format("Imported model: {}", name));
                } else {
                  g_editor_context.log_error(std::format("Failed to load model: {}", model_result.error()));
                }
              } catch (const std::exception& e) {
                g_editor_context.log_error(std::format("Exception importing model: {}", e.what()));
              }
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

      // Hierarchy 层级面板
      ImGui::Begin("Hierarchy");

      // 右键点击空白处创建空实体
      if (ImGui::BeginPopupContextWindow("HierarchyContextMenu")) {
        if (ImGui::MenuItem("Create Empty Entity")) {
          auto new_entity = registry.create();
          registry.emplace<TagComponent>(new_entity, "New Entity");
          registry.emplace<TransformComponent>(new_entity);
        }
        ImGui::EndPopup();
      }

      // 遍历所有实体
      auto all_entities = registry.view<entt::entity>();
      for (auto entity : all_entities) {
        std::string entity_name;
        if (auto* tag = registry.try_get<TagComponent>(entity)) {
          entity_name = tag->tag;
        } else {
          entity_name = std::format("Entity {}", static_cast<uint32_t>(entity));
        }

        // 使用 Selectable 显示实体
        bool is_selected = (g_editor_context.selected_entity == entity);
        if (ImGui::Selectable(entity_name.c_str(), is_selected)) {
          g_editor_context.selected_entity = entity;
        }

        // 右键点击实体
        if (ImGui::BeginPopupContextItem(std::format("EntityContext_{}", static_cast<uint32_t>(entity)).c_str())) {
          if (ImGui::MenuItem("Delete Entity")) {
            if (g_editor_context.selected_entity == entity) {
              g_editor_context.selected_entity = entt::null;
            }
            registry.destroy(entity);
          }
          ImGui::EndPopup();
        }
      }

      ImGui::End();

      // Inspector 属性面板
      ImGui::Begin("Inspector");

      if (g_editor_context.selected_entity != entt::null && registry.valid(g_editor_context.selected_entity)) {
        auto selected = g_editor_context.selected_entity;

        // 显示 TagComponent
        if (auto* tag = registry.try_get<TagComponent>(selected)) {
          if (ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
            static char tag_buffer[256];
            strncpy(tag_buffer, tag->tag.c_str(), sizeof(tag_buffer) - 1);
            if (ImGui::InputText("Name", tag_buffer, sizeof(tag_buffer))) {
              tag->tag = tag_buffer;
            }
          }
        }

        // 显示并编辑 TransformComponent
        if (auto* transform = registry.try_get<TransformComponent>(selected)) {
          if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", &transform->position.x, 0.1f);

            // 使用欧拉角编辑旋转
            glm::vec3 euler = glm::degrees(transform->get_rotation_euler());
            if (ImGui::DragFloat3("Rotation", &euler.x, 1.0f, -180.0f, 180.0f)) {
              transform->set_rotation_euler(glm::radians(euler));
            }

            ImGui::DragFloat3("Scale", &transform->scale.x, 0.1f, 0.01f, 10.0f);
          }
        }

        // 显示并编辑 LightComponent
        if (auto* light = registry.try_get<LightComponent>(selected)) {
          if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Color", &light->color.x);
            ImGui::DragFloat3("Direction", &light->direction.x, 0.1f, -1.0f, 1.0f);
            light->direction = glm::normalize(light->direction);
            ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 10.0f);
          }
        }

        // 显示 MeshComponent
        if (auto* mesh = registry.try_get<MeshComponent>(selected)) {
          if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Model: %s", mesh->model_path.c_str());
            if (mesh->model) {
              ImGui::Text("Meshes: %zu", mesh->model->get_meshes().size());
            }
          }
        }

        // Add Component 按钮
        ImGui::Separator();
        if (ImGui::Button("Add Component")) {
          ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
          if (!registry.all_of<LightComponent>(selected)) {
            if (ImGui::MenuItem("Light Component")) {
              registry.emplace<LightComponent>(selected);
            }
          }
          if (!registry.all_of<MeshComponent>(selected)) {
            if (ImGui::MenuItem("Mesh Component")) {
              registry.emplace<MeshComponent>(selected);
            }
          }
          ImGui::EndPopup();
        }
      } else {
        ImGui::Text("No entity selected");
      }

      ImGui::End();

      // Error Popup
      if (g_editor_context.show_error_popup) {
        ImGui::OpenPopup("Error");
        g_editor_context.show_error_popup = false;
      }
      if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Error:");
        ImGui::Separator();
        ImGui::TextWrapped("%s", g_editor_context.error_message.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }

      // Console 面板 - 显示在底部
      if (first_run) {
        first_run = false;
        // 设置 Console 窗口默认位置在底部
        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImVec2 work_size = main_viewport->WorkSize;
        ImVec2 work_pos = main_viewport->WorkPos;
        ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + work_size.y * 0.75f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(work_size.x, work_size.y * 0.25f), ImGuiCond_FirstUseEver);
      }
      ImGui::Begin("Console");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate,
                  ImGui::GetIO().Framerate);
      ImGui::Separator();

      ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
      for (const auto& msg : g_editor_context.log_messages) {
        ImVec4 color;
        switch (msg.type) {
          case LogMessage::Type::Info: color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
          case LogMessage::Type::Warning: color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break;
          case LogMessage::Type::Error: color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break;
        }
        ImGui::TextColored(color, "%s", msg.message.c_str());
      }

      // 自动滚动到底部
      if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
      }
      ImGui::EndChild();

      ImGui::End();

      // Render ImGui
      ImGui::Render();
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
