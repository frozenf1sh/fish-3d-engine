#include "Texture2D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <utility>

namespace fish::graphics {

auto Texture2D::from_file(std::string_view path, bool srgb)
    -> std::expected<Texture2D, std::string>
{
    int width, height, channels;
    stbi_uc* data = stbi_load(path.data(), &width, &height, &channels, 0);

    if (!data) {
        return std::unexpected(std::string("Failed to load texture: ") + path.data());
    }

    auto result = from_data(data, width, height, channels, srgb);
    stbi_image_free(data);
    return result;
}

auto Texture2D::from_data(const unsigned char* data, int width, int height,
                            int channels, bool srgb)
    -> std::expected<Texture2D, std::string>
{
    Texture2D texture;
    texture.m_width = width;
    texture.m_height = height;
    texture.m_channels = channels;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture.m_id);

    GLenum internal_format;
    GLenum format;

    switch (channels) {
        case 1:
            internal_format = GL_R8;
            format = GL_RED;
            break;
        case 3:
            internal_format = srgb ? GL_SRGB8 : GL_RGB8;
            format = GL_RGB;
            break;
        case 4:
            internal_format = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            format = GL_RGBA;
            break;
        default:
            glDeleteTextures(1, &texture.m_id);
            return std::unexpected("Unsupported number of channels");
    }

    glTextureStorage2D(texture.m_id, 1, internal_format, width, height);
    glTextureSubImage2D(texture.m_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

    texture.set_filter(TextureFilter::LinearMipmapLinear, TextureFilter::Linear);
    texture.set_wrap(TextureWrap::Repeat, TextureWrap::Repeat);
    texture.generate_mipmap();

    return texture;
}

Texture2D::~Texture2D()
{
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0))
    , m_channels(std::exchange(other.m_channels, 0))
{
}

auto Texture2D::operator=(Texture2D&& other) noexcept -> Texture2D&
{
    if (this != &other) {
        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
        }
        m_id = std::exchange(other.m_id, 0);
        m_width = std::exchange(other.m_width, 0);
        m_height = std::exchange(other.m_height, 0);
        m_channels = std::exchange(other.m_channels, 0);
    }
    return *this;
}

void Texture2D::bind(GLuint unit) const
{
    glBindTextureUnit(unit, m_id);
}

void Texture2D::set_wrap(TextureWrap wrap_s, TextureWrap wrap_t) const
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrap_s));
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrap_t));
}

void Texture2D::set_filter(TextureFilter min_filter, TextureFilter mag_filter) const
{
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min_filter));
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag_filter));
}

void Texture2D::generate_mipmap() const
{
    glGenerateTextureMipmap(m_id);
}

auto Texture2D::id() const -> GLuint
{
    return m_id;
}

} // namespace fish::graphics
