#include <cstdio>
#include <cstdlib>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "graphics/Shader.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"
#include "graphics/Camera.hpp"

using namespace fish::graphics;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

// 全局状态
struct Context {
    Camera camera;
    bool first_mouse = true;
    float last_x = 400.0f;
    float last_y = 300.0f;
    float delta_time = 0.0f;
    float last_frame = 0.0f;
    GLFWwindow* window = nullptr;
} g_context;

void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error [%d]: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        static bool cursor_enabled = false;
        cursor_enabled = !cursor_enabled;
        glfwSetInputMode(window, GLFW_CURSOR,
                         cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in)
{
    float xpos = static_cast<float>(xpos_in);
    float ypos = static_cast<float>(ypos_in);

    if (g_context.first_mouse)
    {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_context.camera.process_mouse_scroll(static_cast<float>(yoffset));
}

void process_input(GLFWwindow* window)
{
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
        g_context.camera.process_keyboard(GLFW_KEY_LEFT_SHIFT, g_context.delta_time);
}

int main()
{
    std::printf("Initializing Fish Engine...\n");

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to initialize GLFW!\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(
        800, 600,
        "Fish Engine - 3D Cube",
        nullptr, nullptr
    );

    if (!window)
    {
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

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
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
        // 确保在 glfwTerminate() 之前析构

        // 创建着色器
        auto shader_result = Shader::from_file(
            "assets/shaders/cube.vert",
            "assets/shaders/cube.frag"
        );
        if (!shader_result) {
            std::fprintf(stderr, "%s\n", shader_result.error().c_str());
            // 清理回调
            glfwSetKeyCallback(window, nullptr);
            glfwSetCursorPosCallback(window, nullptr);
            glfwSetScrollCallback(window, nullptr);
            glfwSetFramebufferSizeCallback(window, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }
        auto shader = std::move(*shader_result);

        // 立方体顶点数据（带颜色）
        const std::array vertices = {
            // 前面
            Vertex{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
            Vertex{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
            Vertex{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
            Vertex{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
            Vertex{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
            // 后面
            Vertex{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
            Vertex{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
            Vertex{ {  0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
            Vertex{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
            // 上面
            Vertex{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
            Vertex{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
            Vertex{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
            Vertex{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
            // 下面
            Vertex{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
            Vertex{ {  0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
            Vertex{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
            Vertex{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
            Vertex{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
            Vertex{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
            // 右面
            Vertex{ {  0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
            Vertex{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
            Vertex{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
            Vertex{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },
            Vertex{ {  0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
            // 左面
            Vertex{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
            Vertex{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
            Vertex{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
            Vertex{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
            Vertex{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
            Vertex{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
        };

        // 创建顶点缓冲区
        auto vbo = Buffer::create(BufferType::Vertex, vertices.data(), vertices.size());

        // 创建 VAO
        VertexArray vao;

        // 设置顶点缓冲区绑定 (binding index 0)
        constexpr GLsizei stride = sizeof(Vertex);
        vao.set_vertex_buffer(0, vbo, 0, stride);

        // 设置位置属性 (attrib index 0)
        vao.set_attribute(0, 0, AttributeType::Float, 3, GL_FALSE, offsetof(Vertex, pos));
        vao.enable_attribute(0);

        // 设置颜色属性 (attrib index 1)
        vao.set_attribute(1, 0, AttributeType::Float, 3, GL_FALSE, offsetof(Vertex, color));
        vao.enable_attribute(1);

        // 绑定 VAO 和着色器
        vao.bind();
        shader.bind();

        std::printf("\n=== 控制说明 ===\n");
        std::printf("WASD: 移动\n");
        std::printf("空格/Shift: 上升/下降\n");
        std::printf("鼠标: 视角\n");
        std::printf("滚轮: 缩放 FOV\n");
        std::printf("F1: 切换鼠标捕获\n");
        std::printf("ESC: 退出\n\n");

        while (!glfwWindowShouldClose(window))
        {
            // 计算 delta time
            float current_frame = static_cast<float>(glfwGetTime());
            g_context.delta_time = current_frame - g_context.last_frame;
            g_context.last_frame = current_frame;

            process_input(window);
            glfwPollEvents();

            // 清除颜色和深度缓冲
            glClearColor(0.2f, 0.4f, 0.8f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 获取窗口尺寸
            glfwGetFramebufferSize(window, &width, &height);
            float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

            // 设置 MVP 矩阵
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = g_context.camera.get_view_matrix();
            glm::mat4 projection = g_context.camera.get_projection_matrix(aspect_ratio);

            shader.set_uniform("uModel", model);
            shader.set_uniform("uView", view);
            shader.set_uniform("uProjection", projection);

            // 绘制立方体
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

            glfwSwapBuffers(window);
        }

        // shader, vbo, vao 在此作用域结束时析构
        // 它们的析构函数会在 glfwTerminate() 之前调用
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
