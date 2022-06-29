#include "window.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

#include "res/imagedata.h"
#include "res/meshdata.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/glmesh.h"
#include "input/inputglfw.h"

namespace
{
char const * mesh_fname        = "sphere.txt.msh";
char const * diffuse_tex_fname = "diffuse.tga";
char const * bump_tex_fname    = "normal.tga";
}   // namespace

Window::Window(int width, int height, char const * title) :
    m_size{width, height}, m_title{title}, m_scene_sys{nullptr}, m_reg{}, m_sys{m_reg}
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
    auto & geom = m_reg.get<GlMeshComponent>(m_model);
    // Cleanup VBO and shader
    if(mp_glfw_win)
    {
        UnloadFromGL(geom);
        glDeleteTextures(1, &m_base_map);
        glDeleteTextures(1, &m_bump_map);
        glDeleteProgram(m_program_id);
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
    if(mp_glfw_win != nullptr)
    {
        new_window = glfwCreateWindow(cam.m_vp_size.x, cam.m_vp_size.y, "", mon, mp_glfw_win);
        glfwDestroyWindow(mp_glfw_win);
    }
    else
    {
        new_window = glfwCreateWindow(cam.m_vp_size.x, cam.m_vp_size.y, "", mon, nullptr);
    }

    mp_glfw_win = new_window;
    if(mp_glfw_win == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error{"Failed to create GLFW window"};
    }
    glfwMakeContextCurrent(mp_glfw_win);
    glfwSetWindowTitle(mp_glfw_win, m_title.c_str());

    glViewport(cam.m_vp_pos.x, cam.m_vp_pos.y, cam.m_vp_size.x, cam.m_vp_size.y);
    CameraSystem::SetupProjMatrix(
        cam, 45.0f, static_cast<float>(cam.m_vp_size.x) / static_cast<float>(cam.m_vp_size.y), 0.1f, 100.0f);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(mp_glfw_win, GLFW_STICKY_KEYS, GL_TRUE);

    // Initialize GLEW
    if(glewInit() != GLEW_OK)
    {
        throw std::runtime_error{"Failed to initialize GLEW"};
    }

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_NORMALIZE);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_LIGHTING);

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
    std::unique_ptr<ISystem> ptr;
    ptr         = std::make_unique<evnt::SceneSystem>(m_reg);
    m_scene_sys = static_cast<evnt::SceneSystem *>(ptr.get());
    m_sys.addSystem(std::move(ptr));

    ptr = std::make_unique<CameraSystem>(m_reg);
    // auto * cam_sys = cam_sys_ptr.get()
    m_sys.addSystem(std::move(ptr));

    ptr = std::make_unique<LightSystem>(m_reg);
    m_sys.addSystem(std::move(ptr));

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
    // mesh
    m_model = SceneEntityBuilder::BuildEntity(m_reg, obj_flags);

    m_scene_sys->addNode(m_model, m_root);

    return true;
}

void Window::initScene()
{
    auto & geom = m_reg.get<GlMeshComponent>(m_model);
    auto & mat  = m_reg.get<MaterialComponent>(m_model);

    // Load the textures
    if(!MaterialSystem::LoadTGA(mat, diffuse_tex_fname, bump_tex_fname))
        throw std::runtime_error{"Failed to load texture"};

    // render.uploadMaterialData(m_model);

    {
        if(!tex::ReadTGA(diffuse_tex_fname, mat.m_diff))
            throw std::runtime_error{"Failed to load texture"};

        glGenTextures(1, &m_base_map);
        glBindTexture(GL_TEXTURE_2D, m_base_map);
        glTexImage2D(GL_TEXTURE_2D, 0, mat.m_diff.type == tex::ImageData::PixelType::pt_rgb ? 3 : 4,
                     static_cast<GLsizei>(mat.m_diff.width), static_cast<GLsizei>(mat.m_diff.height), 0,
                     mat.m_diff.type == tex::ImageData::PixelType::pt_rgb ? GL_RGB : GL_RGBA,
                     GL_UNSIGNED_BYTE, mat.m_diff.data.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Load mesh
    if(!LoadGlMeshComponent(mesh_fname, geom))
        throw std::runtime_error{"Failed to load mesh"};

    if(!m_sys.initSystems())
        throw std::runtime_error{"Failed to init systems"};
}

void Window::run()
{
    m_sys.update();

    do
    {
        m_input_ptr->update();
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto const & cam   = m_reg.get<CameraComponent>(m_camera);
        auto const & light = m_reg.get<LightComponent>(m_light);
        auto const & geom  = m_reg.get<GlMeshComponent>(m_model);
        auto const & mat   = m_reg.get<MaterialComponent>(m_model);

        // set lights
        glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(light.position));
        glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(light.ambient));
        glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(light.diffuse));
        glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(light.specular));
        glEnable(GL_LIGHT0);

        // set material
        glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(mat.m_ambient));
        glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(mat.m_diffuse));
        glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(mat.m_specular));
        glMaterialf(GL_FRONT, GL_SHININESS, mat.m_shininess);

        // set matrices
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(cam.m_proj_mat));

        glm::mat4 model      = glm::mat4(1.0f);
        glm::mat4 model_view = cam.m_view_mat * model;

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(model_view));

        // buffers
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER, geom.m_uvbuffer);
        glTexCoordPointer(2, GL_FLOAT, 0, static_cast<char *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, geom.m_vertexbuffer);
        glVertexPointer(3, GL_FLOAT, 0, static_cast<char *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, geom.m_normalbuffer);
        glNormalPointer(GL_FLOAT, 0, static_cast<char *>(nullptr));

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom.m_elementbuffer);

        glBindTexture(GL_TEXTURE_2D, m_base_map);

        // Draw the triangles !
        glDrawElements(GL_TRIANGLES,                    // mode
                       geom.m_indices_size,             // count
                       GL_UNSIGNED_SHORT,               // type
                       static_cast<void *>(nullptr));   // element array buffer offset

        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_LIGHT0);

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
    // auto const & cam = m_reg.get<CameraComponent>(m_camera);

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
