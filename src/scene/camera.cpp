#include "camera.h"
#include "scene.h"
#include <cstring>

namespace evnt
{
Camera::Camera() :
    m_view_mat{1.0f},
    m_proj_mat{1.0f},
    m_frustum{},
    m_abs_pos{0.0f, 0.0f, 0.0f},
    m_vp_pos{0, 0},
    m_vp_size{640, 480},
    m_frust_left{0.0f},
    m_frust_right{0.0f},
    m_frust_bottom{0.0f},
    m_frust_top{0.0f},
    m_frust_near{0.0f},
    m_frust_far{0.0f},
    m_orthographic{false},
    m_manual_proj_mat{false}
{}

void Camera::setupViewParams(float fov, float aspect, float near_plane, float far_plane)
{
    float ymax = near_plane * glm::tan(glm::radians(fov / 2));
    float xmax = ymax * aspect;

    m_frust_left   = -xmax;
    m_frust_right  = xmax;
    m_frust_bottom = -ymax;
    m_frust_top    = ymax;
    m_frust_near   = near_plane;
    m_frust_far    = far_plane;

    // setting view params implicitly disables the manual projection matrix
    m_manual_proj_mat = false;
    // build frustum
    transformUpdate(glm::inverse(m_view_mat));
}

void Camera::setProjectionMatrix(float * proj_mat)
{
    std::memcpy(&m_proj_mat[0], proj_mat, 16 * sizeof(float));
    m_manual_proj_mat = true;
}

void Camera::transformUpdate(glm::mat4 const & new_trans)
{
    // Get position
    m_abs_pos = glm::vec3(new_trans[3][0], new_trans[3][1], new_trans[3][2]);

    // Calculate view matrix
    m_view_mat = glm::inverse(new_trans);

    // Calculate projection matrix if not using a manually set one
    if(!m_manual_proj_mat)
    {
        if(!m_orthographic)
            m_proj_mat = glm::frustum(m_frust_left, m_frust_right, m_frust_bottom, m_frust_top, m_frust_near,
                                      m_frust_far);
        else
            m_proj_mat = glm::ortho(m_frust_left, m_frust_right, m_frust_bottom, m_frust_top, m_frust_near,
                                    m_frust_far);
    }

    // Update frustum
    m_frustum.buildViewFrustum(m_view_mat, m_proj_mat);
}
}   // namespace evnt

CameraComponent CameraSystem::GetDefaultCamComponent()
{
    CameraComponent new_cam;

    new_cam.m_entity_id  = null_entity_id;
    new_cam.m_abs_pos    = glm::vec3{0.0f};
    new_cam.m_view_mat   = glm::mat4{1.0f};
    new_cam.m_proj_mat   = glm::mat4{1.0f};
    new_cam.m_frustum    = evnt::Frustum{};
    new_cam.m_vp_pos     = glm::ivec2{0};
    new_cam.m_vp_size    = glm::ivec2{640, 480};
    new_cam.m_frust_left = new_cam.m_frust_right = new_cam.m_frust_bottom = new_cam.m_frust_top =
        new_cam.m_frust_near = new_cam.m_frust_far = 0.0f;
    new_cam.m_orthographic                         = false;

    return new_cam;
}

void CameraSystem::update(entt::DefaultRegistry & reg, float time_delta)
{
    for(auto ent : reg.view<evnt::SceneComponent, CameraComponent>())
    {
        auto & pos = reg.get<evnt::SceneComponent>(ent);
        auto & cam = reg.get<CameraComponent>(ent);

        if(pos.is_transformed)
        {
            setupViewMatrix(cam, pos.abs);
        }
    }
}

void CameraSystem::setupProjMatrix(CameraComponent & cam, float fov, float aspect, float near_plane,
                                   float far_plane)
{
    float ymax = near_plane * glm::tan(glm::radians(fov / 2));
    float xmax = ymax * aspect;

    cam.m_frust_left   = -xmax;
    cam.m_frust_right  = xmax;
    cam.m_frust_bottom = -ymax;
    cam.m_frust_top    = ymax;
    cam.m_frust_near   = near_plane;
    cam.m_frust_far    = far_plane;

    if(!cam.m_orthographic)
        cam.m_proj_mat = glm::frustum(cam.m_frust_left, cam.m_frust_right, cam.m_frust_bottom,
                                      cam.m_frust_top, cam.m_frust_near, cam.m_frust_far);
    else
        cam.m_proj_mat = glm::ortho(cam.m_frust_left, cam.m_frust_right, cam.m_frust_bottom, cam.m_frust_top,
                                    cam.m_frust_near, cam.m_frust_far);

    cam.m_frustum.buildViewFrustum(cam.m_view_mat, cam.m_proj_mat);
}

void CameraSystem::setupViewMatrix(CameraComponent & cam, glm::mat4 const & new_trans)
{
    cam.m_abs_pos  = glm::vec3(new_trans[3][0], new_trans[3][1], new_trans[3][2]);
    cam.m_view_mat = glm::inverse(new_trans);

    cam.m_frustum.buildViewFrustum(cam.m_view_mat, cam.m_proj_mat);
}
