#ifndef PHYICUIHENG_WINDOW_HPP
#define PHYICUIHENG_WINDOW_HPP

#include <memory>
#include "camera.hpp"
#include "shader.hpp"

struct GLFWwindow;
struct model_t;

struct window_t {
    explicit window_t();
    virtual ~window_t();

    void update();
    void draw(model_t const& model) const;

    bool shouldClose() const;
private:
   GLFWwindow* m_window = nullptr;
   GLuint m_matrix_id;
   std::unique_ptr<shader_t> m_shader = nullptr;
   camera_t m_camera;
};

#endif
