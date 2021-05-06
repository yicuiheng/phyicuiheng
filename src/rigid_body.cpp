#include <optional>
#include "rigid_body.hpp"
#include "model.hpp"

// solve a x + b y + c z = d for x, y, z in R
std::optional<glm::vec3> solve_cramer(
    glm::vec3 const& a,
    glm::vec3 const& b,
    glm::vec3 const& c,
    glm::vec3 const& d) {

    float denominator = glm::determinant(glm::mat3(a, b, c));
    float x = glm::determinant(glm::mat3(d, b, c)) / denominator;
    float y = glm::determinant(glm::mat3(a, d, c)) / denominator;
    float z = glm::determinant(glm::mat3(a, b, d)) / denominator;
    return glm::vec3(z, y, z);
}

// 線分 (p_start, p_end) が三角形 (p1, p2, p3) に交差しているか？
// 交差しているなら交わる座標を, 交差してないなら none を返す
std::optional<glm::vec3> collision(
    glm::vec3 const& p_start, glm::vec3 const& p_end,
    glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3) {

    // 交点P があるとすれば
    //   P = p_start + t * (p_end - p_start)
    // なる 0 <= t <= 1 が存在する.
    // また, それは 0 <= u, v <= 1, 0 <= u + v <= 1 なる u, v を用いて
    //   P = p1 + u (p2 - p1) + v (p3 - p1)
    // とも書ける.
    // よって
    //   p_start + t * (p_end - p_start) = p1 + u (p2 - p1) + v (p3 - p1) であるから
    //   u (p2 - p1) + v (p3 - p1) - t * (p_end - p_start) = p_start - p1
    // このような u, v, t はクラメルの公式によって得られる.
    // それらが上記の条件を満たしていれば衝突している！
    auto res = solve_cramer(p2 - p1, p3 - p1, p_start - p_end, p_start - p1);
    if (res == std::nullopt)
        return std::nullopt;
    float u = res->x;
    float v = res->y;
    float t = res->z;

    auto within = [&](float x) {
        return 0.f <= x && x <= 1.f;
    };

    if (within(t) && within(u) && within(v) && within(u+v)) {
        return p_start + t * (p_end - p_start);
    } else {
        return std::nullopt;
    }
}

void rigid_body_t::update(float dt, model_t& model) {
    auto predict = mass_points;
    for (auto&& p : predict) {
        if (p.weight == 0.0f)
            continue;
        const glm::vec3 gravity = {0.f, -0.98f, 0.f};
        glm::vec3 random_force = {
            (std::rand() % 30) / 900.f,
            (std::rand() % 30) / 900.f,
            (std::rand() % 30) / 900.f
        };
        p.velocity += (gravity + random_force * p.weight) * dt;
        p.position += p.velocity * dt;
    }

    assert(collision(
        glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)) != std::nullopt);
    assert(collision(
        glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)) != std::nullopt);
    assert(collision(
        glm::vec3(0.1f, 0.1f, -1.f), glm::vec3(0.1f, 0.1f, 1.f),
        glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)) != std::nullopt);

    // 衝突検出して constraints に衝突用の制約を追加
    for (int p_idx=0; p_idx<mass_points.size(); p_idx++) {
        for (int triangle_idx = 0; triangle_idx < triangles.size(); triangle_idx++) {
            glm::vec3 start_point = mass_points[p_idx].position;
            glm::vec3 end_point = predict[p_idx].position;

            /*
            if (end_point.y < -2.0f) {
                collision_constraints.emplace_back(p_idx);
                model.debug_indices().push_back(p_idx);
                continue;
            } */

            auto const& triangle = triangles[triangle_idx];
            glm::vec3 p1 = mass_points[triangle.p1_idx].position;
            glm::vec3 p2 = mass_points[triangle.p2_idx].position;
            glm::vec3 p3 = mass_points[triangle.p3_idx].position;
            auto intersection = collision(start_point, end_point, p1, p2, p3);
            if (intersection != std::nullopt) {
                collision_constraints.emplace_back(p_idx);
                model.debug_indices().push_back(p_idx);
            }
        }
    }

    for (int i=0; i<100; i++) {
        for (auto const& constraint : constraints) {
            constraint.update(predict);
        }
        for (auto const& constraint : collision_constraints) {
            constraint.update(predict, mass_points);
        }
    }

    for (int i=0; i < mass_points.size(); i++) {
        mass_points[i].velocity = (predict[i].position - mass_points[i].position)/dt;
        mass_points[i].position = predict[i].position;
    }
    collision_constraints.clear();
}
