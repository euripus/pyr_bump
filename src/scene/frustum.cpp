#include "frustum.h"

namespace evnt
{
void Frustum::buildViewFrustum(glm::mat4 const & trans_mat, float fov, float aspect, float near_plane,
                               float far_plane)
{
    float ymax = near_plane * glm::tan(glm::radians(fov / 2));
    float xmax = ymax * aspect;

    buildViewFrustum(trans_mat, -xmax, xmax, -ymax, ymax, near_plane, far_plane);
}

void Frustum::buildViewFrustum(glm::mat4 const & trans_mat, float left, float right, float bottom, float top,
                               float near_plane, float far_plane)
{
    // Use intercept theorem to get params for far plane
    float left_f   = left * far_plane / near_plane;
    float right_f  = right * far_plane / near_plane;
    float bottom_f = bottom * far_plane / near_plane;
    float top_f    = top * far_plane / near_plane;

    // Get points on near plane
    m_corners[0] = glm::vec3(left, bottom, -near_plane);
    m_corners[1] = glm::vec3(right, bottom, -near_plane);
    m_corners[2] = glm::vec3(right, top, -near_plane);
    m_corners[3] = glm::vec3(left, top, -near_plane);

    // Get points on far plane
    m_corners[4] = glm::vec3(left_f, bottom_f, -far_plane);
    m_corners[5] = glm::vec3(right_f, bottom_f, -far_plane);
    m_corners[6] = glm::vec3(right_f, top_f, -far_plane);
    m_corners[7] = glm::vec3(left_f, top_f, -far_plane);

    // Transform points to fit camera position and rotation
    m_origin = trans_mat * glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for(uint32_t i = 0; i < 8; ++i)
        m_corners[i] = trans_mat * glm::vec4(m_corners[i], 0.0f);

    // Build planes
    m_planes[0] = Plane(m_origin, m_corners[3], m_corners[0]);       // Left
    m_planes[1] = Plane(m_origin, m_corners[1], m_corners[2]);       // Right
    m_planes[2] = Plane(m_origin, m_corners[0], m_corners[1]);       // Bottom
    m_planes[3] = Plane(m_origin, m_corners[2], m_corners[3]);       // Top
    m_planes[4] = Plane(m_corners[0], m_corners[1], m_corners[2]);   // Near
    m_planes[5] = Plane(m_corners[5], m_corners[4], m_corners[7]);   // Far
}

void Frustum::buildViewFrustum(glm::mat4 const & view_mat, glm::mat4 const & proj_mat)
{
    // This routine works with the OpenGL projection matrix
    // The view matrix is the inverse camera transformation matrix
    // Note: Frustum corners are not updated!
    glm::mat4 m = proj_mat * view_mat;

    m_planes[0] = Plane(-(m[0][3] + m[0][0]), -(m[1][3] + m[1][0]), -(m[2][3] + m[2][0]),
                        -(m[3][3] + m[3][0]));   // Left
    m_planes[1] = Plane(-(m[0][3] - m[0][0]), -(m[1][3] - m[1][0]), -(m[2][3] - m[2][0]),
                        -(m[3][3] - m[3][0]));   // Right
    m_planes[2] = Plane(-(m[0][3] + m[0][1]), -(m[1][3] + m[1][1]), -(m[2][3] + m[2][1]),
                        -(m[3][3] + m[3][1]));   // Bottom
    m_planes[3] = Plane(-(m[0][3] - m[0][1]), -(m[1][3] - m[1][1]), -(m[2][3] - m[2][1]),
                        -(m[3][3] - m[3][1]));   // Top
    m_planes[4] = Plane(-(m[0][3] + m[0][2]), -(m[1][3] + m[1][2]), -(m[2][3] + m[2][2]),
                        -(m[3][3] + m[3][2]));   // Near
    m_planes[5] = Plane(-(m[0][3] - m[0][2]), -(m[1][3] - m[1][2]), -(m[2][3] - m[2][2]),
                        -(m[3][3] - m[3][2]));   // Far

    m_origin = glm::inverse(view_mat) * glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Calculate corners
    glm::mat4 mm     = glm::inverse(m);
    glm::vec4 corner = mm * glm::vec4(-1, -1, -1, 1);
    m_corners[0]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(1, -1, -1, 1);
    m_corners[1]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(1, 1, -1, 1);
    m_corners[2]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(-1, 1, -1, 1);
    m_corners[3]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(-1, -1, 1, 1);
    m_corners[4]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(1, -1, 1, 1);
    m_corners[5]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(1, 1, 1, 1);
    m_corners[6]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
    corner           = mm * glm::vec4(-1, 1, 1, 1);
    m_corners[7]     = glm::vec3(corner.x / corner.w, corner.y / corner.w, corner.z / corner.w);
}

void Frustum::buildBoxFrustum(glm::mat4 const & trans_mat, float left, float right, float bottom, float top,
                              float front, float back)
{
    // Get points on front plane
    m_corners[0] = glm::vec3(left, bottom, front);
    m_corners[1] = glm::vec3(right, bottom, front);
    m_corners[2] = glm::vec3(right, top, front);
    m_corners[3] = glm::vec3(left, top, front);

    // Get points on far plane
    m_corners[4] = glm::vec3(left, bottom, back);
    m_corners[5] = glm::vec3(right, bottom, back);
    m_corners[6] = glm::vec3(right, top, back);
    m_corners[7] = glm::vec3(left, top, back);

    // Transform points to fit camera position and rotation
    m_origin = trans_mat * glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for(uint32_t i = 0; i < 8; ++i)
        m_corners[i] = trans_mat * glm::vec4(m_corners[i], 0.0f);

    // Build planes
    m_planes[0] = Plane(m_corners[0], m_corners[3], m_corners[7]);   // Left
    m_planes[1] = Plane(m_corners[2], m_corners[1], m_corners[6]);   // Right
    m_planes[2] = Plane(m_corners[0], m_corners[4], m_corners[5]);   // Bottom
    m_planes[3] = Plane(m_corners[3], m_corners[2], m_corners[6]);   // Top
    m_planes[4] = Plane(m_corners[0], m_corners[1], m_corners[2]);   // Front
    m_planes[5] = Plane(m_corners[4], m_corners[7], m_corners[6]);   // Back
}

bool Frustum::cullSphere(glm::vec3 pos, float rad) const
{
    // Check the distance of the center to the planes
    for(uint32_t i = 0; i < 6; ++i)
    {
        if(m_planes[i].distToPoint(pos) > rad)
            return true;
    }

    return false;
}

bool Frustum::cullBox(AABB const & b) const
{
    // Idea for optimized AABB testing from www.lighthouse3d.com
    for(uint32_t i = 0; i < 6; ++i)
    {
        glm::vec3 const & n = m_planes[i].m_normal;

        glm::vec3 positive = b.min();
        if(n.x <= 0)
            positive.x = b.max().x;
        if(n.y <= 0)
            positive.y = b.max().y;
        if(n.z <= 0)
            positive.z = b.max().z;

        if(m_planes[i].distToPoint(positive) > 0)
            return true;
    }

    return false;
}

bool Frustum::cullFrustum(Frustum const & frust) const
{
    for(uint32_t i = 0; i < 6; ++i)
    {
        bool all_out = true;

        for(uint32_t j = 0; j < 8; ++j)
        {
            if(m_planes[i].distToPoint(frust.m_corners[j]) < 0)
            {
                all_out = false;
                break;
            }
        }

        if(all_out)
            return true;
    }

    return false;
}

void Frustum::calcAABB(glm::vec3 & mins, glm::vec3 & maxs) const
{
    constexpr float const f_max = std::numeric_limits<float>::max();

    mins.x = f_max;
    mins.y = f_max;
    mins.z = f_max;

    maxs.x = -f_max;
    maxs.y = -f_max;
    maxs.z = -f_max;

    for(uint32_t i = 0; i < 8; ++i)
    {
        if(m_corners[i].x < mins.x)
            mins.x = m_corners[i].x;
        if(m_corners[i].y < mins.y)
            mins.y = m_corners[i].y;
        if(m_corners[i].z < mins.z)
            mins.z = m_corners[i].z;
        if(m_corners[i].x > maxs.x)
            maxs.x = m_corners[i].x;
        if(m_corners[i].y > maxs.y)
            maxs.y = m_corners[i].y;
        if(m_corners[i].z > maxs.z)
            maxs.z = m_corners[i].z;
    }
}
}   // namespace evnt
