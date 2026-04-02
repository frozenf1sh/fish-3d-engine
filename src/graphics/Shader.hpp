#pragma once

#include <expected>
#include <string>
#include <string_view>

#include <glad/glad.h>

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

private:
    Shader() = default;

    static auto compile_shader(GLenum type, std::string_view source)
        -> std::expected<GLuint, std::string>;

    static auto link_program(GLuint vertex_shader, GLuint fragment_shader)
        -> std::expected<GLuint, std::string>;

    GLuint m_id = 0;
};

} // namespace fish::graphics
