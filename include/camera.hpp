#ifndef PHYICUIHENG_CAMERA_HPP
#define PHYICUIHENG_CAMERA_HPP

#include <numbers>
#include <glm/glm.hpp>

struct GLFWwindow;

struct camera_t {
    explicit camera_t();
    void update(GLFWwindow*);

    glm::mat4 projection() const;
    glm::mat4 view() const;
private:
    glm::mat4 m_view;
    glm::vec3 m_position = glm::vec3(0, 0, 5);
    float m_horizontal_angle = std::numbers::pi_v<float>;
    float m_vertical_angle = 0.0f;

    double m_last_time;
};

#endif
