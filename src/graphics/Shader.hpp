#pragma once

#include <expected>
#include <string>
#include <string_view>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace fish::graphics {

class Shader {
public:
    static auto from_file(std::string_view vertex_path, std::string_view fragment_path)
        -> std::expected<Shader, std::string>;

    static auto from_source(std::string_view vertex_source, std::string_view fragment_source)
        -> std::expected<Shader, std::string>;

    ~Shader();

    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;

    Shader(Shader&& other) noexcept;
    auto operator=(Shader&& other) noexcept -> Shader&;

    void bind() const;
    [[nodiscard]] auto id() const -> GLuint;

    void set_uniform(std::string_view name, int value) const;
    void set_uniform(std::string_view name, float value) const;
    void set_uniform(std::string_view name, const glm::vec2& value) const;
    void set_uniform(std::string_view name, const glm::vec3& value) const;
    void set_uniform(std::string_view name, const glm::vec4& value) const;
    void set_uniform(std::string_view name, const glm::mat2& value) const;
    void set_uniform(std::string_view name, const glm::mat3& value) const;
    void set_uniform(std::string_view name, const glm::mat4& value) const;

private:
    Shader() = default;

    static auto compile_shader(GLenum type, std::string_view source)
        -> std::expected<GLuint, std::string>;

    static auto link_program(GLuint vertex_shader, GLuint fragment_shader)
        -> std::expected<GLuint, std::string>;

    GLuint m_id = 0;
};

} // namespace fish::graphics
