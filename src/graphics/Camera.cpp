#include "Camera.hpp"

#include <GLFW/glfw3.h>

namespace fish::graphics {

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position)
    , m_front(glm::vec3(0.0f, 0.0f, -1.0f))
    , m_world_up(up)
    , m_yaw(yaw)
    , m_pitch(pitch)
    , m_movement_speed(2.5f)
    , m_mouse_sensitivity(0.1f)
    , m_fov(45.0f)
{
    update_camera_vectors();
}

auto Camera::get_view_matrix() const -> glm::mat4
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

auto Camera::get_projection_matrix(float aspect_ratio, float fov,
                                    float near, float far) const -> glm::mat4
{
    return glm::perspective(glm::radians(fov), aspect_ratio, near, far);
}

void Camera::process_keyboard(int key, float delta_time)
{
    float velocity = m_movement_speed * delta_time;

    switch (key) {
        case GLFW_KEY_W:
            m_position += m_front * velocity;
            break;
        case GLFW_KEY_S:
            m_position -= m_front * velocity;
            break;
        case GLFW_KEY_A:
            m_position -= m_right * velocity;
            break;
        case GLFW_KEY_D:
            m_position += m_right * velocity;
            break;
        case GLFW_KEY_SPACE:
            m_position += m_up * velocity;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            m_position -= m_up * velocity;
            break;
    }
}

void Camera::process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch)
{
    x_offset *= m_mouse_sensitivity;
    y_offset *= m_mouse_sensitivity;

    m_yaw += x_offset;
    m_pitch += y_offset;

    if (constrain_pitch) {
        if (m_pitch > 89.0f) {
            m_pitch = 89.0f;
        }
        if (m_pitch < -89.0f) {
            m_pitch = -89.0f;
        }
    }

    update_camera_vectors();
}

void Camera::process_mouse_scroll(float y_offset)
{
    m_fov -= y_offset;
    if (m_fov < 1.0f) {
        m_fov = 1.0f;
    }
    if (m_fov > 90.0f) {
        m_fov = 90.0f;
    }
}

void Camera::update_camera_vectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_world_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace fish::graphics
