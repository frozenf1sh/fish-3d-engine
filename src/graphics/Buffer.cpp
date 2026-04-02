#include "Buffer.hpp"

namespace fish::graphics {

auto Buffer::create(BufferType type, const void* data, size_t size_bytes,
                    BufferUsage usage) -> Buffer
{
    GLuint id;
    glCreateBuffers(1, &id);

    GLbitfield flags = 0;
    switch (usage) {
        case BufferUsage::Static:
            flags = GL_DYNAMIC_STORAGE_BIT;
            break;
        case BufferUsage::Dynamic:
            flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;
            break;
        case BufferUsage::Stream:
            flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;
            break;
    }

    glNamedBufferStorage(id, size_bytes, data, flags);

    return Buffer(id, size_bytes);
}

Buffer::~Buffer()
{
    if (m_id != 0) {
        glDeleteBuffers(1, &m_id);
    }
}

Buffer::Buffer(Buffer&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_size(std::exchange(other.m_size, 0))
{
}

auto Buffer::operator=(Buffer&& other) noexcept -> Buffer&
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteBuffers(1, &m_id);
        }
        m_id = std::exchange(other.m_id, 0);
        m_size = std::exchange(other.m_size, 0);
    }
    return *this;
}

void Buffer::set_sub_data(const void* data, size_t size_bytes, size_t offset_bytes) const
{
    glNamedBufferSubData(m_id, offset_bytes, size_bytes, data);
}

void Buffer::bind(BufferType type) const
{
    glBindBuffer(static_cast<GLenum>(type), m_id);
}

void Buffer::bind_base(BufferType type, GLuint index) const
{
    glBindBufferBase(static_cast<GLenum>(type), index, m_id);
}

auto Buffer::id() const -> GLuint
{
    return m_id;
}

auto Buffer::size() const -> size_t
{
    return m_size;
}

Buffer::Buffer(GLuint id, size_t size)
    : m_id(id)
    , m_size(size)
{
}

} // namespace fish::graphics
