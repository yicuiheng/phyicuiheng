#include <iostream>
#include <memory>

#include "model.hpp"
#include "window.hpp"

int main() {
    auto window = std::make_unique<window_t>();
    auto cloth = model_t::make_cloth();

    while (true) {
        // update models

        window->update();
        window->draw(cloth);

        if (window->shouldClose())
            break;
    }
    return 0;
}