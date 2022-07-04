#include "renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "../scene/material.h"
#include "../scene/light.h"
#include "../scene/model.h"

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

    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    return true;
}

void Renderer::setMatrix(MatrixType type, glm::mat4 const & matrix)
{
    GLenum const matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadMatrixf(glm::value_ptr(matrix));
}

void Renderer::loadIdentityMatrix(MatrixType type)
{
    GLenum const matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadIdentity();
}

void Renderer::uploadMaterialData(Entity entity_id)
{
    auto & mat = m_reg.get<MaterialComponent>(entity_id);

    glGenTextures(1, &mat.m_base_tex_id);
    glBindTexture(GL_TEXTURE_2D, mat.m_base_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, mat.m_diff.type == tex::ImageData::PixelType::pt_rgb ? 3 : 4,
                 static_cast<GLsizei>(mat.m_diff.width), static_cast<GLsizei>(mat.m_diff.height), 0,
                 mat.m_diff.type == tex::ImageData::PixelType::pt_rgb ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE,
                 mat.m_diff.data.get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::bindMaterial(Entity entity_id)
{
    auto const & mat = m_reg.get<MaterialComponent>(entity_id);

    glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(mat.m_ambient));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(mat.m_diffuse));
    glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(mat.m_specular));
    glMaterialf(GL_FRONT, GL_SHININESS, mat.m_shininess);

    glBindTexture(GL_TEXTURE_2D, mat.m_base_tex_id);
}

void Renderer::unloadMaterialData(Entity entity_id)
{
    auto const & mat = m_reg.get<MaterialComponent>(entity_id);

    glDeleteTextures(1, &mat.m_base_tex_id);
}

void Renderer::lighting(bool enable)
{
    if(enable)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);
}

void Renderer::bindLight(Entity entity_id, uint32_t light_num)
{
    assert(light_num < 8);

    auto const & lgh           = m_reg.get<LightComponent>(entity_id);
    GLenum const light_src_num = GL_LIGHT0 + light_num;

    glLightfv(light_src_num, GL_POSITION, glm::value_ptr(lgh.position));
    glLightfv(light_src_num, GL_AMBIENT, glm::value_ptr(lgh.ambient));
    glLightfv(light_src_num, GL_DIFFUSE, glm::value_ptr(lgh.diffuse));
    glLightfv(light_src_num, GL_SPECULAR, glm::value_ptr(lgh.specular));

    if(lgh.type == LightType::Point || lgh.type == LightType::Spot)
    {
        glLightf(light_src_num, GL_CONSTANT_ATTENUATION, lgh.attenuation.x);
        glLightf(light_src_num, GL_LINEAR_ATTENUATION, lgh.attenuation.y);
        glLightf(light_src_num, GL_QUADRATIC_ATTENUATION, lgh.attenuation.z);
    }
    if(lgh.type == LightType::Spot)
    {
        float angle = glm::degrees(glm::acos(lgh.spotCosCutoff));

        glLightf(light_src_num, GL_SPOT_CUTOFF, angle);
        glLightfv(light_src_num, GL_SPOT_DIRECTION, glm::value_ptr(lgh.spotDirection));
        glLightf(light_src_num, GL_SPOT_EXPONENT, lgh.spotExponent);
    }
    // Enable light source
    glEnable(light_src_num);
}

void Renderer::unbindLight(uint32_t light_num)
{
    assert(light_num < 8);
    glDisable(GL_LIGHT0 + light_num);
}

void Renderer::uploadModel(Entity entity_id)
{
    auto const & mdl = m_reg.get<ModelComponent>(entity_id);
    RenderModel  gl_mdl;

    // Generate buffers
    for(auto const & msh : mdl.meshes)
    {
        RenderModel::mesh cur_msh;

        glGenBuffers(1, &cur_msh.m_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, cur_msh.m_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, msh.pos.size() * 3 * sizeof(float), msh.pos.data(), GL_DYNAMIC_DRAW);

        glGenBuffers(1, &cur_msh.m_normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, cur_msh.m_normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, msh.normal.size() * 3 * sizeof(float), msh.normal.data(),
                     GL_DYNAMIC_DRAW);

        glGenBuffers(1, &cur_msh.m_uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, cur_msh.m_uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, msh.tex_coords.size() * 2 * sizeof(float), msh.tex_coords.data(),
                     GL_STATIC_DRAW);

        // Generate a buffer for the indices as well
        glGenBuffers(1, &cur_msh.m_elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cur_msh.m_elementbuffer);
        cur_msh.m_indices_size = static_cast<GLsizei>(msh.indexes.size());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(cur_msh.m_indices_size) * sizeof(uint32_t),
                     msh.indexes.data(), GL_STATIC_DRAW);

        gl_mdl.model.push_back(cur_msh);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_reg.add_component<RenderModel>(entity_id, gl_mdl);
}

void Renderer::draw(Entity entity_id)
{
    auto const & mdl = m_reg.get<RenderModel>(entity_id);

    for(auto const & msh : mdl.model)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER, msh.m_normalbuffer);
        glNormalPointer(GL_FLOAT, 0, static_cast<void *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, msh.m_uvbuffer);
        glTexCoordPointer(2, GL_FLOAT, 0, static_cast<void *>(nullptr));
        glBindBuffer(GL_ARRAY_BUFFER, msh.m_vertexbuffer);
        glVertexPointer(3, GL_FLOAT, 0, static_cast<void *>(nullptr));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, msh.m_elementbuffer);

        glDrawElements(GL_TRIANGLES, msh.m_indices_size, GL_UNSIGNED_INT, static_cast<void *>(nullptr));

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Renderer::unloadModel(Entity entity_id)
{
    if(!m_reg.has<RenderModel>(entity_id))
        return;

    auto const & mdl = m_reg.get<RenderModel>(entity_id);

    for(auto const & msh : mdl.model)
    {
        glDeleteBuffers(1, &msh.m_vertexbuffer);
        glDeleteBuffers(1, &msh.m_normalbuffer);
        glDeleteBuffers(1, &msh.m_uvbuffer);
        glDeleteBuffers(1, &msh.m_elementbuffer);
    }
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
