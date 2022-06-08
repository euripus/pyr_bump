﻿#ifndef WINDOW_H
#define WINDOW_H

#include <memory>
#include <string>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene/sceneentitybuilder.h"
#include "input/input.h"
#include "scene/camera.h"

class Window
{
    // window state
    bool                m_is_fullscreen    = false;
    GLFWvidmode const * mp_base_video_mode = nullptr;
    GLFWwindow *        mp_glfw_win        = nullptr;
    glm::ivec2          m_size;
    std::string         m_title;

    std::unique_ptr<Input>             m_input_ptr;
    std::vector<std::function<void()>> m_post_render_jobs;
    // scene state
    GLuint m_base_map   = 0;
    GLuint m_bump_map   = 0;
    GLuint m_program_id = 0;
    // shader constants location
    GLint m_tangent_atr     = 0;
    GLint m_bitangent_atr   = 0;
    GLint m_light_pos_id    = 0;
    GLint m_cam_pos_id      = 0;
    GLint m_ambient_col_id  = 0;
    GLint m_specular_col_id = 0;
    GLint m_specular_pow_id = 0;
    // buffers
    GLuint  m_vertexbuffer    = 0;
    GLuint  m_uvbuffer        = 0;
    GLuint  m_normalbuffer    = 0;
    GLuint  m_tangentbuffer   = 0;
    GLuint  m_bitangentbuffer = 0;
    GLsizei m_indices_size    = 0;
    GLuint  m_elementbuffer   = 0;

public:
    evnt::Camera m_cam;
    // World
    Entity m_root;
    Entity m_camera;
    // App
    Registry m_reg;

    Window(int width, int height, char const * title);
    ~Window();

    Window(Window const &)             = delete;
    Window & operator=(Window const &) = delete;

    bool isFullscreen() const { return m_is_fullscreen; }

    void create();
    void initScene();
    void fullscreen(bool is_fullscreen);
    void run();

    // keys
    void key_f1();
};

#endif   // WINDOW_H
