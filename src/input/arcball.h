#ifndef ARCBALL_H
#define ARCBALL_H

#include <glm/glm.hpp>
#include "input.h"
#include "src/scene/sceneentitybuilder.h"

class Arcball
{
    glm::ivec2 m_window_size;
    bool       m_right_pressed{false};
    glm::ivec2 m_old_cursor_pos{0};

public:
    Arcball(uint32_t x_size, uint32_t y_size) : m_window_size{x_size, y_size} {}

    void setNewWindowSize(uint32_t x_size, uint32_t y_size) noexcept
    {
        m_window_size.x = static_cast<int32_t>(x_size);
        m_window_size.y = static_cast<int32_t>(y_size);
    }

    glm::vec3 getArcballVector(glm::ivec2 pos) const;
    glm::mat4 getTransformMatrix(glm::ivec2 old_pos, glm::ivec2 new_pos) const;

    void update(Input const * input_ptr, Entity camera, Registry & reg);
};

#endif   // ARCBALL_H
