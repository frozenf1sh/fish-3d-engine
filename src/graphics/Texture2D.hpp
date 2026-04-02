#pragma once

#include <expected>
#include <string>
#include <string_view>

#include <glad/glad.h>

namespace fish::graphics {

enum class TextureFormat {
    R8 = GL_R8,
    RGB8 = GL_RGB8,
    RGBA8 = GL_RGBA8,
    SRGB8 = GL_SRGB8,
    SRGBA8 = GL_SRGB8_ALPHA8,
};

enum class TextureWrap {
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    ClampToBorder = GL_CLAMP_TO_BORDER,
};

enum class TextureFilter {
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
    NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
    LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
};

class Texture2D {
public:
    static auto from_file(std::string_view path, bool srgb = true)
        -> std::expected<Texture2D, std::string>;

    static auto from_data(const unsigned char* data, int width, int height,
                          int channels, bool srgb = true)
        -> std::expected<Texture2D, std::string>;

    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    auto operator=(const Texture2D&) -> Texture2D& = delete;

    Texture2D(Texture2D&& other) noexcept;
    auto operator=(Texture2D&& other) noexcept -> Texture2D&;

    void bind(GLuint unit = 0) const;

    void set_wrap(TextureWrap wrap_s, TextureWrap wrap_t) const;
    void set_filter(TextureFilter min_filter, TextureFilter mag_filter) const;
    void generate_mipmap() const;

    [[nodiscard]] auto id() const -> GLuint;
    [[nodiscard]] auto width() const -> int { return m_width; }
    [[nodiscard]] auto height() const -> int { return m_height; }
    [[nodiscard]] auto channels() const -> int { return m_channels; }

private:
    Texture2D() = default;

    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};

} // namespace fish::graphics
