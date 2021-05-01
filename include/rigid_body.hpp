#ifndef PHYICUIHENG_RIGID_BODY_HPP
#define PHYICUIHENG_RIGID_BODY_HPP

#include <vector>
#include "mass_point.hpp"
#include "stretch_constraint.hpp"

struct rigid_body_t {
    // mass_points の頂点座標を constraints に沿って更新
    void update(float dt) {
        auto predict = mass_points;
        for (int i=1; i<predict.size(); i++) {
            predict[i].velocity.y -= 0.98 * dt;
            predict[i].position += predict[i].velocity * dt;
        }

        for (int i=0; i<100; i++) {
            for (auto const& constraint : constraints) {
                constraint.update(predict);
            }
        }

        for (int i=0; i < mass_points.size(); i++) {
            mass_points[i].velocity = (predict[i].position - mass_points[i].position)/dt;
            mass_points[i].position = predict[i].position;
        }
    }

    std::vector<mass_point_t> mass_points;
    std::vector<stretch_constraint_t> constraints;
};

#endif
