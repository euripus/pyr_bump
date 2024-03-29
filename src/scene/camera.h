#ifndef CAMERA_H
#define CAMERA_H

#include "frustum.h"
#include "sceneentitybuilder.h"

struct CameraComponent
{
    Entity        m_entity_id;
    glm::mat4     m_view_mat;
    glm::mat4     m_proj_mat;
    evnt::Frustum m_frustum;
    glm::vec3     m_abs_pos;
    glm::ivec2    m_vp_pos;
    glm::ivec2    m_vp_size;
    float         m_frust_left, m_frust_right, m_frust_bottom, m_frust_top, m_frust_near, m_frust_far;
    bool          m_orthographic;   // Perspective or orthographic frustum?
};

class CameraSystem : public ISystem
{
public:
    static CameraComponent GetDefaultCamComponent();
    static void            SetupProjMatrix(CameraComponent & cam, float fov, float aspect, float near_plane,
                                           float far_plane);
    static void            SetupViewMatrix(CameraComponent & cam, glm::mat4 const & new_trans);

    CameraSystem(Registry & reg) : ISystem(reg) {}

    // bool        init() override;
    void        update(double time) override;
    std::string getName() const override { return "Camera"; }
};

#endif   // CAMERA_H
