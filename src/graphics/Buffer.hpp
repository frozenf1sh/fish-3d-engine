#pragma once

#include <cstddef>
#include <utility>

#include <glad/glad.h>

namespace fish::graphics {

enum class BufferType {
    Vertex = GL_ARRAY_BUFFER,
    Index = GL_ELEMENT_ARRAY_BUFFER,
};

enum class BufferUsage {
    Static = GL_STATIC_DRAW,
    Dynamic = GL_DYNAMIC_DRAW,
    Stream = GL_STREAM_DRAW,
};

class Buffer {
public:
    Buffer() = default;

    template <typename T>
    static auto create(BufferType type, const T* data, size_t count,
                       BufferUsage usage = BufferUsage::Static) -> Buffer;

    static auto create(BufferType type, const void* data, size_t size_bytes,
                       BufferUsage usage = BufferUsage::Static) -> Buffer;

    ~Buffer();

    Buffer(const Buffer&) = delete;
    auto operator=(const Buffer&) -> Buffer& = delete;

    Buffer(Buffer&& other) noexcept;
    auto operator=(Buffer&& other) noexcept -> Buffer&;

    template <typename T>
    void set_sub_data(const T* data, size_t count, size_t offset = 0) const;

    void set_sub_data(const void* data, size_t size_bytes, size_t offset_bytes = 0) const;

    void bind(BufferType type) const;
    void bind_base(BufferType type, GLuint index) const;

    [[nodiscard]] auto id() const -> GLuint;
    [[nodiscard]] auto size() const -> size_t;

private:
    Buffer(GLuint id, size_t size);

    GLuint m_id = 0;
    size_t m_size = 0;
};

template <typename T>
auto Buffer::create(BufferType type, const T* data, size_t count, BufferUsage usage) -> Buffer
{
    return create(type, static_cast<const void*>(data), count * sizeof(T), usage);
}

template <typename T>
void Buffer::set_sub_data(const T* data, size_t count, size_t offset) const
{
    set_sub_data(data, count * sizeof(T), offset * sizeof(T));
}

} // namespace fish::graphics
