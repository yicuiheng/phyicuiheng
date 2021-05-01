#ifndef PHYICUIHENG_CONSTRAINT_HPP
#define PHYICUIHENG_CONSTRAINT_HPP

#include <vector>
#include <glm/glm.hpp>
#include "mass_point.hpp"

struct stretch_constraint_t {
    explicit stretch_constraint_t(std::vector<mass_point_t> const& mass_points, size_t p1_idx, size_t p2_idx) :
        m_p1_idx{p1_idx}, m_p2_idx{p2_idx}
    {
        auto const& p1 = mass_points[p1_idx];
        auto const& p2 = mass_points[p2_idx];
        m_initial_distance = glm::length(p1.position - p2.position);
    }

    void update(std::vector<mass_point_t>& predict_mass_points) const {
        auto& p1 = predict_mass_points[m_p1_idx];
        auto& p2 = predict_mass_points[m_p2_idx];
        glm::vec3 diff = p1.position - p2.position;
        float distance = glm::length(diff);
        glm::vec3 dir = glm::normalize(diff);

        glm::vec3 dp1 = - p1.weight / (p1.weight + p2.weight) * (distance - m_initial_distance) * dir;
        glm::vec3 dp2 = p2.weight / (p1.weight + p2.weight) * (distance - m_initial_distance) * dir;

        p1.position += dp1;
        p2.position += dp2;
    }
private:
    float stiffness = 0.1f;
    size_t m_p1_idx;
    size_t m_p2_idx;
    float m_initial_distance;
};

#endif
