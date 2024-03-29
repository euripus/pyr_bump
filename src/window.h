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
#include "input/arcball.h"
#include "scene/scenecmp.h"
#include "scene/model.h"

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
    Arcball                m_arcball;

    bool createDefaultScene(int width, int height);

public:
    // World
    Entity                               m_camera;
    Entity                               m_model;
    Entity                               m_cube;
    std::shared_ptr<EntityCreatorSystem> m_entity_creator_sys;
    std::shared_ptr<SceneSystem>         m_scene_sys;
    std::shared_ptr<ModelSystem>         m_model_sys;
    std::shared_ptr<Renderer>            m_render;
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

    // camera move
    void moveForward(float speed);
    void moveSideward(float speed);
    void moveUp(float speed);
    // object move
    void objMoveUp(float speed);
    void objMoveSide(float speed);
    void objRotateUp(float speed);
    void objRotateSide(float speed);
    // object creation
    void objCreate();
    void objDelete();

    // keys
    void key_f1();
};

#endif   // WINDOW_H
