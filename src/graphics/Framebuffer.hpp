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

    /// @brief 创建一个包含颜色和深度附件的帧缓冲（用于场景渲染）
    /// @param width 宽度
    /// @param height 高度
    /// @return Framebuffer 对象
    static auto create_color_depth(unsigned int width, unsigned int height) -> Framebuffer;

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

    /// @brief 绑定颜色纹理到指定纹理单元
    void bind_color_texture(unsigned int unit = 0) const;

    /// @brief 重新调整帧缓冲大小
    /// @param width 新宽度
    /// @param height 新高度
    void resize(unsigned int width, unsigned int height);

    [[nodiscard]] auto id() const -> GLuint { return m_id; }
    [[nodiscard]] auto depth_texture_id() const -> GLuint { return m_depth_texture; }
    [[nodiscard]] auto color_texture_id() const -> GLuint { return m_color_texture; }
    [[nodiscard]] auto width() const -> unsigned int { return m_width; }
    [[nodiscard]] auto height() const -> unsigned int { return m_height; }
    [[nodiscard]] auto has_color_attachment() const -> bool { return m_color_texture != 0; }

private:
    Framebuffer() = default;

    /// @brief 销毁所有资源
    void destroy();

    GLuint m_id = 0;
    GLuint m_depth_texture = 0;
    GLuint m_color_texture = 0;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
};

} // namespace fish::graphics
