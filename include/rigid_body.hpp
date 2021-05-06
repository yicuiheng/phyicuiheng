#ifndef PHYICUIHENG_RIGID_BODY_HPP
#define PHYICUIHENG_RIGID_BODY_HPP

#include <vector>
#include "mass_point.hpp"
#include "stretch_constraint.hpp"
#include "collision_constraint.hpp"

struct model_t;

struct triangle_t {
    unsigned short p1_idx, p2_idx, p3_idx;
};

struct rigid_body_t {
    // mass_points の頂点座標を constraints に沿って更新
    void update(float dt, model_t& model);

    std::vector<mass_point_t> mass_points;
    std::vector<triangle_t> triangles;
    std::vector<stretch_constraint_t> constraints;
    std::vector<collision_constraint_t> collision_constraints;
};

#endif
