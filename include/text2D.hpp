#ifndef PHYICUIHENG_TEXT2D_HPP
#define PHYICUIHENG_TEXT2D_HPP

#include <GL/glew.h>

struct TextTexture {
    explicit TextTexture(const char* text, int x, int y, int size);
    ~TextTexture();
    void draw() const;
private:
    std::string m_text;
    int m_x, m_y, m_size;
    GLuint m_vertex_array_id;
    GLuint m_vertex_buffer_id;
    GLuint m_uv_buffer_id;
};

#endif