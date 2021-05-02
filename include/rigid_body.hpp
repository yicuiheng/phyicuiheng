#ifndef PHYICUIHENG_RIGID_BODY_HPP
#define PHYICUIHENG_RIGID_BODY_HPP

#include <vector>
#include "mass_point.hpp"
#include "stretch_constraint.hpp"

struct rigid_body_t {
    // mass_points の頂点座標を constraints に沿って更新
    void update(float dt) {
        auto predict = mass_points;
        for (auto&& p : predict) {
            if (p.weight == 0.0f)
                continue;
            const glm::vec3 gravity = {0.f, -0.98f, 0.f};
            glm::vec3 force =
                gravity / p.weight + // 重力
                -0.1f * p.velocity +
                glm::vec3{(std::rand() % 30) / 90.f, (std::rand() % 30) / 90.f, (std::rand() % 30) / 90.f}; // 空気抵抗

            p.velocity += force * p.weight * dt;
            p.position += p.velocity * dt;
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
