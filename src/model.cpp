#include "model.hpp"

model_t::model_t(std::vector<glm::vec3>&& vertices, std::vector<unsigned short>&& indices) :
    m_vertices(std::move(vertices)),
    m_indices(std::move(indices))
{
    glGenVertexArrays(1, &m_vertex_array_id);
    glBindVertexArray(m_vertex_array_id);

    glGenBuffers(1, &m_vertex_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof (glm::vec3), m_vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_index_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof (unsigned short), m_indices.data(), GL_STATIC_DRAW);
}

model_t::~model_t() {
    glDeleteVertexArrays(1, &m_vertex_array_id);
    glDeleteBuffers(1, &m_vertex_buffer_id);
    glDeleteBuffers(1, &m_index_buffer_id);
}

model_t model_t::make_cloth() {
    constexpr int LENGTH = 50;
    glm::vec3 vertices[LENGTH+1][LENGTH+1] = {{}};
    for (int j=0; j<=LENGTH; j++) {
        for (int i=0; i<=LENGTH; i++) {
            float x = 2.0f * (double)i / (double)LENGTH - 1.0f;
            float y = 2.0f * (double)j / (double)LENGTH - 1.0f;
            vertices[j][i] = glm::vec3(x, y, 0.0f);
        }
    }
    std::vector<glm::vec3> result_vertices((glm::vec3*)vertices, (glm::vec3*)vertices + (LENGTH+1) * (LENGTH+1));

    std::vector<unsigned short> indices;
    indices.reserve(2 * (LENGTH+1) * (LENGTH+1));
    for (int j=0; j<=LENGTH; j++) {
        for (int i=0; i<LENGTH; i++) {
            indices.push_back((LENGTH+1) * j + i);
            indices.push_back((LENGTH+1) * j + i + 1);

            indices.push_back((LENGTH+1) * i + j);
            indices.push_back((LENGTH+1) * i + j + LENGTH + 1);
        }
    }
    indices.shrink_to_fit();
    return model_t{std::move(result_vertices), std::move(indices)};
}

void model_t::draw() const {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);

    glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_SHORT, nullptr);

    glDisableVertexAttribArray(0);
}