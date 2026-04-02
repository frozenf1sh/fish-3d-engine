#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace fish::graphics {

auto Shader::from_file(std::string_view vertex_path, std::string_view fragment_path)
    -> std::expected<Shader, std::string>
{
    auto read_file = [](std::string_view path) -> std::expected<std::string, std::string> {
        std::ifstream file(path.data());
        if (!file.is_open()) {
            return std::unexpected(std::string("Failed to open file: ") + path.data());
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };

    auto vertex_source = read_file(vertex_path);
    if (!vertex_source) {
        return std::unexpected(vertex_source.error());
    }

    auto fragment_source = read_file(fragment_path);
    if (!fragment_source) {
        return std::unexpected(fragment_source.error());
    }

    return from_source(*vertex_source, *fragment_source);
}

auto Shader::from_source(std::string_view vertex_source, std::string_view fragment_source)
    -> std::expected<Shader, std::string>
{
    Shader shader;

    auto vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (!vertex_shader) {
        return std::unexpected(vertex_shader.error());
    }

    auto fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (!fragment_shader) {
        glDeleteShader(*vertex_shader);
        return std::unexpected(fragment_shader.error());
    }

    auto program = link_program(*vertex_shader, *fragment_shader);

    glDeleteShader(*vertex_shader);
    glDeleteShader(*fragment_shader);

    if (!program) {
        return std::unexpected(program.error());
    }

    shader.m_id = *program;
    return shader;
}

Shader::~Shader()
{
    if (m_id != 0) {
        glDeleteProgram(m_id);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
{
}

auto Shader::operator=(Shader&& other) noexcept -> Shader&
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteProgram(m_id);
        }
        m_id = std::exchange(other.m_id, 0);
    }
    return *this;
}

void Shader::bind() const
{
    glUseProgram(m_id);
}

auto Shader::id() const -> GLuint
{
    return m_id;
}

void Shader::set_uniform(std::string_view name, int value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniform1i(m_id, location, value);
}

void Shader::set_uniform(std::string_view name, float value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniform1f(m_id, location, value);
}

void Shader::set_uniform(std::string_view name, const glm::vec2& value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniform2fv(m_id, location, 1, &value[0]);
}

void Shader::set_uniform(std::string_view name, const glm::vec3& value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniform3fv(m_id, location, 1, &value[0]);
}

void Shader::set_uniform(std::string_view name, const glm::vec4& value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniform4fv(m_id, location, 1, &value[0]);
}

void Shader::set_uniform(std::string_view name, const glm::mat2& value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniformMatrix2fv(m_id, location, 1, GL_FALSE, &value[0][0]);
}

void Shader::set_uniform(std::string_view name, const glm::mat3& value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniformMatrix3fv(m_id, location, 1, GL_FALSE, &value[0][0]);
}

void Shader::set_uniform(std::string_view name, const glm::mat4& value) const
{
    GLint location = glGetUniformLocation(m_id, name.data());
    glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, &value[0][0]);
}

auto Shader::compile_shader(GLenum type, std::string_view source)
    -> std::expected<GLuint, std::string>
{
    GLuint shader = glCreateShader(type);
    const char* source_ptr = source.data();
    GLint source_length = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &source_ptr, &source_length);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::string log(log_length, ' ');
        glGetShaderInfoLog(shader, log_length, nullptr, log.data());
        glDeleteShader(shader);

        std::string shader_type = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        return std::unexpected(std::string("SHADER_COMPILATION_ERROR [") + shader_type + "]:\n" + log);
    }

    return shader;
}

auto Shader::link_program(GLuint vertex_shader, GLuint fragment_shader)
    -> std::expected<GLuint, std::string>
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        std::string log(log_length, ' ');
        glGetProgramInfoLog(program, log_length, nullptr, log.data());
        glDeleteProgram(program);
        return std::unexpected(std::string("PROGRAM_LINK_ERROR:\n") + log);
    }

    return program;
}

} // namespace fish::graphics
