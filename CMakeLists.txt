cmake_minimum_required(VERSION 3.30)
project(fish-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# tinygltf is header-only
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h" REQUIRED)

# stb
find_package(Stb REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/graphics/Shader.cpp
    src/graphics/Buffer.cpp
    src/graphics/VertexArray.cpp
    src/graphics/Camera.cpp
    src/graphics/Texture2D.cpp
    src/graphics/Mesh.cpp
    src/graphics/Model.cpp
    src/graphics/Framebuffer.cpp
    src/scene/RenderSystem.cpp
    src/scene/SceneSerializer.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${TINYGLTF_INCLUDE_DIRS}
    ${Stb_INCLUDE_DIR}
)

# 复制资源目录到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad::glad
    glm::glm
    EnTT::EnTT
    nlohmann_json::nlohmann_json
    imgui::imgui
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
