#ifndef PHYICUIHENG_MODEL_HPP
#define PHYICUIHENG_MODEL_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

struct model_t {
    static model_t make_cloth();
    virtual ~model_t();

    void draw() const;
private:
    explicit model_t() = delete;
    explicit model_t(std::vector<glm::vec3>&& vertices, std::vector<unsigned short>&& indices);

    GLuint m_vertex_array_id;
    GLuint m_vertex_buffer_id;
    GLuint m_index_buffer_id;
    std::vector<glm::vec3> m_vertices;
    std::vector<unsigned short> m_indices;
};

#endif
