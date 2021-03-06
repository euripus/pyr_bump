#ifndef AABB_H
#define AABB_H

#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <vector>

namespace evnt
{
//! Axis-aligned minimum bounding box class
/*!
    This class holds AABB for a given point set,
    is its minimum bounding box subject to the constraint
    that the edges of the box are parallel to the (Cartesian) coordinate axes..
*/
class AABB
{
    glm::vec3 m_min; /*!< The corner with the smallest values for each coordinate of the AABB */
    glm::vec3 m_max; /*!< The corner with the largest values for each coordinate of the AABB */
public:
    //! Construct to invalid values to represent an unset bounding box
    inline AABB() :
        m_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max()),
        m_max(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
              std::numeric_limits<float>::min())
    {}

    //! Construct to with specified min and max values
    inline AABB(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) :
        m_min(xmin, ymin, zmin), m_max(xmax, ymax, zmax)
    {}

    //! Construct to with specified min and max values
    inline AABB(glm::vec3 min, glm::vec3 max) : m_min(min), m_max(max) {}

    inline AABB(AABB const & bb) : m_min(bb.m_min), m_max(bb.m_max) {}

    inline AABB(AABB && bb) : m_min(bb.m_min), m_max(bb.m_max) {}

    inline AABB & operator=(AABB const & bb)
    {
        if(this != &bb)
        {
            m_min = bb.m_min;
            m_max = bb.m_max;
        }

        return *this;
    }

    inline AABB & operator=(AABB && bb)
    {
        if(this != &bb)
        {
            std::swap(m_min, bb.m_min);
            std::swap(m_max, bb.m_max);
        }

        return *this;
    }

    ~AABB() {}

    inline bool operator==(AABB const & rhs) const { return m_min == rhs.m_min && m_max == rhs.m_max; }

    inline bool operator!=(AABB const & rhs) const { return m_min != rhs.m_min || m_max != rhs.m_max; }

    inline glm::vec3 min() const { return m_min; }

    inline glm::vec3 max() const { return m_max; }

    /*! Expands the bounding box to include the given coordinate.
        If this box is uninitialized, set its min and max extents to v.
        \param[in] v given coordinate
    */
    inline void expandBy(glm::vec3 const & v)
    {
        if(v.x < m_min.x)
            m_min.x = v.x;
        if(v.x > m_max.x)
            m_max.x = v.x;

        if(v.y < m_min.y)
            m_min.y = v.y;
        if(v.y > m_max.y)
            m_max.y = v.y;

        if(v.z < m_min.z)
            m_min.z = v.z;
        if(v.z > m_max.z)
            m_max.z = v.z;
    }

    /*! Expands this bounding box to include the given bounding box.
        If this box is uninitialized, set it equal to bb.
        \param[in] bb given bounding box
    */
    void expandBy(AABB const & bb)
    {
        if(bb.m_min.x < m_min.x)
            m_min.x = bb.m_min.x;
        if(bb.m_max.x > m_max.x)
            m_max.x = bb.m_max.x;

        if(bb.m_min.y < m_min.y)
            m_min.y = bb.m_min.y;
        if(bb.m_max.y > m_max.y)
            m_max.y = bb.m_max.y;

        if(bb.m_min.z < m_min.z)
            m_min.z = bb.m_min.z;
        if(bb.m_max.z > m_max.z)
            m_max.z = bb.m_max.z;
    }

    /*! Calculate intersection of bounding boxes
        \param[in] bb specified bounding box
        \return AABB the intersection of this bounding box and the specified bounding box.
    */
    inline AABB intersect(AABB const & bb) const
    {
        return AABB(std::max(m_min.x, bb.m_min.x), std::max(m_min.y, bb.m_min.y),
                    std::max(m_min.z, bb.m_min.z), std::min(m_max.x, bb.m_max.x),
                    std::min(m_max.y, bb.m_max.y), std::min(m_max.z, bb.m_max.z));
    }

    /*! Check for intersection of bounding boxes
        \param[in] bb AABB that will be checked
        \return True if this bounding box intersects the specified bounding box.
    */
    inline bool intersects(AABB const & bb) const
    {
        return std::max(bb.m_min.x, m_min.x) <= std::min(bb.m_max.x, m_max.x)
               && std::max(bb.m_min.y, m_min.y) <= std::min(bb.m_max.y, m_max.y)
               && std::max(bb.m_min.z, m_min.z) <= std::min(bb.m_max.z, m_max.z);
    }

    /*! Check for the containing of the coordinate in the AABB
        \param[in] v coordinate that will be checked
        \return True if this AABB contains the specified coordinate.
    */
    inline bool contains(glm::vec3 const & v) const
    {
        return (v.x >= m_min.x && v.x <= m_max.x) && (v.y >= m_min.y && v.y <= m_max.y)
               && (v.z >= m_min.z && v.z <= m_max.z);
    }

    //! Transform this bounding box
    /*! Transform a given axis aligned bounding box by some matrix and then
        convert it into an axis aligned bounding box in the resulting
        coordinate space again
        \param[in] matrix transformation matrix
    */
    inline void transform(glm::mat4 const & matrix)
    {
        // http://dev.theomader.com/transform-bounding-boxes/
        glm::vec3 xa = glm::vec3(glm::column(matrix, 0)) * m_min.x;
        glm::vec3 xb = glm::vec3(glm::column(matrix, 0)) * m_max.x;

        glm::vec3 ya = glm::vec3(glm::column(matrix, 1)) * m_min.y;
        glm::vec3 yb = glm::vec3(glm::column(matrix, 1)) * m_max.y;

        glm::vec3 za = glm::vec3(glm::column(matrix, 2)) * m_min.z;
        glm::vec3 zb = glm::vec3(glm::column(matrix, 2)) * m_max.z;

        m_min = glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + glm::vec3(glm::column(matrix, 3));
        m_max = glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + glm::vec3(glm::column(matrix, 3));
    }

    /*! Build the bounding box to include the given coordinates.
        \param[in] positions point set for building AABB
    */
    void buildBoundBox(std::vector<glm::vec3> const & positions)
    {
        m_min = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max());
        m_max = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                          std::numeric_limits<float>::min());

        for(unsigned int i = 0; i < positions.size(); i++)
        {
            glm::vec3 const & pos = positions[i];

            expandBy(pos);
        }
    }
};
}   // namespace evnt

#endif   // AABB_H
