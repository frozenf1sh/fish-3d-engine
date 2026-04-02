#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <glad/glad.h>

#include "Buffer.hpp"

namespace fish::graphics {

enum class AttributeType {
    Float = GL_FLOAT,
    Double = GL_DOUBLE,
    Int = GL_INT,
    UnsignedInt = GL_UNSIGNED_INT,
    Byte = GL_BYTE,
    UnsignedByte = GL_UNSIGNED_BYTE,
    Short = GL_SHORT,
    UnsignedShort = GL_UNSIGNED_SHORT,
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    auto operator=(const VertexArray&) -> VertexArray& = delete;

    VertexArray(VertexArray&& other) noexcept;
    auto operator=(VertexArray&& other) noexcept -> VertexArray&;

    void bind() const;

    void set_index_buffer(const Buffer& buffer);

    void set_vertex_buffer(GLuint binding_index, const Buffer& buffer, GLintptr offset, GLsizei stride);

    void set_attribute(GLuint attrib_index, GLuint binding_index,
                       AttributeType type, GLint count, GLboolean normalized,
                       GLuint relative_offset);

    void enable_attribute(GLuint attrib_index);
    void disable_attribute(GLuint attrib_index);

    void set_attribute_binding(GLuint attrib_index, GLuint binding_index);

    void set_binding_divisor(GLuint binding_index, GLuint divisor);

    [[nodiscard]] auto id() const -> GLuint;

private:
    GLuint m_id = 0;
};

} // namespace fish::graphics
