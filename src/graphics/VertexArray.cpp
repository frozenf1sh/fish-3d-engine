#include "VertexArray.hpp"

namespace fish::graphics {

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    if (m_id != 0) {
        glDeleteVertexArrays(1, &m_id);
    }
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
{
}

auto VertexArray::operator=(VertexArray&& other) noexcept -> VertexArray&
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteVertexArrays(1, &m_id);
        }
        m_id = std::exchange(other.m_id, 0);
    }
    return *this;
}

void VertexArray::bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::set_index_buffer(const Buffer& buffer)
{
    glVertexArrayElementBuffer(m_id, buffer.id());
}

void VertexArray::set_vertex_buffer(GLuint binding_index, const Buffer& buffer,
                                     GLintptr offset, GLsizei stride)
{
    glVertexArrayVertexBuffer(m_id, binding_index, buffer.id(), offset, stride);
}

void VertexArray::set_attribute(GLuint attrib_index, GLuint binding_index,
                                 AttributeType type, GLint count, GLboolean normalized,
                                 GLuint relative_offset)
{
    glVertexArrayAttribFormat(m_id, attrib_index, count,
                               static_cast<GLenum>(type), normalized, relative_offset);
    glVertexArrayAttribBinding(m_id, attrib_index, binding_index);
}

void VertexArray::enable_attribute(GLuint attrib_index)
{
    glEnableVertexArrayAttrib(m_id, attrib_index);
}

void VertexArray::disable_attribute(GLuint attrib_index)
{
    glDisableVertexArrayAttrib(m_id, attrib_index);
}

void VertexArray::set_attribute_binding(GLuint attrib_index, GLuint binding_index)
{
    glVertexArrayAttribBinding(m_id, attrib_index, binding_index);
}

void VertexArray::set_binding_divisor(GLuint binding_index, GLuint divisor)
{
    glVertexArrayBindingDivisor(m_id, binding_index, divisor);
}

auto VertexArray::id() const -> GLuint
{
    return m_id;
}

} // namespace fish::graphics
