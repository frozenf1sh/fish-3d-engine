#include <cstdio>
#include <cstdlib>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/Shader.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Model.hpp"
#include "graphics/Texture2D.hpp"
#include "graphics/Framebuffer.hpp"

using namespace fish::graphics;

// 全局状态
struct Context {
    Camera camera{glm::vec3(0.0f, 2.0f, 6.0f)};
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

/// @brief 绘制场景（用于阴影 Pass 和最终 Pass）
/// @param shader 要使用的着色器
/// @param model_matrix 模型矩阵
/// @param model 狐狸模型
/// @param floor_vao 地板 VAO
/// @param floor_index_count 地板索引数量
void draw_scene(const Shader& shader, const glm::mat4& model_matrix,
                const Model& model, const VertexArray& floor_vao,
                size_t floor_index_count)
{
    // 绘制狐狸模型
    for (const auto& mesh : model.get_meshes()) {
        const auto& material = mesh.get_material();

        // 只在非阴影 Pass 时设置材质属性
        // 阴影 Pass 的着色器只需要位置
        shader.set_uniform("uModel", model_matrix);
        mesh.draw();
    }

    // 绘制地板
    glm::mat4 floor_model = glm::mat4(1.0f);
    floor_model = glm::translate(floor_model, glm::vec3(0.0f, -0.5f, 0.0f));
    shader.set_uniform("uModel", floor_model);
    floor_vao.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(floor_index_count), GL_UNSIGNED_INT, nullptr);
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
        "Fish Engine - Directional Shadow Mapping",
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

        //======================================================================
        // 1. 创建着色器
        //======================================================================

        // 主着色器（带阴影计算）
        auto shader_result = Shader::from_file(
            "assets/shaders/model.vert",
            "assets/shaders/model.frag"
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

        // 阴影着色器（只输出深度）
        auto shadow_shader_result = Shader::from_file(
            "assets/shaders/shadow.vert",
            "assets/shaders/shadow.frag"
        );
        if (!shadow_shader_result) {
            std::fprintf(stderr, "%s\n", shadow_shader_result.error().c_str());
            // 清理回调
            glfwSetKeyCallback(window, nullptr);
            glfwSetCursorPosCallback(window, nullptr);
            glfwSetScrollCallback(window, nullptr);
            glfwSetFramebufferSizeCallback(window, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }
        auto shadow_shader = std::move(*shadow_shader_result);

        //======================================================================
        // 2. 创建阴影贴图 FBO
        //======================================================================

        const unsigned int SHADOW_WIDTH = 2048;
        const unsigned int SHADOW_HEIGHT = 2048;
        auto shadow_fbo = Framebuffer::create_depth(SHADOW_WIDTH, SHADOW_HEIGHT);
        std::printf("Shadow map created: %dx%d\n", SHADOW_WIDTH, SHADOW_HEIGHT);

        //======================================================================
        // 3. 加载模型和创建地板
        //======================================================================

        // 加载狐狸模型
        std::printf("Loading model...\n");
        auto model_result = Model::from_file("assets/models/Fox.gltf");
        if (!model_result) {
            std::fprintf(stderr, "%s\n", model_result.error().c_str());
            // 清理回调
            glfwSetKeyCallback(window, nullptr);
            glfwSetCursorPosCallback(window, nullptr);
            glfwSetScrollCallback(window, nullptr);
            glfwSetFramebufferSizeCallback(window, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }
        auto model = std::move(*model_result);
        std::printf("Model loaded! %zu meshes\n", model.get_meshes().size());

        // 创建地板平面顶点数据
        struct FloorVertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 tex_coord;
        };

        const std::array floor_vertices = {
            FloorVertex{ { -10.0f, 0.0f, -10.0f }, { 0.0f, 1.0f, 0.0f }, {  0.0f,  0.0f } },
            FloorVertex{ {  10.0f, 0.0f, -10.0f }, { 0.0f, 1.0f, 0.0f }, { 10.0f,  0.0f } },
            FloorVertex{ {  10.0f, 0.0f,  10.0f }, { 0.0f, 1.0f, 0.0f }, { 10.0f, 10.0f } },
            FloorVertex{ { -10.0f, 0.0f,  10.0f }, { 0.0f, 1.0f, 0.0f }, {  0.0f, 10.0f } },
        };

        const std::array<unsigned int, 6> floor_indices = {
            0, 1, 2,
            2, 3, 0
        };

        // 创建地板的 Buffer 和 VAO
        auto floor_vbo = Buffer::create(BufferType::Vertex, floor_vertices.data(), floor_vertices.size());
        auto floor_ebo = Buffer::create(BufferType::Index, floor_indices.data(), floor_indices.size());

        VertexArray floor_vao;
        constexpr GLsizei floor_stride = sizeof(FloorVertex);
        floor_vao.set_vertex_buffer(0, floor_vbo, 0, floor_stride);
        floor_vao.set_index_buffer(floor_ebo);

        // Position attribute
        floor_vao.set_attribute(0, 0, AttributeType::Float, 3, GL_FALSE, offsetof(FloorVertex, position));
        floor_vao.enable_attribute(0);

        // Normal attribute
        floor_vao.set_attribute(1, 0, AttributeType::Float, 3, GL_FALSE, offsetof(FloorVertex, normal));
        floor_vao.enable_attribute(1);

        // TexCoord attribute
        floor_vao.set_attribute(2, 0, AttributeType::Float, 2, GL_FALSE, offsetof(FloorVertex, tex_coord));
        floor_vao.enable_attribute(2);

        //======================================================================
        // 4. 光源参数
        //======================================================================

        // 方向光 - 从这个位置看向原点
        glm::vec3 light_pos = glm::vec3(3.0f, 4.0f, 2.0f);
        glm::vec3 light_target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 light_dir = glm::normalize(light_target - light_pos);
        glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);

        //======================================================================
        // 5. 主循环
        //======================================================================

        std::printf("\n=== 控制说明 ===\n");
        std::printf("WASD: 移动\n");
        std::printf("空格/Shift: 上升/下降\n");
        std::printf("鼠标: 视角\n");
        std::printf("滚轮: 缩放 FOV\n");
        std::printf("F1: 切换鼠标捕获\n");
        std::printf("ESC: 退出\n\n");
        std::printf("=== 阴影映射特性 ===\n");
        std::printf("- Directional Light (方向光)\n");
        std::printf("- PCF Soft Shadows (4x4采样软阴影)\n");
        std::printf("- Shadow Bias (阴影偏移防止痤疮)\n\n");

        while (!glfwWindowShouldClose(window))
        {
            // 计算 delta time
            float current_frame = static_cast<float>(glfwGetTime());
            g_context.delta_time = current_frame - g_context.last_frame;
            g_context.last_frame = current_frame;

            process_input(window);
            glfwPollEvents();

            //==================================================================
            // Pass 1: Shadow Pass（阴影渲染 Pass）
            //==================================================================

            // 1.1 设置光源的正交投影矩阵（方向光用正交投影）
            // 正交投影覆盖的范围需要足够大，包含场景中所有投射阴影的物体
            float ortho_left = -15.0f;
            float ortho_right = 15.0f;
            float ortho_bottom = -15.0f;
            float ortho_top = 15.0f;
            float ortho_near = 1.0f;
            float ortho_far = 30.0f;

            glm::mat4 light_projection = glm::ortho(
                ortho_left, ortho_right, ortho_bottom, ortho_top, ortho_near, ortho_far
            );

            // 1.2 设置光源的观察矩阵
            glm::mat4 light_view = glm::lookAt(light_pos, light_target, glm::vec3(0.0f, 1.0f, 0.0f));

            // 1.3 光源空间矩阵 = 投影 * 观察
            glm::mat4 light_space_matrix = light_projection * light_view;

            // 1.4 绑定阴影 FBO，设置视口
            shadow_fbo.bind();
            glViewport(0, 0, shadow_fbo.width(), shadow_fbo.height());
            glClear(GL_DEPTH_BUFFER_BIT);

            // 1.5 使用阴影着色器，只渲染深度
            shadow_shader.bind();
            shadow_shader.set_uniform("uLightSpaceMatrix", light_space_matrix);

            // 狐狸模型矩阵
            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
            model_matrix = glm::scale(model_matrix, glm::vec3(0.01f));

            // 1.6 绘制场景（狐狸 + 地板）到阴影贴图
            draw_scene(shadow_shader, model_matrix, model, floor_vao, floor_indices.size());

            //==================================================================
            // Pass 2: Lighting Pass（光照渲染 Pass）
            //==================================================================

            // 2.1 解绑 FBO，恢复默认视口
            shadow_fbo.unbind();
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);

            // 2.2 清除颜色和深度缓冲
            glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 2.3 使用主着色器
            shader.bind();

            // 2.4 设置 MVP 矩阵
            float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
            glm::mat4 view_matrix = g_context.camera.get_view_matrix();
            glm::mat4 projection_matrix = g_context.camera.get_projection_matrix(aspect_ratio);

            shader.set_uniform("uModel", model_matrix);
            shader.set_uniform("uView", view_matrix);
            shader.set_uniform("uProjection", projection_matrix);

            // 2.5 设置光照参数
            shader.set_uniform("uLightPos", light_pos);
            shader.set_uniform("uViewPos", g_context.camera.get_position());
            shader.set_uniform("uLightColor", light_color);
            shader.set_uniform("uLightDir", light_dir);

            // 2.6 设置阴影相关参数
            shader.set_uniform("uLightSpaceMatrix", light_space_matrix);
            shadow_fbo.bind_depth_texture(1);  // 纹理单元 1
            shader.set_uniform("uShadowMap", 1);  // 告诉着色器用纹理单元 1

            // 2.7 绘制场景（狐狸 + 地板）- 这次带纹理和阴影
            for (const auto& mesh : model.get_meshes()) {
                const auto& material = mesh.get_material();
                shader.set_uniform("uBaseColor", material.base_color);
                shader.set_uniform("uMetallic", material.metallic);
                shader.set_uniform("uRoughness", material.roughness);
                shader.set_uniform("uHasTexture", material.base_color_texture != nullptr);

                // 绑定漫反射纹理到纹理单元 0
                if (material.base_color_texture) {
                    material.base_color_texture->bind(0);
                }
                shader.set_uniform("uBaseColorTexture", 0);

                mesh.draw();
            }

            // 2.8 绘制地板（简单白色）
            glm::mat4 floor_model = glm::mat4(1.0f);
            floor_model = glm::translate(floor_model, glm::vec3(0.0f, -0.5f, 0.0f));
            shader.set_uniform("uModel", floor_model);
            shader.set_uniform("uBaseColor", glm::vec3(0.6f, 0.6f, 0.6f));
            shader.set_uniform("uHasTexture", false);

            floor_vao.bind();
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(floor_indices.size()), GL_UNSIGNED_INT, nullptr);

            // 2.9 交换缓冲区
            glfwSwapBuffers(window);
        }

        // shader 和 model 在此作用域结束时析构
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
