#include "window.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "scene/camera.h"
#include "render/renderer.h"
#include "scene/light.h"
#include "input/inputglfw.h"

namespace
{
char const * mesh_fname        = "test.txt.msh";
char const * anim_fname        = "test.txt.anm";
char const * diffuse_tex_fname = "uv.tga";
// char const *    bump_tex_fname    = "normal.tga";
constexpr float def_speed = 0.02f;
}   // namespace

Window::Window(int width, int height, char const * title) :
    m_size{width, height},
    m_title{title},
    m_arcball{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
    m_reg{},
    m_sys{}
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
        m_render->terminate();
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
    m_arcball.setNewWindowSize(static_cast<uint32_t>(cam.m_vp_size.x),
                               static_cast<uint32_t>(cam.m_vp_size.y));

    // bind keys
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_W, std::bind(&Window::moveForward, this, def_speed),
                                "move forward");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_S, std::bind(&Window::moveForward, this, -def_speed),
                                "move backward");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_A, std::bind(&Window::moveSideward, this, -def_speed),
                                "move left");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_D, std::bind(&Window::moveSideward, this, def_speed),
                                "move right");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_Z, std::bind(&Window::moveUp, this, def_speed), "move up");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_X, std::bind(&Window::moveUp, this, -def_speed),
                                "move down");

    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_C, std::bind(&Window::objMoveUp, this, def_speed),
                                "move up");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_V, std::bind(&Window::objMoveUp, this, -def_speed),
                                "move down");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_F, std::bind(&Window::objMoveSide, this, def_speed),
                                "move side");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_G, std::bind(&Window::objMoveSide, this, -def_speed),
                                "move side");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_B, std::bind(&Window::objRotateUp, this, def_speed),
                                "rotate up");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_N, std::bind(&Window::objRotateUp, this, -def_speed),
                                "rotate down");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_M, std::bind(&Window::objRotateSide, this, def_speed),
                                "rotate side");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_K, std::bind(&Window::objRotateSide, this, -def_speed),
                                "rotate side");

    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_Q, std::bind(&Window::objCreate, this), "create cube");
    m_input_ptr->bindKeyFunctor(KeyboardKey::Key_E, std::bind(&Window::objDelete, this), "delete cube");
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
    // always first
    m_entity_creator_sys = std::make_shared<EntityCreatorSystem>(m_reg);
    m_sys.addSystem(m_entity_creator_sys);

    m_scene_sys = std::make_shared<SceneSystem>(m_reg);
    m_sys.addSystem(m_scene_sys);

    std::shared_ptr<ISystem> ptr;
    ptr = std::make_shared<CameraSystem>(m_reg);
    m_sys.addSystem(ptr);

    ptr = std::make_shared<LightSystem>(m_reg);
    m_sys.addSystem(ptr);

    ptr = std::make_shared<JointSystem>(m_reg);
    m_sys.addSystem(ptr);

    // update joints transform matrices
    // m_sys.addSystem(m_scene_sys);

    m_model_sys = std::make_shared<ModelSystem>(m_reg);
    m_sys.addSystem(m_model_sys);

    m_render = std::make_shared<Renderer>(m_reg);
    m_sys.addSystem(m_render);

    // always last
    ptr = std::make_shared<EntityDeleterSystem>(m_reg);
    m_sys.addSystem(ptr);

    // add nodes
    Event::Scene::TransformComponent transform{};
    transform.replase_local_matrix = true;

    // root
    auto root = EntityBuilder::BuildEntity(m_reg, pos_flags);
    m_scene_sys->connectNode(root);
    // camera
    m_camera   = EntityBuilder::BuildEntity(m_reg, cam_flags);
    auto & cam = m_reg.get<CameraComponent>(m_camera);

    cam.m_vp_size.x = width;
    cam.m_vp_size.y = height;

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    CameraSystem::SetupProjMatrix(cam, 45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Set cam transform
    glm::mat4 view    = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 10.0f));
    transform.new_mat = glm::rotate(view, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_reg.add_component<Event::Scene::TransformComponent>(m_camera, transform);

    m_scene_sys->connectNode(m_camera, root);
    // light
    auto light_id = EntityBuilder::BuildEntity(m_reg, light_flags);

    auto & light_pos = m_reg.get<SceneComponent>(light_id);
    light_pos.rel    = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));

    m_scene_sys->connectNode(light_id, root);

    light_id = EntityBuilder::BuildEntity(m_reg, light_flags);

    light_pos     = m_reg.get<SceneComponent>(light_id);
    light_pos.rel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));

    m_scene_sys->connectNode(light_id, root);

    return true;
}

void Window::initScene()
{
    if(!m_sys.initSystems())
        throw std::runtime_error{"Failed to init systems"};

    auto func = [this]() {
        m_model = EntityBuilder::BuildEntity(m_reg, obj_flags);

        Event::Model::CreateModel cm_event{m_scene_sys.get(),        // scene_system pointer
                                           m_scene_sys->getRoot(),   // parent node
                                           mesh_fname,               // mesh data file
                                           anim_fname,               // anim data file
                                           diffuse_tex_fname,        // material data file
                                           glm::mat4(1.0f)};         // relative matrix

        m_reg.add_component<Event::Model::CreateModel>(m_model, std::move(cm_event));
    };
    m_entity_creator_sys->addCreatorFunctor(std::move(func));

    m_cube = null_entity_id;

    //m_scene_sys->connectNode(m_model, m_scene_sys->getRoot());
}

void Window::run()
{
    m_sys.update(0.0);

    do
    {
        m_input_ptr->update();
        // Clear the screen
        m_render->clearBuffers();

        auto const & cam = m_reg.get<CameraComponent>(m_camera);
        m_render->setMatrix(Renderer::MatrixType::PROJECTION, cam.m_proj_mat);

        m_scene_sys->updateQueues(cam.m_frustum, nullptr);

        // set lights
        for(uint32_t i = 0; i < m_scene_sys->getLightQueue().size() && i < 8; ++i)
        {
            if(i == 0)
                m_render->lighting();

            auto const light_id = m_scene_sys->getLightQueue()[i];

            m_render->bindLight(light_id, i);
        }

        for(auto node : m_scene_sys->getModelsQueue())
        {
            auto const & node_pos   = m_reg.get<SceneComponent>(node);
            glm::mat4    model_view = cam.m_view_mat * node_pos.abs;

            m_render->bindMaterial(node);
            m_render->setMatrix(Renderer::MatrixType::MODELVIEW, model_view);
            m_render->draw(node);
            m_render->drawBBox(node);
        }

        for(uint32_t i = 0; i < m_scene_sys->getLightQueue().size() && i < 8; ++i)
        {
            if(i == 0)
                m_render->lighting(false);

            m_render->unbindLight(i);
        }

        m_render->loadIdentityMatrix(Renderer::MatrixType::PROJECTION);
        m_render->loadIdentityMatrix(Renderer::MatrixType::MODELVIEW);

        // Swap buffers
        glfwSwapBuffers(mp_glfw_win);
        glfwPollEvents();

        m_sys.update(glfwGetTime() / 10.0);

        if(m_input_ptr->isKeyPressed(KeyboardKey::Key_F1))
            key_f1();

        m_arcball.update(m_input_ptr.get(), m_camera, m_reg);
    }   // Check if the ESC key was pressed or the window was closed
    while(!m_input_ptr->isKeyPressed(KeyboardKey::Key_Escape) && glfwWindowShouldClose(mp_glfw_win) == 0);
}

void Window::moveForward(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_camera);

    glm::vec4 dir     = pos.abs * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec4 new_pos = dir * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_camera, tr_cmp);
}

void Window::moveSideward(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_camera);

    glm::vec4 right   = pos.abs * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 new_pos = right * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_camera, tr_cmp);
}

void Window::moveUp(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_camera);

    glm::vec4 up      = pos.abs * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 new_pos = up * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_camera, tr_cmp);
}

void Window::objMoveUp(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_model);

    glm::vec4 up      = pos.abs * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 new_pos = up * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_model, tr_cmp);
}

void Window::objMoveSide(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_model);

    glm::vec4 up      = pos.abs * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 new_pos = up * speed;

    glm::mat4 new_trans = glm::translate(glm::mat4(1.0f), glm::vec3(new_pos));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_model, tr_cmp);
}

void Window::objRotateUp(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_model);

    glm::vec4 up        = pos.abs * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::mat4 new_trans = glm::rotate(glm::mat4(1.0f), speed, glm::vec3(up));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_model, tr_cmp);
}

void Window::objRotateSide(float speed)
{
    auto const & pos = m_reg.get<SceneComponent>(m_model);

    glm::vec4 up        = pos.abs * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    glm::mat4 new_trans = glm::rotate(glm::mat4(1.0f), speed, glm::vec3(up));

    Event::Scene::TransformComponent tr_cmp;
    tr_cmp.replase_local_matrix = false;
    tr_cmp.new_mat              = new_trans;

    m_reg.add_component<Event::Scene::TransformComponent>(m_model, tr_cmp);
}

void Window::objCreate()
{
    if(m_reg.valid(m_cube))
        return;

    auto func = [this]() {
        auto joint_id = m_model_sys->getJointIdFromName(m_model, "foot");
        if(joint_id)
        {
            m_cube = EntityBuilder::BuildEntity(m_reg, obj_flags);

            Event::Model::CreateModel cm_event{
                m_scene_sys.get(),                                       // scene_system pointer
                *joint_id,                                               // parent node
                "cube.txt.msh",                                          // mesh data file
                "",                                                      // anim data file
                diffuse_tex_fname,                                       // material data file
                glm::translate(glm::mat4(1.0f), {0.0f, -5.0f, 0.0f})};   // relative matrix

            m_reg.add_component<Event::Model::CreateModel>(m_cube, std::move(cm_event));
        }
    };

    m_entity_creator_sys->addCreatorFunctor(std::move(func));
}

void Window::objDelete()
{
    if(!m_reg.valid(m_cube))
        return;

    m_reg.add_component<Event::Model::DestroyModel>(m_cube);
    m_cube = null_entity_id;
}

void Window::key_f1()
{
    fullscreen(!m_is_fullscreen);
}
