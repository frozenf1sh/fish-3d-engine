#include "Framebuffer.hpp"

#include <utility>

namespace fish::graphics {

auto Framebuffer::create_depth(unsigned int width, unsigned int height) -> Framebuffer
{
    Framebuffer fbo;
    fbo.m_width = width;
    fbo.m_height = height;

    // 1. 创建帧缓冲对象 (DSA方式)
    glCreateFramebuffers(1, &fbo.m_id);

    // 2. 创建深度纹理 (DSA方式)
    glCreateTextures(GL_TEXTURE_2D, 1, &fbo.m_depth_texture);

    // 3. 为深度纹理分配存储 - 使用24位深度
    glTextureStorage2D(fbo.m_depth_texture, 1, GL_DEPTH_COMPONENT24, width, height);

    // 4. 配置深度纹理参数
    glTextureParameteri(fbo.m_depth_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fbo.m_depth_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 使用 GL_CLAMP_TO_BORDER 并设置边框颜色为1.0
    // 这样超出阴影贴图范围的区域会被认为是完全照亮的
    glTextureParameteri(fbo.m_depth_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(fbo.m_depth_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(fbo.m_depth_texture, GL_TEXTURE_BORDER_COLOR, border_color);

    // 5. 将深度纹理附加到帧缓冲
    glNamedFramebufferTexture(fbo.m_id, GL_DEPTH_ATTACHMENT, fbo.m_depth_texture, 0);

    // 6. 告诉OpenGL我们不使用颜色附件
    glNamedFramebufferDrawBuffer(fbo.m_id, GL_NONE);
    glNamedFramebufferReadBuffer(fbo.m_id, GL_NONE);

    // 7. 检查帧缓冲是否完整
    GLenum status = glCheckNamedFramebufferStatus(fbo.m_id, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        // 这里可以处理错误，不过为了简化我们暂时不抛出异常
        // 实际项目中应该使用 std::expected 或类似机制
    }

    return fbo;
}

Framebuffer::~Framebuffer()
{
    if (m_depth_texture != 0) {
        glDeleteTextures(1, &m_depth_texture);
    }
    if (m_id != 0) {
        glDeleteFramebuffers(1, &m_id);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_depth_texture(std::exchange(other.m_depth_texture, 0))
    , m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0))
{
}

auto Framebuffer::operator=(Framebuffer&& other) noexcept -> Framebuffer&
{
    if (this != &other) {
        if (m_depth_texture != 0) {
            glDeleteTextures(1, &m_depth_texture);
        }
        if (m_id != 0) {
            glDeleteFramebuffers(1, &m_id);
        }
        m_id = std::exchange(other.m_id, 0);
        m_depth_texture = std::exchange(other.m_depth_texture, 0);
        m_width = std::exchange(other.m_width, 0);
        m_height = std::exchange(other.m_height, 0);
    }
    return *this;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind_depth_texture(unsigned int unit) const
{
    glBindTextureUnit(unit, m_depth_texture);
}

} // namespace fish::graphics
