#include <iostream>
#include <memory>

#include "model.hpp"
#include "window.hpp"
#include "GLFW/glfw3.h"

int main() {
    auto window = std::make_unique<window_t>();
    rigid_body_t rigid_body;
    auto cloth = model_t::make_cloth(rigid_body);

    while (true) {
        rigid_body.update(0.1f);

        window->update();
        window->draw(cloth);

        if (window->shouldClose())
            break;
    }
    return 0;
}