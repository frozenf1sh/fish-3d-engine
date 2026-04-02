#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fish::graphics {

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    [[nodiscard]] auto get_view_matrix() const -> glm::mat4;
    [[nodiscard]] auto get_projection_matrix(float aspect_ratio,
                                               float fov = 45.0f,
                                               float near = 0.1f,
                                               float far = 100.0f) const -> glm::mat4;

    void process_keyboard(int key, float delta_time);
    void process_mouse_movement(float x_offset, float y_offset,
                                 bool constrain_pitch = true);
    void process_mouse_scroll(float y_offset);

    [[nodiscard]] auto get_position() const -> const glm::vec3& { return m_position; }
    [[nodiscard]] auto get_front() const -> const glm::vec3& { return m_front; }
    [[nodiscard]] auto get_fov() const -> float { return m_fov; }

    void set_movement_speed(float speed) { m_movement_speed = speed; }
    void set_mouse_sensitivity(float sensitivity) { m_mouse_sensitivity = sensitivity; }

private:
    void update_camera_vectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;

    float m_yaw;
    float m_pitch;

    float m_movement_speed;
    float m_mouse_sensitivity;
    float m_fov;
};

} // namespace fish::graphics
