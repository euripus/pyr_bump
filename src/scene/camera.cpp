#include "camera.h"
#include "scene.h"
#include <cstring>

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

void CameraSystem::update(Registry & reg, float time_delta)
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

void CameraSystem::SetupProjMatrix(CameraComponent & cam, float fov, float aspect, float near_plane,
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

void CameraSystem::SetupViewMatrix(CameraComponent & cam, glm::mat4 const & new_trans)
{
    cam.m_abs_pos  = glm::vec3(new_trans[3][0], new_trans[3][1], new_trans[3][2]);
    cam.m_view_mat = glm::inverse(new_trans);

    cam.m_frustum.buildViewFrustum(cam.m_view_mat, cam.m_proj_mat);
}
