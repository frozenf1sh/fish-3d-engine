#include <cstdio>
#include <cstdlib>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "graphics/Shader.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"

using namespace fish::graphics;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error [%d]: %s\n", error, description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
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
        "Fish Engine",
        nullptr, nullptr
    );

    if (!window)
    {
        std::fprintf(stderr, "Failed to create GLFW window!\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::fprintf(stderr, "Failed to initialize GLAD!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    std::printf("Renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("Vendor: %s\n", glGetString(GL_VENDOR));
    std::printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // 创建着色器
    auto shader_result = Shader::from_file(
        "assets/shaders/triangle.vert",
        "assets/shaders/triangle.frag"
    );
    if (!shader_result) {
        std::fprintf(stderr, "%s\n", shader_result.error().c_str());
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    auto shader = std::move(*shader_result);

    // 三角形顶点数据
    const std::array vertices = {
        Vertex{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        Vertex{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        Vertex{ {  0.0f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::printf("Engine shutdown complete.\n");
    return EXIT_SUCCESS;
}
