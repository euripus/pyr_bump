#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "AABB.h"
#include "plane.h"

namespace evnt
{
class Frustum
{
public:
    glm::vec3 const & getOrigin() const { return m_origin; }

    glm::vec3 const & getCorner(uint32_t index) const { return m_corners[index]; }

    void buildViewFrustum(glm::mat4 const & trans_mat, float fov, float aspect, float near_plane,
                          float far_plane);
    void buildViewFrustum(glm::mat4 const & trans_mat, float left, float right, float bottom, float top,
                          float near_plane, float far_plane);
    void buildViewFrustum(glm::mat4 const & view_mat, glm::mat4 const & proj_mat);
    void buildBoxFrustum(glm::mat4 const & trans_mat, float left, float right, float bottom, float top,
                         float front, float back);
    bool cullSphere(glm::vec3 pos, float rad) const;
    bool cullBox(AABB const & b) const;
    bool cullFrustum(Frustum const & frust) const;

    void calcAABB(glm::vec3 & mins, glm::vec3 & maxs) const;

private:
    Plane     m_planes[6];   // Planes of frustum
    glm::vec3 m_origin;
    glm::vec3 m_corners[8];   // Corner points
};
}   // namespace evnt

#endif   // FRUSTUM_H
