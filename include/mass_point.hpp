#ifndef PHYICUIHENG_MASS_POINT_HPP
#define PHYICUIHENG_MASS_POINT_HPP

#include <vector>
#include <glm/glm.hpp>

struct mass_point_t {
    explicit mass_point_t(glm::vec3& p) : position{p} {}

    float weight = 1.0f;
    glm::vec3& position;
    glm::vec3 velocity = glm::vec3{0.0f, 0.0f, 0.0f};
};


#endif
