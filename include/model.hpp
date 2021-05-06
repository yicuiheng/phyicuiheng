#ifndef PHYICUIHENG_MODEL_HPP
#define PHYICUIHENG_MODEL_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <rigid_body.hpp>

static constexpr int LENGTH = 5;

struct model_t {
    // 布のモデルを生成して、それの剛体モデルを rigid_body にアレする
    static model_t make_cloth(rigid_body_t& rigid_body);
    virtual ~model_t();

    void draw();
    void debug_draw();
    std::vector<unsigned short>& debug_indices() { return m_highlighted_vertex_indices; }
private:
    explicit model_t() = delete;
    explicit model_t(
        std::vector<glm::vec3>&& vertices,
        std::vector<unsigned short>&& indices,
        std::vector<glm::vec2>&& coords);

    GLuint m_vertex_array_id;

    GLuint m_vertex_buffer_id;
    std::vector<glm::vec3> m_vertices;

    GLuint m_normal_buffer_id;

    GLuint m_coord_buffer_id;
    std::vector<glm::vec2> m_coords;

    GLuint m_index_buffer_id;
    std::vector<unsigned short> m_indices;

    GLuint m_highlighted_vertex_index_buffer_id;
    std::vector<unsigned short> m_highlighted_vertex_indices; // for debug
};

#endif
