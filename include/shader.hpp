#ifndef PHYICUIHENG_SHADER_HPP
#define PHYICUIHENG_SHADER_HPP

#include <GL/glew.h>

struct shader_t {
    static shader_t load_from_files(const char* vertex_filename, const char* fragment_filename);

    GLuint id() const { return m_program_id; }    
private:
    explicit shader_t() = delete;
    explicit shader_t(GLuint program_id) :
        m_program_id{program_id}
    {}
    GLuint m_program_id;
};

#endif
