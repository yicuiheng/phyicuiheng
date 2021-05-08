#include <iostream>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


#include "window.hpp"
#include "shader.hpp"
#include "model.hpp"
#include "text2D.hpp"

window_t::window_t() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        std::exit(-1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1024, 768, "Phyicuiheng", nullptr, nullptr);
    if (m_window == nullptr) {
        std::cerr << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        std::exit(-1);
    }
    glfwMakeContextCurrent(m_window);

	glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        std::exit(-1);
    }

    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);
    // glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(m_window, 1024/2, 768/2);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glPointSize(10.0f);

    m_shader = std::make_unique<shader_t>(shader_t::load_from_files("resource/VertexShader.glsl", "resource/FragmentShader.glsl"));
    m_debug_shader = std::make_unique<shader_t>(shader_t::load_from_files("resource/VertexShader.glsl", "resource/DebugFragmentShader.glsl"));
    m_mvp_matrix_id = glGetUniformLocation(m_shader->id(), "MVP");
    m_view_matrix_id = glGetUniformLocation(m_shader->id(), "V");
    m_model_matrix_id = glGetUniformLocation(m_shader->id(), "M");
    m_light_id = glGetUniformLocation(m_shader->id(), "LightPosition_worldspace");

    m_hoge_text = std::make_unique<TextTexture>("hoge fuga", 120, 120, 24);
}

window_t::~window_t() {
    glDeleteProgram(m_shader->id());
    glfwTerminate();
}

void window_t::update() {
    m_camera.update(m_window);
}

void window_t::draw(model_t& model) const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shader->id());

    glm::mat4 projection_matrix = m_camera.projection();
    glm::mat4 view_matrix = m_camera.view();
    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 MVP = projection_matrix * view_matrix * model_matrix;

    glUniformMatrix4fv(m_mvp_matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(m_model_matrix_id, 1, GL_FALSE, &model_matrix[0][0]);
    glUniformMatrix4fv(m_view_matrix_id, 1, GL_FALSE, &view_matrix[0][0]);

    glm::vec3 lightPos = glm::vec3(4.0f,4.0f,-1.0f);
    glUniform3f(m_light_id, lightPos.x, lightPos.y, lightPos.z);

    model.draw();

    glUseProgram(m_debug_shader->id());
    glUniformMatrix4fv(m_mvp_matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(m_model_matrix_id, 1, GL_FALSE, &model_matrix[0][0]);
    glUniformMatrix4fv(m_view_matrix_id, 1, GL_FALSE, &view_matrix[0][0]);
    glUniform3f(m_light_id, lightPos.x, lightPos.y, lightPos.z);
    model.debug_draw();

    m_hoge_text->draw();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool window_t::shouldClose() const {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        return true;
    if (glfwWindowShouldClose(m_window) != 0)
        return true;
    return false;
}

bool window_t::isSpacePressed() const {
    return glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS;
}
