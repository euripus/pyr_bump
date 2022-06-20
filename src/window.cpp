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
char const * mesh_fname            = "sphere.txt.msh";
char const * diffuse_tex_fname     = "diffuse.tga";
char const * bump_tex_fname        = "normal.tga";
char const * vertex_shader_fname   = "bump2.0.vert";
char const * fragment_shader_fname = "bump2.0.frag";
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

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
    glDepthFunc(GL_LESS);

    glEnable(GL_TEXTURE_2D);

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

    ptr = std::make_unique<CameraSystem>();
    // auto * cam_sys = cam_sys_ptr.get()
    m_sys.addSystem(std::move(ptr));

    ptr = std::make_unique<LightSystem>();
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
    {
        tex::ImageData tex_data;
        if(!tex::ReadTGA(diffuse_tex_fname, tex_data))
            throw std::runtime_error{"Failed to load texture"};

        glGenTextures(1, &m_base_map);
        glBindTexture(GL_TEXTURE_2D, m_base_map);
        glTexImage2D(GL_TEXTURE_2D, 0, tex_data.type == tex::ImageData::PixelType::pt_rgb ? 3 : 4,
                     static_cast<GLsizei>(tex_data.width), static_cast<GLsizei>(tex_data.height), 0,
                     tex_data.type == tex::ImageData::PixelType::pt_rgb ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE,
                     tex_data.data.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    {
        tex::ImageData tex_data;
        if(!tex::ReadTGA(bump_tex_fname, tex_data))
            throw std::runtime_error{"Failed to load texture"};

        glGenTextures(1, &m_bump_map);
        glBindTexture(GL_TEXTURE_2D, m_bump_map);
        glTexImage2D(GL_TEXTURE_2D, 0, tex_data.type == tex::ImageData::PixelType::pt_rgb ? 3 : 4,
                     static_cast<GLsizei>(tex_data.width), static_cast<GLsizei>(tex_data.height), 0,
                     tex_data.type == tex::ImageData::PixelType::pt_rgb ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE,
                     tex_data.data.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Load mesh
    if(!LoadGlMeshComponent(mesh_fname, geom))
        throw std::runtime_error{"Failed to load mesh"};

    // Shader load and compiling
    // Read Shaders code from the file
    std::string vertex_shader_code;
    std::string fragment_shader_code;

    {
        std::ifstream ifile(vertex_shader_fname, std::ios::in);
        if(!ifile.is_open())
            throw std::runtime_error{"Failed to load vertex shader"};

        std::stringstream sstr;
        sstr << ifile.rdbuf();
        vertex_shader_code = sstr.str();
        ifile.close();
    }
    {
        std::ifstream ifile(fragment_shader_fname, std::ios::in);
        if(!ifile.is_open())
            throw std::runtime_error{"Failed to load fragment shader"};

        std::stringstream sstr;
        sstr << ifile.rdbuf();
        fragment_shader_code = sstr.str();
        ifile.close();
    }

    GLint result = GL_FALSE;
    int   info_log_length{0};

    // Create the shaders
    GLuint vertex_shader_id   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_iD = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Vertex Shader
    char const * vertex_source_pointer = vertex_shader_code.c_str();
    glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr);
    glCompileShader(vertex_shader_id);
    // Check Vertex Shader
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0)
    {
        std::vector<char> vertex_shader_error_message(static_cast<size_t>(info_log_length + 1));
        glGetShaderInfoLog(vertex_shader_id, info_log_length, nullptr, vertex_shader_error_message.data());
        std::cout << vertex_shader_error_message.data() << std::endl;
    }

    // Compile Fragment Shader
    char const * fragment_source_pointer = fragment_shader_code.c_str();
    glShaderSource(fragment_shader_iD, 1, &fragment_source_pointer, nullptr);
    glCompileShader(fragment_shader_iD);
    // Check Fragment Shader
    glGetShaderiv(fragment_shader_iD, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragment_shader_iD, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0)
    {
        std::vector<char> fragment_shader_error_message(static_cast<size_t>(info_log_length + 1));
        glGetShaderInfoLog(fragment_shader_iD, info_log_length, nullptr,
                           fragment_shader_error_message.data());
        std::cout << fragment_shader_error_message.data() << std::endl;
    }

    // Link the program
    m_program_id = glCreateProgram();
    glAttachShader(m_program_id, vertex_shader_id);
    glAttachShader(m_program_id, fragment_shader_iD);
    glLinkProgram(m_program_id);
    // Check the program
    glGetProgramiv(m_program_id, GL_LINK_STATUS, &result);
    glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0)
    {
        std::vector<char> program_error_message(static_cast<size_t>(info_log_length + 1));
        glGetProgramInfoLog(m_program_id, info_log_length, nullptr, program_error_message.data());
        std::cout << program_error_message.data() << std::endl;
    }

    glDetachShader(m_program_id, vertex_shader_id);
    glDetachShader(m_program_id, fragment_shader_iD);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_iD);

    m_tangent_atr     = glGetAttribLocation(m_program_id, "rm_tangent");
    m_bitangent_atr   = glGetAttribLocation(m_program_id, "rm_bitang");
    m_light_pos_id    = glGetUniformLocation(m_program_id, "light_pos_world");
    m_cam_pos_id      = glGetUniformLocation(m_program_id, "cam_pos_world");
    m_ambient_col_id  = glGetUniformLocation(m_program_id, "ambient_col");
    m_specular_col_id = glGetUniformLocation(m_program_id, "specular_col");
    m_specular_pow_id = glGetUniformLocation(m_program_id, "specular_pow");

    glUseProgram(m_program_id);
    glUniform1i(glGetUniformLocation(m_program_id, "base_map"), 0);
    glUniform1i(glGetUniformLocation(m_program_id, "bump_map"), 1);
    glUseProgram(0);
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

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(cam.m_proj_mat));

        glm::mat4 model      = glm::mat4(1.0f);
        glm::mat4 model_view = cam.m_view_mat * model;

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(model_view));

        glUseProgram(m_program_id);

        glUniform3fv(m_light_pos_id, 1, glm::value_ptr(glm::vec3(light.position)));
        glUniform3fv(m_cam_pos_id, 1, glm::value_ptr(cam.m_abs_pos));
        glUniform3f(m_ambient_col_id, 0.1f, 0.1f, 0.1f);
        glUniform3f(m_specular_col_id, 1.0f, 1.0f, 1.0f);
        glUniform1f(m_specular_pow_id, 25.0f);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableVertexAttribArray(m_tangent_atr);
        glEnableVertexAttribArray(m_bitangent_atr);

        glBindBuffer(GL_ARRAY_BUFFER, geom.m_uvbuffer);
        glTexCoordPointer(2, GL_FLOAT, 0, static_cast<char *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, geom.m_vertexbuffer);
        glVertexPointer(3, GL_FLOAT, 0, static_cast<char *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, geom.m_normalbuffer);
        glNormalPointer(GL_FLOAT, 0, static_cast<char *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, geom.m_tangentbuffer);
        glVertexAttribPointer(m_tangent_atr, 3, GL_FLOAT, GL_FALSE, 0, static_cast<char *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, geom.m_bitangentbuffer);
        glVertexAttribPointer(m_bitangent_atr, 3, GL_FLOAT, GL_FALSE, 0, static_cast<char *>(nullptr));

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom.m_elementbuffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_base_map);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_bump_map);

        // Draw the triangles !
        glDrawElements(GL_TRIANGLES,                    // mode
                       geom.m_indices_size,             // count
                       GL_UNSIGNED_SHORT,               // type
                       static_cast<void *>(nullptr));   // element array buffer offset

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableVertexAttribArray(m_bitangent_atr);
        glDisableVertexAttribArray(m_tangent_atr);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glUseProgram(0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

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
