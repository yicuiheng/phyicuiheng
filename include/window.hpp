#ifndef PHYICUIHENG_WINDOW_HPP
#define PHYICUIHENG_WINDOW_HPP

#include <memory>

#include "text2D.hpp"
#include "camera.hpp"
#include "shader.hpp"

struct GLFWwindow;
struct model_t;

struct window_t {
    explicit window_t();
    virtual ~window_t();

    void update();
    void draw(model_t& model) const;

    bool shouldClose() const;
    bool isSpacePressed() const;
private:
    GLFWwindow* m_window = nullptr;
    GLuint m_mvp_matrix_id, m_view_matrix_id, m_model_matrix_id;
    GLuint m_light_id;
    std::unique_ptr<shader_t> m_shader = nullptr;
    std::unique_ptr<shader_t> m_debug_shader = nullptr;
    std::unique_ptr<TextTexture> m_hoge_text = nullptr;
    camera_t m_camera;
};

#endif
