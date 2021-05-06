#include <vector>
#include <cstring>
#include <iostream>
#include <string>
#include <array>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
    int advance_y;

    int left;
    int top;
    int width;
    int height;

    int texture_x;
    int texture_y;
} char_infos[128];

int atlas_width = 0;
int atlas_height = 0;

static GLuint g_text_texture_id;
static GLuint g_vertex_array_id;
static GLuint g_text_vertex_buffer_id;
static GLuint g_text_uv_buffer_id;
static std::unique_ptr<shader_t> g_text_shader_id;
static GLuint g_text_uniform_id;

static FT_Library g_library;
static FT_Face g_face;

void initText2D(){
    auto error = FT_Init_FreeType(&g_library);
    if (error) {
        throw std::runtime_error{"failed to initialize freetype2"};
    }
    auto font_path = get_default_font_path();
    error = FT_New_Face(g_library, font_path.c_str(), 0, &g_face);
    if (error == FT_Err_Unknown_File_Format) {
        throw std::runtime_error{"freetype2 does not support default font format: " + font_path};
    } else if (error) {
        throw std::runtime_error{"failed to load font file"};
    }
    std::cout << "load font: " << font_path << std::endl;
    if (g_face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
        std::cout << "  loaded font is bitmap font!" << std::endl;
    }
    FT_Set_Pixel_Sizes(g_face, 0, 48);

    // calc texture width and height
    int w = 0;
    int h = 0;
    for (int i=32; i<128; i++) {
        if (FT_Load_Char(g_face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", i);
            continue;
        }
        w += g_face->glyph->bitmap.width;
        h = std::max(h, (int)g_face->glyph->bitmap.rows);
    }
    atlas_width = w;
    atlas_height = h;

    // create empty texture with width and height
    glGenTextures(1, &g_text_texture_id);
    glBindTexture(GL_TEXTURE_2D, g_text_texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    int x = 0;
    for(int i = 32; i < 128; i++) {
        if(FT_Load_Char(g_face, i, FT_LOAD_RENDER))
            continue;
        auto glyph = g_face->glyph;
        int glyph_w = glyph->bitmap.width;
        int glyph_h = glyph->bitmap.rows;
        std::vector<GLubyte> buf(glyph_w * glyph_h * 4);
        for (int i=0; i<glyph_h * glyph_w; i++) {
            buf[i*4 + 0] = glyph->bitmap.buffer[i];
            buf[i*4 + 1] = glyph->bitmap.buffer[i];
            buf[i*4 + 2] = glyph->bitmap.buffer[i];
            buf[i*4 + 3] = glyph->bitmap.buffer[i];
        }
        glTextureSubImage2D(g_text_texture_id, 0, x, 0, glyph->bitmap.width, glyph->bitmap.rows, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

        char_infos[i].advance_x = glyph->advance.x >> 6;
        char_infos[i].advance_y = glyph->advance.y >> 6;
        char_infos[i].left = glyph->bitmap_left;
        char_infos[i].top = glyph->bitmap_top;
        char_infos[i].width = glyph->bitmap.width;
        char_infos[i].height = glyph->bitmap.rows;
        char_infos[i].texture_x = x;
        char_infos[i].texture_y = 0;

        x += glyph->bitmap.width;
    }

	GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_text_texture_id, 0);
    int data_size = atlas_width * atlas_height * 4;
    GLubyte* pixels = new GLubyte[atlas_width * atlas_height * 4];
    glReadPixels(0, 0, atlas_width, atlas_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    stbi_write_bmp("fontdata.bmp", atlas_width, atlas_height, 4, pixels);
    delete[] pixels;

   glGenVertexArrays(1, &g_vertex_array_id);
   glBindVertexArray(g_vertex_array_id);
   glBindVertexArray(g_vertex_array_id);

	// Initialize VBO
	glGenBuffers(1, &g_text_vertex_buffer_id);
	glGenBuffers(1, &g_text_uv_buffer_id);

	// Initialize Shader
	g_text_shader_id = std::make_unique<shader_t>(shader_t::load_from_files( "resource/TextVertexShader.vertexshader", "resource/TextVertexShader.fragmentshader" ));

	// Initialize uniforms' IDs
	g_text_uniform_id = glGetUniformLocation( g_text_shader_id->id(), "myTextureSampler" );

}

void printText2D(const char * text, int x, int y, int size){

    unsigned int length = strlen(text);

    // Fill buffers
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;
    for ( unsigned int i=0 ; i<length ; i++ ){
        int c = text[i];
        glm::vec2 vertex_up_left = glm::vec2(x, y);
        glm::vec2 vertex_up_right = glm::vec2(x+size, y);
        glm::vec2 vertex_down_right = glm::vec2(x+size, y+size);
        glm::vec2 vertex_down_left = glm::vec2(x, y+size);

        vertices.push_back(vertex_up_left);
        vertices.push_back(vertex_down_left);
        vertices.push_back(vertex_up_right);

        vertices.push_back(vertex_down_right);
        vertices.push_back(vertex_up_right);
        vertices.push_back(vertex_down_left);

        float uv_x = (float)char_infos[c].texture_x / atlas_width;
        float uv_y = (float)(char_infos[c].texture_y + char_infos[c].height) / atlas_height;
        float uv_width = (float)char_infos[c].width / atlas_width;
        float uv_height = (float)(char_infos[c].texture_y + char_infos[c].height) / atlas_height;

        x += size;

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
    glBindBuffer(GL_ARRAY_BUFFER, g_text_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, g_text_uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);

    // Bind shader
    glUseProgram(g_text_shader_id->id());

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_text_texture_id);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(g_text_uniform_id, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_text_vertex_buffer_id);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, g_text_uv_buffer_id);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Draw call
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/2);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void cleanupText2D(){

	// Delete buffers
	glDeleteBuffers(1, &g_text_vertex_buffer_id);
	glDeleteBuffers(1, &g_text_uv_buffer_id);

	// Delete texture
	glDeleteTextures(1, &g_text_texture_id);

	// Delete shader
	glDeleteProgram(g_text_shader_id->id());
}
