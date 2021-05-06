#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include "model.hpp"
#include "window.hpp"

int main() {
    auto window = std::make_unique<window_t>();
    rigid_body_t rigid_body;
    auto cloth = model_t::make_cloth(rigid_body);

    while (true) {
            // rigid_body.mass_points[(LENGTH+1) * LENGTH + LENGTH/2].weight = 1.0f;
        if (window->isSpacePressed())
            rigid_body.update(0.2f, cloth);

        window->update();

        window->draw(cloth);

        if (window->shouldClose())
            break;
    }
    return 0;
}