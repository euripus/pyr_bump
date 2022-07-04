#ifndef WINDOW_H
#define WINDOW_H

#include <memory>
#include <string>

#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>

#include "scene/sceneentitybuilder.h"
#include "input/input.h"
#include "scene/scene.h"
#include "scene/camera.h"

class Renderer;

class Window
{
    // window state
    bool                m_is_fullscreen    = false;
    GLFWvidmode const * mp_base_video_mode = nullptr;
    GLFWwindow *        mp_glfw_win        = nullptr;
    glm::ivec2 const    m_size;   // initial size
    std::string         m_title;

    std::unique_ptr<Input> m_input_ptr;

    bool createDefaultScene(int width, int height);

public:
    // World
    Entity              m_root;
    Entity              m_camera;
    Entity              m_light;
    Entity              m_model;
    evnt::SceneSystem * m_scene_sys;
    Renderer *          m_render;
    // App
    Registry   m_reg;
    SystemsMgr m_sys;

    Window(int width, int height, char const * title);
    ~Window();

    Window(Window const &)             = delete;
    Window & operator=(Window const &) = delete;

    bool isFullscreen() const { return m_is_fullscreen; }

    void create();
    void initScene();
    void fullscreen(bool is_fullscreen);
    void run();

    void moveForward(float speed);
    void moveSideward(float speed);
    void moveUp(float speed);

    // keys
    void key_f1();
};

#endif   // WINDOW_H
