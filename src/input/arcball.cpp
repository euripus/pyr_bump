#include "arcball.h"
#include "glm/ext/matrix_transform.hpp"
#include "src/scene/scenecmp.h"

// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball

glm::vec3 Arcball::getArcballVector(glm::ivec2 pos) const
{
    glm::vec3 vec =
        glm::vec3(static_cast<float>(pos.x) / static_cast<float>(m_window_size.x) * 2.0f - 1.0f,
                  static_cast<float>(pos.y) / static_cast<float>(m_window_size.y) * 2.0f - 1.0f, 0.0f);

    vec.y             = -vec.y;
    float vec_squared = vec.x * vec.x + vec.y * vec.y;
    if(vec_squared <= 1.0f)
        vec.z = glm::sqrt(1.0f - vec_squared);   // Pythagoras
    else
        glm::normalize(vec);

    return vec;
}

glm::mat4 Arcball::getTransformMatrix(glm::ivec2 old_pos, glm::ivec2 new_pos) const
{
    if(old_pos == new_pos)
        return glm::mat4(1.0f);

    glm::vec3 vo = getArcballVector(old_pos);
    glm::vec3 vn = getArcballVector(new_pos);

    float     angle = glm::acos(glm::min(1.0f, glm::dot(vo, vn)));
    glm::vec3 axis  = glm::cross(vo, vn);
    return glm::rotate(glm::mat4(1.0f), angle, axis);
}

void Arcball::update(Input const * input_ptr, Entity camera, Registry & reg)
{
    if(input_ptr->getMouseButton(Buttons::Button_0))
    {
        if(m_right_pressed)
        {
            glm::ivec2 new_corsor_pos = input_ptr->getMousePosition();
            if(m_old_cursor_pos != new_corsor_pos)
            {
                auto mat         = getTransformMatrix(m_old_cursor_pos, new_corsor_pos);
                m_old_cursor_pos = new_corsor_pos;

                Event::Scene::TransformComponent tr_cmp;
                tr_cmp.replase_local_matrix = false;
                tr_cmp.new_mat              = mat;

                reg.add_component<Event::Scene::TransformComponent>(camera, tr_cmp);
            }
        }
        else
        {
            m_right_pressed  = true;
            m_old_cursor_pos = input_ptr->getMousePosition();
        }
    }
    else
    {
        m_right_pressed = false;
    }
}
