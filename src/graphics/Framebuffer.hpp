#pragma once

#include <glad/glad.h>

namespace fish::graphics {

class Framebuffer {
public:
    /// @brief 创建一个只含深度附件的帧缓冲（用于阴影贴图）
    /// @param width 宽度
    /// @param height 高度
    /// @return Framebuffer 对象
    static auto create_depth(unsigned int width, unsigned int height) -> Framebuffer;

    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    auto operator=(const Framebuffer&) -> Framebuffer& = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    auto operator=(Framebuffer&& other) noexcept -> Framebuffer&;

    /// @brief 绑定帧缓冲
    void bind() const;

    /// @brief 解绑帧缓冲（绑定默认 FBO）
    void unbind() const;

    /// @brief 绑定深度纹理到指定纹理单元
    void bind_depth_texture(unsigned int unit = 0) const;

    [[nodiscard]] auto id() const -> GLuint { return m_id; }
    [[nodiscard]] auto depth_texture_id() const -> GLuint { return m_depth_texture; }
    [[nodiscard]] auto width() const -> unsigned int { return m_width; }
    [[nodiscard]] auto height() const -> unsigned int { return m_height; }

private:
    Framebuffer() = default;

    GLuint m_id = 0;
    GLuint m_depth_texture = 0;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
};

} // namespace fish::graphics
