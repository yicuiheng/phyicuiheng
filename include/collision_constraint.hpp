#ifndef PHYICUIHENG_HPP
#define PHYICUIHENG_HPP

#include "rigid_body.hpp"

struct collision_constraint_t {
    explicit collision_constraint_t(int p_idx) : m_p_idx{p_idx} {}

    void update(std::vector<mass_point_t>& predict_mass_points, std::vector<mass_point_t> const& mass_points) const {
        predict_mass_points[m_p_idx].position = mass_points[m_p_idx].position;
    }
private:
    int m_p_idx;
};

#endif
