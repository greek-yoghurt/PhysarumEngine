#pragma once

// renderer.h — Physarum Engine (Week 3: pheromone trail support)
// Single-header OpenGL renderer: fullscreen quad + grayscale pixel texture
// Requires: OpenGL 3.3 Core, GLAD, GLFW 3.4
// Usage: include this file in exactly ONE translation unit to get the implementation.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstring>    // memset
#include <cstdio>     // fprintf
#include <cstdlib>    // exit
#include <algorithm>  // std::clamp

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static constexpr int RENDERER_WIDTH  = 800;
static constexpr int RENDERER_HEIGHT = 600;
static constexpr int RENDERER_PIXELS = RENDERER_WIDTH * RENDERER_HEIGHT;

// ---------------------------------------------------------------------------
// Internal state  (all static — lives in the TU that includes this header)
// ---------------------------------------------------------------------------

static float  g_pixels  [RENDERER_PIXELS];  // CPU pixel buffer   — cleared every frame
static float  g_trailMap[RENDERER_PIXELS];  // pheromone buffer   — persists between frames
static float  g_trailTmp[RENDERER_PIXELS];  // scratch buffer used during diffusion

static GLuint g_texture = 0;
static GLuint g_shader  = 0;
static GLuint g_vao     = 0;
static GLuint g_vbo     = 0;

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
// Public API — existing functions (unchanged behaviour)
// ---------------------------------------------------------------------------

/// Sets up the texture, shaders, fullscreen quad VAO/VBO, and zeroes both
/// the pixel buffer and the trail map.  Call once after creating the OpenGL
/// context.
inline void initRenderer()
{
    // --- buffers ---
    memset(g_pixels,   0, sizeof(g_pixels));
    memset(g_trailMap, 0, sizeof(g_trailMap));
    memset(g_trailTmp, 0, sizeof(g_trailTmp));

    // --- texture ---
    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R32F,
                 RENDERER_WIDTH,
                 RENDERER_HEIGHT,
                 0,
                 GL_RED,
                 GL_FLOAT,
                 g_pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    // --- shaders ---
    GLuint vert = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    g_shader    = linkProgram(vert, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(g_shader);
    glUniform1i(glGetUniformLocation(g_shader, "uTexture"), 0);
    glUseProgram(0);

    // --- fullscreen quad ---
    // Two triangles covering NDC [-1,1]x[-1,1].
    // UV Y-flipped so row 0 of the buffer = top of screen.
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
    glGenBuffers(1,      &g_vbo);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), QUAD, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/// Clears the pixel display buffer to 0 (black).
/// NOTE: does NOT touch the trail map — trails persist between frames.
inline void clearPixels()
{
    memset(g_pixels, 0, sizeof(g_pixels));
}

/// Sets the pixel at (x, y) to 1.0 (white) in the display buffer.
/// Origin is top-left; out-of-bounds writes are silently ignored.
inline void setPixel(int x, int y)
{
    if (x < 0 || x >= RENDERER_WIDTH ||
        y < 0 || y >= RENDERER_HEIGHT)
        return;

    g_pixels[y * RENDERER_WIDTH + x] = 1.0f;
}

/// Uploads the pixel buffer to the GPU and draws the fullscreen quad.
/// Call once per frame after all setPixel() / trailToPixels() calls.
inline void renderPixels()
{
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0, 0,
                    RENDERER_WIDTH,
                    RENDERER_HEIGHT,
                    GL_RED,
                    GL_FLOAT,
                    g_pixels);

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

/// Releases all OpenGL resources.  Call before destroying the OpenGL context.
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

// ---------------------------------------------------------------------------
// Public API — Week 3: pheromone trail functions
// ---------------------------------------------------------------------------

/// Adds `amount` of pheromone to the trail map at (x, y).
/// Value is clamped to [0, 1] so trails never blow up.
/// Out-of-bounds writes are silently ignored.
inline void depositTrail(int x, int y, float amount)
{
    if (x < 0 || x >= RENDERER_WIDTH ||
        y < 0 || y >= RENDERER_HEIGHT)
        return;

    float& cell = g_trailMap[y * RENDERER_WIDTH + x];
    cell = std::clamp(cell + amount, 0.0f, 1.0f);
}

/// Returns the pheromone value at (x, y) in the trail map.
/// Returns 0.0 for any out-of-bounds position.
inline float senseTrail(int x, int y)
{
    if (x < 0 || x >= RENDERER_WIDTH ||
        y < 0 || y >= RENDERER_HEIGHT)
        return 0.0f;

    return g_trailMap[y * RENDERER_WIDTH + x];
}

/// Decays and diffuses the trail map.  Call once per frame.
///
/// Two-pass operation:
///   1. Diffuse  — each cell becomes a weighted blend of itself (60%) and the
///                 average of its 8 neighbours (40%).  This spreads trails
///                 outward by one pixel per frame, creating soft halos.
///   2. Decay    — multiply every cell by (1 - decayRate).
///                 decayRate = 0.01 → very slow fade (long-lived trails)
///                 decayRate = 0.10 → fast fade (short-lived trails)
///
/// Border cells use only the neighbours that exist (no wrap-around).
inline void decayAndDiffuseTrails(float decayRate)
{
    // --- pass 1: diffuse into g_trailTmp ---
    for (int y = 0; y < RENDERER_HEIGHT; ++y)
    {
        for (int x = 0; x < RENDERER_WIDTH; ++x)
        {
            float sum    = 0.0f;
            int   count  = 0;

            // accumulate the 8 neighbours
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    if (dx == 0 && dy == 0) continue; // skip self

                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx < 0 || nx >= RENDERER_WIDTH  ||
                        ny < 0 || ny >= RENDERER_HEIGHT)
                        continue;

                    sum += g_trailMap[ny * RENDERER_WIDTH + nx];
                    ++count;
                }
            }

            float self      = g_trailMap[y * RENDERER_WIDTH + x];
            float neighbourAvg = (count > 0) ? (sum / count) : self;

            // 60% self, 40% neighbour average
            g_trailTmp[y * RENDERER_WIDTH + x] = self * 0.6f + neighbourAvg * 0.4f;
        }
    }

    // --- pass 2: decay and write back into g_trailMap ---
    float keepRate = 1.0f - decayRate;
    for (int i = 0; i < RENDERER_PIXELS; ++i)
        g_trailMap[i] = g_trailTmp[i] * keepRate;
}

/// Copies the trail map into the display pixel buffer so it gets rendered.
/// Call this instead of (or after) clearPixels() + setPixel() when you want
/// trails to be visible.  Agent pixels can still be drawn on top with
/// setPixel() after calling trailToPixels().
inline void trailToPixels()
{
    memcpy(g_pixels, g_trailMap, sizeof(g_pixels));
}