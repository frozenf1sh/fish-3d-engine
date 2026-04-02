#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::printf("Engine shutdown complete.\n");
    return EXIT_SUCCESS;
}
