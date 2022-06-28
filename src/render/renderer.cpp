#include "renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

// Mappings
GLenum g_gl_compare_mode[static_cast<uint32_t>(CompareMode::QUANTITY)] = {
    GL_NEVER,      // NEVER
    GL_LESS,       // LESS
    GL_EQUAL,      // EQUAL
    GL_LEQUAL,     // LEQUAL
    GL_GREATER,    // GREATER
    GL_NOTEQUAL,   // NOTEQUAL
    GL_GEQUAL,     // GEQUAL
    GL_ALWAYS      // ALWAYS
};

GLenum g_gl_alpha_src_blend[static_cast<uint32_t>(AlphaState::SrcBlendMode::QUANTITY)] = {
    GL_ZERO,                       // ZERO
    GL_ONE,                        // ONE
    GL_DST_COLOR,                  // DST_COLOR
    GL_ONE_MINUS_DST_COLOR,        // ONE_MINUS_DST_COLOR
    GL_SRC_ALPHA,                  // SRC_ALPHA
    GL_ONE_MINUS_SRC_ALPHA,        // ONE_MINUS_SRC_ALPHA
    GL_DST_ALPHA,                  // DST_ALPHA
    GL_ONE_MINUS_DST_ALPHA,        // ONE_MINUS_DST_ALPHA
    GL_SRC_ALPHA_SATURATE,         // SRC_ALPHA_SATURATE
    GL_CONSTANT_COLOR,             // CONSTANT_COLOR
    GL_ONE_MINUS_CONSTANT_COLOR,   // ONE_MINUS_CONSTANT_COLOR
    GL_CONSTANT_ALPHA,             // CONSTANT_ALPHA
    GL_ONE_MINUS_CONSTANT_ALPHA    // ONE_MINUS_CONSTANT_ALPHA
};

GLenum g_gl_alpha_dst_blend[static_cast<uint32_t>(AlphaState::DstBlendMode::QUANTITY)] = {
    GL_ZERO,                       // ZERO
    GL_ONE,                        // ONE
    GL_SRC_COLOR,                  // SRC_COLOR
    GL_ONE_MINUS_SRC_COLOR,        // ONE_MINUS_SRC_COLOR
    GL_SRC_ALPHA,                  // SRC_ALPHA
    GL_ONE_MINUS_SRC_ALPHA,        // ONE_MINUS_SRC_ALPHA
    GL_DST_ALPHA,                  // DST_ALPHA
    GL_ONE_MINUS_DST_ALPHA,        // ONE_MINUS_DST_ALPHA
    GL_CONSTANT_COLOR,             // CONSTANT_COLOR
    GL_ONE_MINUS_CONSTANT_COLOR,   // ONE_MINUS_CONSTANT_COLOR
    GL_CONSTANT_ALPHA,             // CONSTANT_ALPHA
    GL_ONE_MINUS_CONSTANT_ALPHA    // ONE_MINUS_CONSTANT_ALPHA
};

GLenum g_gl_stencil_operation[static_cast<uint32_t>(StencilState::OperationType::QUANTITY)] = {
    GL_KEEP,      // KEEP
    GL_ZERO,      // ZERO
    GL_REPLACE,   // REPLACE
    GL_INCR,      // INCREMENT
    GL_DECR,      // DECREMENT
    GL_INVERT     // INVERT
};

bool Renderer::init()
{
    commitAllStates();
    clearBuffers();

    return true;
}

void Renderer::setMatrix(MatrixType type, glm::mat4 const & matrix)
{
    GLenum matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadMatrixf(glm::value_ptr(matrix));
}

void Renderer::loadIdentityMatrix(MatrixType type)
{
    GLenum matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadIdentity();
}

void Renderer::clearColorBuffer()
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearDepthBuffer()
{
    glClearDepth(m_clear_depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearStencilBuffer()
{
    glClearStencil(m_clear_stencil);
    glClear(GL_STENCIL_BUFFER_BIT);
}

void Renderer::clearBuffers()
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClearDepth(m_clear_depth);
    glClearStencil(m_clear_stencil);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::setViewport(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height)
{
    glViewport(x_pos, y_pos, width, height);
}

void Renderer::setAlphaState(AlphaState const & new_state)
{
    if(m_alpha == new_state)
        return;

    m_alpha = new_state;
    commitAlphaState();
}

void Renderer::setCullState(CullState const & new_state)
{
    if(m_cull == new_state)
        return;

    m_cull = new_state;
    commitCullState();
}

void Renderer::setDepthState(DepthState const & new_state)
{
    if(m_depth == new_state)
        return;

    m_depth = new_state;
    commitDepthState();
}

void Renderer::setOffsetState(OffsetState const & new_state)
{
    if(m_offset == new_state)
        return;

    m_offset = new_state;
    commitOffsetState();
}

void Renderer::setStencilState(StencilState const & new_state)
{
    if(m_stencil == new_state)
        return;

    m_stencil = new_state;
    commitStencilState();
}

void Renderer::setWireState(WireState const & new_state)
{
    if(m_wire == new_state)
        return;

    m_wire = new_state;
    commitWireState();
}

void Renderer::commitAlphaState()
{
    if(m_alpha.blend_enabled)
    {
        GLenum src_blend = g_gl_alpha_src_blend[static_cast<uint32_t>(m_alpha.src_blend)];
        GLenum dst_blend = g_gl_alpha_dst_blend[static_cast<uint32_t>(m_alpha.dst_blend)];

        glEnable(GL_BLEND);
        glBlendFunc(src_blend, dst_blend);
        glBlendColor(m_alpha.constant_color[0], m_alpha.constant_color[1], m_alpha.constant_color[2],
                     m_alpha.constant_color[3]);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if(m_alpha.compare_enabled)
    {
        GLenum compare = g_gl_compare_mode[static_cast<uint32_t>(m_alpha.compare)];

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(compare, m_alpha.reference);
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
    }
}

void Renderer::commitCullState()
{
    if(m_cull.enabled)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        bool order = m_cull.ccw_order;
        if(order)
            glCullFace(GL_BACK);
        else
            glCullFace(GL_FRONT);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void Renderer::commitDepthState()
{
    if(m_depth.enabled)
    {
        GLenum compare = g_gl_compare_mode[static_cast<uint32_t>(m_depth.compare)];

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(compare);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    if(m_depth.writable)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void Renderer::commitOffsetState()
{
    if(m_offset.fill_enabled)
        glEnable(GL_POLYGON_OFFSET_FILL);
    else
        glDisable(GL_POLYGON_OFFSET_FILL);

    if(m_offset.line_enabled)
        glEnable(GL_POLYGON_OFFSET_LINE);
    else
        glDisable(GL_POLYGON_OFFSET_LINE);

    if(m_offset.point_enabled)
        glEnable(GL_POLYGON_OFFSET_POINT);
    else
        glDisable(GL_POLYGON_OFFSET_POINT);

    glPolygonOffset(m_offset.scale, m_offset.bias);
}

void Renderer::commitStencilState()
{
    if(m_stencil.enabled)
    {
        glEnable(GL_STENCIL_TEST);

        GLenum compare   = g_gl_compare_mode[static_cast<uint32_t>(m_stencil.compare)];
        GLenum on_fail   = g_gl_stencil_operation[static_cast<uint32_t>(m_stencil.on_fail)];
        GLenum on_z_fail = g_gl_stencil_operation[static_cast<uint32_t>(m_stencil.on_z_fail)];
        GLenum on_z_pass = g_gl_stencil_operation[static_cast<uint32_t>(m_stencil.on_z_pass)];

        glStencilFunc(compare, m_stencil.reference, m_stencil.mask);
        glStencilMask(m_stencil.write_mask);
        glStencilOp(on_fail, on_z_fail, on_z_pass);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
}

void Renderer::commitWireState()
{
    if(m_wire.enabled)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::commitAllStates()
{
    commitAlphaState();
    commitCullState();
    commitDepthState();
    commitOffsetState();
    commitStencilState();
    commitWireState();
}
