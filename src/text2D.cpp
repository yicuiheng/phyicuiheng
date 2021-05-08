#include <vector>
#include <cstring>
#include <iostream>
#include <string>
#include <array>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.hpp"

#include "text2D.hpp"

// TODO: not to use pipe, use fontconfig library
std::string get_default_font_path() {
    std::array<char, 128> buffer;
    std::string stdout;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("fc-match -v | grep file:", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        stdout += buffer.data();
    }
    std::string result;
    bool is_in_filepath = false;
    for (auto c : stdout) {
        if (is_in_filepath && c != '\"') {
            result += c;
        }
        if (c == '\"')
            is_in_filepath = !is_in_filepath;
    }
    return result;
}

struct char_info_t {
    int advance_x;
    int width;
    int height;
    int bearing_x;
    int bearing_y;

    int texture_x;
    int texture_y;
};

static std::unique_ptr<shader_t> g_text_shader_id;
GLuint g_texture_sampler_id;
static std::unique_ptr<FT_Library> g_library;

struct FontData {
    ~FontData() {
        glDeleteTextures(1, &texture_id);
        glDeleteProgram(g_text_shader_id->id());
    }
    GLuint texture_id;
    FT_Face face;
    int atlas_width, atlas_height;
    char_info_t char_infos[128];
};

static std::unique_ptr<FontData> g_default_font;

void init_freetype2_library() {
    assert(g_library == nullptr);

    g_library = std::make_unique<FT_Library>();
    auto error = FT_Init_FreeType(g_library.get());
    if (error) {
        throw std::runtime_error{"failed to initialize freetype2"};        
    }
}

void init_default_font(int size) {
    assert(g_text_shader_id == nullptr);
    assert(g_default_font == nullptr);

    // initialize shader
	g_text_shader_id = std::make_unique<shader_t>(shader_t::load_from_files( "resource/TextVertexShader.vertexshader", "resource/TextVertexShader.fragmentshader" ));
    g_texture_sampler_id = glGetUniformLocation( g_text_shader_id->id(), "myTextureSampler" );

    // initialize font data
    g_default_font = std::make_unique<FontData>();

    auto font_path = get_default_font_path();
    auto error = FT_New_Face(*g_library, font_path.c_str(), 0, &g_default_font->face);
    if (error == FT_Err_Unknown_File_Format) {
        throw std::runtime_error{"freetype2 does not support default font format: " + font_path};
    } else if (error) {
        throw std::runtime_error{"failed to load font file"};
    }
    FT_Set_Pixel_Sizes(g_default_font->face, 0, size);

    // calc text texture size
    int w = 0;
    int h = 0;
    for (int i=32; i<128; i++) {
        if (FT_Load_Char(g_default_font->face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", i);
            continue;            
        }
        auto const& bitmap = g_default_font->face->glyph->bitmap;
        w += bitmap.width;
        h = std::max(h, (int)bitmap.rows);
    }
    g_default_font->atlas_width = w;
    g_default_font->atlas_height = h;

    // create empty texture with width and height
    glGenTextures(1, &g_default_font->texture_id);
    glBindTexture(GL_TEXTURE_2D, g_default_font->texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // write font glyph bitmap to text texture
    int x = 0;
    for(int i = 32; i < 128; i++) {
        if(FT_Load_Char(g_default_font->face, i, FT_LOAD_RENDER))
            continue;
        auto glyph = g_default_font->face->glyph;
        int glyph_w = glyph->bitmap.width;
        int glyph_h = glyph->bitmap.rows;
        std::vector<GLubyte> buf(glyph_w * glyph_h * 4);
        for (int i=0; i<glyph_h * glyph_w; i++) {
            buf[i*4 + 0] = glyph->bitmap.buffer[i];
            buf[i*4 + 1] = glyph->bitmap.buffer[i];
            buf[i*4 + 2] = glyph->bitmap.buffer[i];
            buf[i*4 + 3] = glyph->bitmap.buffer[i];
        }
        glTextureSubImage2D(g_default_font->texture_id, 0, x, 0, glyph->bitmap.width, glyph->bitmap.rows, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

        g_default_font->char_infos[i].advance_x = glyph->advance.x >> 6;
        g_default_font->char_infos[i].width = glyph->bitmap.width;
        g_default_font->char_infos[i].height = glyph->bitmap.rows;
        g_default_font->char_infos[i].bearing_x = glyph->metrics.horiBearingX >> 6;
        g_default_font->char_infos[i].bearing_y = glyph->metrics.horiBearingY >> 6;
        g_default_font->char_infos[i].texture_x = x;
        g_default_font->char_infos[i].texture_y = 0;

        x += glyph->bitmap.width;
    }
}

TextTexture::TextTexture(const char* text, int x, int y, int size) :
    m_text{text}, m_x{x}, m_y{y}, m_size{size}
{
    if (g_library == nullptr)
        init_freetype2_library();
    if (g_default_font == nullptr)
        init_default_font(size);
    
    glGenVertexArrays(1, &m_vertex_array_id);
    glBindVertexArray(m_vertex_array_id);

    // generate buffers for VBO
    glGenBuffers(1, &m_vertex_buffer_id);
    glGenBuffers(1, &m_uv_buffer_id);

    // Fill buffers
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;
    for (int i=0; text[i] != '\0'; i++) {
        int c = text[i];
        auto const& char_info = g_default_font->char_infos[c];
        int left_x = x + char_info.bearing_x;
        int right_x = left_x + char_info.width;
        int up_y = y + char_info.bearing_y;
        int down_y = up_y + char_info.height;
        glm::vec2 vertex_up_left = glm::vec2(left_x, up_y);
        glm::vec2 vertex_up_right = glm::vec2(right_x, up_y);
        glm::vec2 vertex_down_right = glm::vec2(right_x, down_y);
        glm::vec2 vertex_down_left = glm::vec2(left_x, down_y);
        /*
        glm::vec2 vertex_up_left = glm::vec2(x, y);
        glm::vec2 vertex_up_right = glm::vec2(x+char_info.width, y);
        glm::vec2 vertex_down_right = glm::vec2(x+char_info.width, y+char_info.height);
        glm::vec2 vertex_down_left = glm::vec2(x, y+char_info.height);
        */

        vertices.push_back(vertex_up_left);
        vertices.push_back(vertex_down_left);
        vertices.push_back(vertex_up_right);

        vertices.push_back(vertex_down_right);
        vertices.push_back(vertex_up_right);
        vertices.push_back(vertex_down_left);

        float uv_x = (float)char_info.texture_x / g_default_font->atlas_width;
        float uv_width = (float)char_info.width / g_default_font->atlas_width;
        float uv_height = (float)(char_info.texture_y + char_info.height) / g_default_font->atlas_height;

        x += char_info.advance_x;

        glm::vec2 uv_up_left = glm::vec2(uv_x, uv_height);
        glm::vec2 uv_up_right = glm::vec2(uv_x+uv_width, uv_height);
        glm::vec2 uv_down_right = glm::vec2(uv_x+uv_width, 0.f);
        glm::vec2 uv_down_left = glm::vec2(uv_x, 0.f);

        UVs.push_back(uv_up_left);
        UVs.push_back(uv_down_left);
        UVs.push_back(uv_up_right);

        UVs.push_back(uv_down_right);
        UVs.push_back(uv_up_right);
        UVs.push_back(uv_down_left);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), UVs.data(), GL_STATIC_DRAW);

}

void TextTexture::draw() const {
    glUseProgram(g_text_shader_id->id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_default_font->texture_id);
    glUniform1i(g_texture_sampler_id, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer_id);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, m_text.size()*6);
    glDisable(GL_BLEND);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

TextTexture::~TextTexture() {
    glDeleteBuffers(1, &m_vertex_buffer_id);
    glDeleteBuffers(1, &m_uv_buffer_id);
}

