#pragma once

// renderer.h — Physarum Engine
// Single-header OpenGL renderer: fullscreen quad + grayscale pixel texture
// Requires: OpenGL 3.3 Core, GLAD, GLFW 3.4
// Usage: include this file in exactly ONE translation unit to get the implementation.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstring>   // memset
#include <cstdio>    // fprintf
#include <cstdlib>   // exit

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static constexpr int RENDERER_WIDTH  = 800;
static constexpr int RENDERER_HEIGHT = 600;
static constexpr int RENDERER_PIXELS = RENDERER_WIDTH * RENDERER_HEIGHT;

// ---------------------------------------------------------------------------
// Internal state  (all static — lives in the TU that includes this header)
// ---------------------------------------------------------------------------

static float       g_pixels[RENDERER_PIXELS]; // CPU pixel buffer
static GLuint      g_texture  = 0;
static GLuint      g_shader   = 0;
static GLuint      g_vao      = 0;
static GLuint      g_vbo      = 0;

// ---------------------------------------------------------------------------
// Shader sources
// ---------------------------------------------------------------------------

static const char* VERT_SRC = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord   = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

static const char* FRAG_SRC = R"GLSL(
#version 330 core
in  vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    float brightness = texture(uTexture, vTexCoord).r;
    FragColor = vec4(brightness, brightness, brightness, 1.0);
}
)GLSL";

// ---------------------------------------------------------------------------
// Helper: compile + link shaders
// ---------------------------------------------------------------------------

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fprintf(stderr, "[Renderer] Shader compile error:\n%s\n", log);
        exit(EXIT_FAILURE);
    }
    return shader;
}

static GLuint linkProgram(GLuint vert, GLuint frag)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        fprintf(stderr, "[Renderer] Shader link error:\n%s\n", log);
        exit(EXIT_FAILURE);
    }
    return prog;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Sets up the texture, shaders, and fullscreen quad VAO/VBO.
/// Call once after creating the OpenGL context.
inline void initRenderer()
{
    // --- pixel buffer ---
    memset(g_pixels, 0, sizeof(g_pixels));

    // --- texture ---
    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Allocate GPU storage (1-channel float texture)
    glTexImage2D(GL_TEXTURE_2D,
                 0,                  // mip level
                 GL_R32F,            // internal format: single-channel 32-bit float
                 RENDERER_WIDTH,
                 RENDERER_HEIGHT,
                 0,
                 GL_RED,             // pixel data format
                 GL_FLOAT,
                 g_pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    // --- shaders ---
    GLuint vert = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    g_shader    = linkProgram(vert, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    // Bind the texture sampler uniform once (it never changes)
    glUseProgram(g_shader);
    glUniform1i(glGetUniformLocation(g_shader, "uTexture"), 0); // texture unit 0
    glUseProgram(0);

    // --- fullscreen quad ---
    // Two triangles covering NDC [-1,1] x [-1,1].
    // Texture coords are flipped on Y so (0,0) maps to top-left of the pixel
    // buffer (row 0 = top of screen).
    //
    //  aPos (x,y)     aTexCoord (u,v)
    static const float QUAD[] = {
        // triangle 1
        -1.0f,  1.0f,   0.0f, 0.0f,   // top-left
        -1.0f, -1.0f,   0.0f, 1.0f,   // bottom-left
         1.0f, -1.0f,   1.0f, 1.0f,   // bottom-right
        // triangle 2
        -1.0f,  1.0f,   0.0f, 0.0f,   // top-left
         1.0f, -1.0f,   1.0f, 1.0f,   // bottom-right
         1.0f,  1.0f,   1.0f, 0.0f,   // top-right
    };

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), QUAD, GL_STATIC_DRAW);

    // layout(location = 0): position (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // layout(location = 1): texcoord (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/// Sets all pixels in the CPU buffer to 0 (black).
/// Call at the start of each frame before placing agents.
inline void clearPixels()
{
    memset(g_pixels, 0, sizeof(g_pixels));
}

/// Sets the pixel at (x, y) to 1.0 (white).
/// Origin is top-left; x in [0, WIDTH-1], y in [0, HEIGHT-1].
/// Out-of-bounds writes are silently ignored.
inline void setPixel(int x, int y)
{
    if (x < 0 || x >= RENDERER_WIDTH ||
        y < 0 || y >= RENDERER_HEIGHT)
        return;

    g_pixels[y * RENDERER_WIDTH + x] = 1.0f;
}

/// Uploads the CPU pixel buffer to the GPU texture and draws the fullscreen quad.
/// Call once per frame after all setPixel() calls.
inline void renderPixels()
{
    // Upload updated pixel data to GPU
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,                  // mip level
                    0, 0,               // x, y offset
                    RENDERER_WIDTH,
                    RENDERER_HEIGHT,
                    GL_RED,
                    GL_FLOAT,
                    g_pixels);

    // Draw
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

/// Releases all OpenGL resources allocated by initRenderer().
/// Call before destroying the OpenGL context.
inline void cleanupRenderer()
{
    glDeleteVertexArrays(1, &g_vao);
    glDeleteBuffers(1,      &g_vbo);
    glDeleteTextures(1,     &g_texture);
    glDeleteProgram(g_shader);

    g_vao     = 0;
    g_vbo     = 0;
    g_texture = 0;
    g_shader  = 0;
}