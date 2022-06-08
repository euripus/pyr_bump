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

    bool        init() override;
    void        update(entt::DefaultRegistry & reg, float time_delta) override;
    std::string getName() const override { return "Camera"; }

    void setupProjMatrix(CameraComponent & cam, float fov, float aspect, float near_plane, float far_plane);
    // new_trans - transform matrix from pos component
    void setupViewMatrix(CameraComponent & cam, glm::mat4 const & new_trans);
};

namespace evnt
{
class Camera
{
public:
    Camera();
    ~Camera() = default;

    void setupViewParams(float fov, float aspect, float near_plane, float far_plane);
    void setProjectionMatrix(float * proj_mat);

    Frustum const &   getFrustum() { return m_frustum; }
    glm::mat4 const & getViewMat() { return m_view_mat; }
    glm::mat4 const & getProjMat() { return m_proj_mat; }
    glm::vec3 const & getAbsPos() { return m_abs_pos; }

    // new_trans - transform matrix from pos component
    void transformUpdate(glm::mat4 const & new_trans);

private:
    glm::mat4  m_view_mat, m_proj_mat;
    Frustum    m_frustum;
    glm::vec3  m_abs_pos;
    glm::ivec2 m_vp_pos;
    glm::ivec2 m_vp_size;
    float      m_frust_left, m_frust_right, m_frust_bottom, m_frust_top, m_frust_near, m_frust_far;
    bool       m_orthographic;      // Perspective or orthographic frustum?
    bool       m_manual_proj_mat;   // Projection matrix manually set?
};
}   // namespace evnt

#endif   // CAMERA_H
