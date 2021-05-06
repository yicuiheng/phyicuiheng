#include <numeric>
#include "model.hpp"

model_t::model_t(
    std::vector<glm::vec3>&& vertices,
    std::vector<unsigned short>&& indices,
    std::vector<glm::vec2>&& coords) :
    m_vertices(std::move(vertices)),
    m_indices(std::move(indices)),
    m_coords(std::move(coords))
{
    glGenVertexArrays(1, &m_vertex_array_id);
    glBindVertexArray(m_vertex_array_id);

    glGenBuffers(1, &m_vertex_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof (glm::vec3), m_vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_normal_buffer_id);

    glGenBuffers(1, &m_coord_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_coord_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_coords.size() * sizeof (glm::vec2), m_coords.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_index_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof (unsigned short), m_indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_highlighted_vertex_index_buffer_id);
}

model_t::~model_t() {
    glDeleteVertexArrays(1, &m_vertex_array_id);
    glDeleteBuffers(1, &m_vertex_buffer_id);
    glDeleteBuffers(1, &m_normal_buffer_id);
    glDeleteBuffers(1, &m_coord_buffer_id);
    glDeleteBuffers(1, &m_index_buffer_id);
    glDeleteBuffers(1, &m_highlighted_vertex_index_buffer_id);
}

model_t model_t::make_cloth(rigid_body_t& rigid_body) {
    std::vector<glm::vec3> vertices((LENGTH+1) * (LENGTH+1));
    rigid_body.mass_points.reserve(vertices.size());

    std::vector<glm::vec2> coords(vertices.size());

    for (int j=0; j<=LENGTH; j++) {
        for (int i=0; i<=LENGTH; i++) {
            float x = 2.0f * (double)i / (double)LENGTH - 1.0f;
            float y = 2.0f * (double)j / (double)LENGTH - 1.0f;
            vertices[j * (LENGTH+1) + i] = glm::vec3(x, y, 0.0f);
            coords[j * (LENGTH+1) + i] = glm::vec2(x, y);

            rigid_body.mass_points.emplace_back(vertices[j * (LENGTH+1) + i]);
        }
    }
    rigid_body.mass_points[(LENGTH+1) * LENGTH + LENGTH/2].weight = 0.0f;
    // rigid_body.mass_points[(LENGTH+1) * (LENGTH+1) - 1].weight = 0.0f;

    std::vector<unsigned short> indices;
    indices.reserve(2 * (LENGTH+1) * (LENGTH+1));
    for (int j=0; j<=LENGTH; j++) {
        for (int i=0; i<=LENGTH; i++) {
            int left_top = (LENGTH + 1) * j + i;
            int right_top = (LENGTH + 1) * j + i + 1;
            int left_bottom = (LENGTH + 1) * (j + 1) + i;
            int right_bottom = (LENGTH + 1) * (j + 1) + i + 1;

            if (i != LENGTH && j != LENGTH) {
                indices.push_back(left_top);
                indices.push_back(left_bottom);
                indices.push_back(right_top);
                rigid_body.triangles.emplace_back(left_top, left_bottom, right_top);

                indices.push_back(right_bottom);
                indices.push_back(right_top);
                indices.push_back(left_bottom);
                rigid_body.triangles.emplace_back(right_bottom, right_top, left_bottom);
            }
            if (i != LENGTH) {
                // (i, j) - (i+1, j)
                rigid_body.constraints.emplace_back(rigid_body.mass_points, left_top, right_top);
            }
            if (j != LENGTH) {
                // (i, j) - (i, j+1)
                rigid_body.constraints.emplace_back(rigid_body.mass_points, left_top, left_bottom);
            }
            if (i != LENGTH && j != LENGTH) {
                // (i, j) - (i+1, j+1)
                rigid_body.constraints.emplace_back(rigid_body.mass_points, left_top, right_bottom);
            }
        }
    }
    indices.shrink_to_fit();
    model_t model{std::move(vertices), std::move(indices), std::move(coords)};
    return model;
}

std::vector<glm::vec3> calc_normals(std::vector<glm::vec3> const& vertices) {
    std::vector<glm::vec3> normals(vertices.size());
    for (int j=0; j<=LENGTH; j++) {
        for (int i=0; i<=LENGTH; i++) {
            int center = (LENGTH + 1) * j + i;
            int left = (LENGTH + 1) * j + i - 1;
            int right = (LENGTH + 1) * j + i + 1;
            int up = (LENGTH + 1) * (j - 1) + i;
            int down = (LENGTH + 1) * (j + 1) + i;

            std::vector<glm::vec3> local_normals;
            if (i != 0 && j != 0) {
                local_normals.push_back(glm::cross(
                    vertices[up] - vertices[center],
                    vertices[left] - vertices[center]));
            }
            if (i != 0 && j != LENGTH) {
                local_normals.push_back(glm::cross(
                    vertices[left] - vertices[center],
                    vertices[down] - vertices[center]));
            }
            if (i != LENGTH && j != LENGTH) {
                local_normals.push_back(glm::cross(
                    vertices[down] - vertices[center],
                    vertices[right] - vertices[center]));
            }
            if (i != LENGTH && j != 0) {
                local_normals.push_back(glm::cross(
                    vertices[right] - vertices[center],
                    vertices[up] - vertices[center]));
            }
            normals[center] = std::accumulate(local_normals.begin(), local_normals.end(), glm::vec3{});
            normals[center] = glm::normalize(normals[center]);
        }
    }
    return normals;
}

void model_t::draw() {
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), m_vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    std::vector<glm::vec3> normals = calc_normals(m_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof (glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer_id);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_coord_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_coords.size() * sizeof(glm::vec2), m_coords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_coord_buffer_id);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_SHORT, nullptr);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void model_t::debug_draw() {
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), m_vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_highlighted_vertex_index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_highlighted_vertex_indices.size() * sizeof (unsigned short), m_highlighted_vertex_indices.data(), GL_STATIC_DRAW);
    glDrawElements(GL_POINTS, m_highlighted_vertex_indices.size(), GL_UNSIGNED_SHORT, nullptr);

    glDisableVertexAttribArray(0);

    // m_highlighted_vertex_indices.clear();
}
