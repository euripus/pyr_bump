#include "window.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "scene/camera.h"
#include "render/renderer.h"
#include "scene/light.h"
//#include "scene/material.h"
#include "input/inputglfw.h"

namespace
{
char const * mesh_fname        = "test.txt.msh";
char const * anim_fname        = "test.txt.anm";
char const * diffuse_tex_fname = "diffuse.tga";
char const * bump_tex_fname    = "normal.tga";
}   // namespace

Window::Window(int width, int height, char const * title) :
    m_size{width, height}, m_title{title}, m_reg{}, m_sys{}
{
    // Create scene
    if(!createDefaultScene(width, height))
        throw std::runtime_error{"Failed to create scene."};

    // Initialise GLFW
    if(!glfwInit())
    {
        throw std::runtime_error{"Failed to initialize GLFW"};
    }

    mp_base_video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
}

Window::~Window()
{
    // Cleanup VBO and shader
    if(mp_glfw_win)
    {
        m_render->unloadModel(m_model);
        m_render->unloadMaterialData(m_model);
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void Window::create()
{
    auto & cam = m_reg.get<CameraComponent>(m_camera);

    GLFWmonitor * mon;
    if(m_is_fullscreen)
    {
        mon             = glfwGetPrimaryMonitor();
        cam.m_vp_size.x = mp_base_video_mode->width;
        cam.m_vp_size.y = mp_base_video_mode->height;
    }
    else
    {
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        mon             = nullptr;
        cam.m_vp_size.x = m_size.x;
        cam.m_vp_size.y = m_size.y;
    }

    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    GLFWwindow * new_window{nullptr};
    new_window = glfwCreateWindow(cam.m_vp_size.x, cam.m_vp_size.y, "", mon, mp_glfw_win);
    if(mp_glfw_win != nullptr)
        glfwDestroyWindow(mp_glfw_win);

    mp_glfw_win = new_window;
    if(mp_glfw_win == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error{"Failed to create GLFW window"};
    }
    glfwMakeContextCurrent(mp_glfw_win);
    glfwSetWindowTitle(mp_glfw_win, m_title.c_str());

    // Initialize GLEW
    if(glewInit() != GLEW_OK)
    {
        throw std::runtime_error{"Failed to initialize GLEW"};
    }

    m_render->setViewport(cam.m_vp_pos.x, cam.m_vp_pos.y, cam.m_vp_size.x, cam.m_vp_size.y);
    CameraSystem::SetupProjMatrix(
        cam, 45.0f, static_cast<float>(cam.m_vp_size.x) / static_cast<float>(cam.m_vp_size.y), 0.1f, 100.0f);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(mp_glfw_win, GLFW_STICKY_KEYS, GL_TRUE);

    m_render->init();
    m_render->setClearColor(glm::vec4(0.0f, 0.0f, 0.4f, 0.0f));

    // input backend
    m_input_ptr = std::make_unique<InputGLFW>(mp_glfw_win);

    // bind keys
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_W, std::bind(&Window::moveForward, this, 0.02f),
                                "move forward");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_S, std::bind(&Window::moveForward, this, -0.02),
                                "move backward");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_A, std::bind(&Window::moveSideward, this, -0.02f),
                                "move left");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_D, std::bind(&Window::moveSideward, this, 0.02),
                                "move right");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_Z, std::bind(&Window::moveUp, this, 0.02f), "move up");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_X, std::bind(&Window::moveUp, this, -0.02), "move down");
}

void Window::fullscreen(bool is_fullscreen)
{
    if(is_fullscreen == m_is_fullscreen)
        return;

    m_is_fullscreen = is_fullscreen;
    create();
}

bool Window::createDefaultScene(int width, int height)
{
    // create systems
    m_scene_sys = std::make_shared<evnt::SceneSystem>(m_reg);
    m_sys.addSystem(m_scene_sys);

    std::shared_ptr<ISystem> ptr;
    ptr = std::make_shared<CameraSystem>(m_reg);
    m_sys.addSystem(ptr);

    ptr = std::make_shared<LightSystem>(m_reg);
    m_sys.addSystem(ptr);

    ptr = std::make_shared<JointSystem>(m_reg);
    m_sys.addSystem(ptr);

    // update joints transform matrices
    m_sys.addSystem(m_scene_sys);

    m_model_sys = std::make_shared<ModelSystem>(m_reg);
    m_sys.addSystem(m_model_sys);

    m_render = std::make_shared<Renderer>(m_reg);
    m_sys.addSystem(m_render);

    // add nodes
    evnt::TransformComponent transform{};
    transform.replase_local_matrix = true;

    // root
    m_root = SceneEntityBuilder::BuildEntity(m_reg, pos_flags);
    m_scene_sys->addNode(m_root);
    // camera
    m_camera   = SceneEntityBuilder::BuildEntity(m_reg, cam_flags);
    auto & cam = m_reg.get<CameraComponent>(m_camera);

    cam.m_vp_size.x = width;
    cam.m_vp_size.y = height;

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    CameraSystem::SetupProjMatrix(cam, 45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Set cam transform
    glm::mat4 view    = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 3.0f));
    transform.new_mat = glm::rotate(view, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_reg.add_component<evnt::TransformComponent>(m_camera, transform);

    m_scene_sys->addNode(m_camera, m_root);
    // light
    m_light = SceneEntityBuilder::BuildEntity(m_reg, light_flags);

    transform.new_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
    m_reg.add_component<evnt::TransformComponent>(m_light, transform);

    m_scene_sys->addNode(m_light, m_root);

    return true;
}

void Window::initScene()
{
    if(!m_sys.initSystems())
        throw std::runtime_error{"Failed to init systems"};

    // mesh
    m_model = m_model_sys->loadModel(*m_scene_sys.get(), mesh_fname, anim_fname);

    m_render->uploadMaterialData(m_model);
    m_render->uploadModel(m_model);

    m_scene_sys->addNode(m_model, m_root);
}

void Window::run()
{
    m_sys.update();

    do
    {
        m_input_ptr->update();
        // Clear the screen
        m_render->clearBuffers();

        auto const & cam     = m_reg.get<CameraComponent>(m_camera);
        auto const & mdl_pos = m_reg.get<evnt::SceneComponent>(m_model);

        // set lights
        m_render->lighting();
        m_render->bindLight(m_light);

        // set material
        m_render->bindMaterial(m_model);

        // set matrices
        m_render->setMatrix(Renderer::MatrixType::PROJECTION, cam.m_proj_mat);

        glm::mat4 model_view = cam.m_view_mat * mdl_pos.abs;
        m_render->setMatrix(Renderer::MatrixType::MODELVIEW, model_view);

        m_render->draw(m_model);

        // glBindTexture(GL_TEXTURE_2D, 0);
        m_render->unbindLight();
        m_render->lighting(false);

        m_render->loadIdentityMatrix(Renderer::MatrixType::PROJECTION);
        m_render->loadIdentityMatrix(Renderer::MatrixType::MODELVIEW);

        // Swap buffers
        glfwSwapBuffers(mp_glfw_win);
        glfwPollEvents();

        m_sys.update();

        if(m_input_ptr->isKeyPressed(KeyboardKey::Key_F1))
            key_f1();
    }   // Check if the ESC key was pressed or the window was closed
    while(!m_input_ptr->isKeyPressed(KeyboardKey::Key_Escape) && glfwWindowShouldClose(mp_glfw_win) == 0);
}

void Window::moveForward(float speed)
{
    auto const & pos = m_reg.get<evnt::SceneComponent>(m_camera);

    glm::vec4 dir     = pos.abs * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec4 new_pos = dir * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    evnt::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<evnt::TransformComponent>(m_camera, tr_cmp);
}

void Window::moveSideward(float speed)
{
    auto const & pos = m_reg.get<evnt::SceneComponent>(m_camera);

    glm::vec4 right   = pos.abs * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 new_pos = right * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    evnt::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<evnt::TransformComponent>(m_camera, tr_cmp);
}

void Window::moveUp(float speed)
{
    auto const & pos = m_reg.get<evnt::SceneComponent>(m_camera);

    glm::vec4 up      = pos.abs * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 new_pos = up * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    evnt::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<evnt::TransformComponent>(m_camera, tr_cmp);
}

void Window::key_f1()
{
    fullscreen(!m_is_fullscreen);
}
