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

void Renderer::update(double time)
{
    for(auto ent : m_reg.view<ModelComponent, Event::Model::UploadBuffer>())
    {
        uploadModel(ent);
    }
    m_reg.reset<Event::Model::UploadBuffer>();

    for(auto ent : m_reg.view<ModelComponent, Event::Model::UploadTexture>())
    {
        uploadMaterialData(ent);
    }
    m_reg.reset<Event::Model::UploadTexture>();

    for(auto ent : m_reg.view<ModelComponent, RenderModel, Event::Model::VertexDataChanged>())
    {
        auto const & geom   = m_reg.get<ModelComponent>(ent);
        auto const & gl_mdl = m_reg.get<RenderModel>(ent);

        for(uint32_t i = 0; i < geom.meshes.size(); ++i)
        {
            auto const & msh = geom.meshes[i];

            glBindBuffer(GL_ARRAY_BUFFER_ARB, gl_mdl.model[i].m_vertexbuffer);
            glBufferSubData(GL_ARRAY_BUFFER_ARB, 0, msh.frame_pos.size() * 3 * sizeof(float),
                            &msh.frame_pos[0]);

            glBindBuffer(GL_ARRAY_BUFFER_ARB, gl_mdl.model[i].m_normalbuffer);
            glBufferSubData(GL_ARRAY_BUFFER_ARB, 0, msh.frame_normal.size() * 3 * sizeof(float),
                            &msh.frame_normal[0]);
        }
    }

    for(auto ent : m_reg.view<ModelComponent, RenderModel, Event::Model::UnloadBuffer>())
    {
        unloadModel(ent);
    }
    m_reg.reset<Event::Model::UnloadBuffer>();

    for(auto ent : m_reg.view<ModelComponent, MaterialComponent, Event::Model::UnloadTexture>())
    {
        unloadMaterialData(ent);
    }
    m_reg.reset<Event::Model::UnloadTexture>();
}

bool Renderer::init()
{
    commitAllStates();
    clearBuffers();

    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    // bbox
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f,  0.5f, -0.5f, -0.5f, 1.0f, 0.5f, 0.5f, -0.5f,
        1.0f,  -0.5f, 0.5f,  -0.5f, 1.0f, -0.5f, -0.5f, 0.5f, 1.0f, 0.5f, -0.5f,
        0.5f,  1.0f,  0.5f,  0.5f,  0.5f, 1.0f,  -0.5f, 0.5f, 0.5f, 1.0f,
    };

    unsigned short elements[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7};

    glGenBuffers(1, &m_bbox_vbo_vertices);
    glGenBuffers(1, &m_bbox_ibo_elements);

    glBindBuffer(GL_ARRAY_BUFFER, m_bbox_vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bbox_ibo_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void Renderer::terminate()
{
    if(!m_terminated)
    {
        glDeleteBuffers(1, &m_bbox_vbo_vertices);
        glDeleteBuffers(1, &m_bbox_ibo_elements);

        m_bbox_vbo_vertices = m_bbox_ibo_elements = 0;

        for(auto ent : m_reg.view<ModelComponent, RenderModel, MaterialComponent>())
        {
            unloadMaterialData(ent);
            unloadModel(ent);
        }

        m_terminated = true;
    }
}

void Renderer::setMatrix(MatrixType type, glm::mat4 const & matrix) const
{
    GLenum const matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadMatrixf(glm::value_ptr(matrix));
}

void Renderer::loadIdentityMatrix(MatrixType type) const
{
    GLenum const matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadIdentity();
}

void Renderer::uploadMaterialData(Entity entity_id) const
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

    glGenTextures(1, &mat.m_bump_tex_id);
    glBindTexture(GL_TEXTURE_2D, mat.m_bump_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, mat.m_bump.type == tex::ImageData::PixelType::pt_rgb ? 3 : 4,
                 static_cast<GLsizei>(mat.m_bump.width), static_cast<GLsizei>(mat.m_bump.height), 0,
                 mat.m_bump.type == tex::ImageData::PixelType::pt_rgb ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE,
                 mat.m_bump.data.get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::bindMaterial(Entity entity_id) const
{
    auto const & mat = m_reg.get<MaterialComponent>(entity_id);

    glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(mat.m_ambient));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(mat.m_diffuse));
    glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(mat.m_specular));
    glMaterialf(GL_FRONT, GL_SHININESS, mat.m_shininess);

    glBindTexture(GL_TEXTURE_2D, mat.m_base_tex_id);
}

void Renderer::unloadMaterialData(Entity entity_id) const
{
    if(!m_reg.has<MaterialComponent>(entity_id))
        return;

    auto & mat = m_reg.get<MaterialComponent>(entity_id);

    glDeleteTextures(1, &mat.m_base_tex_id);
    glDeleteTextures(1, &mat.m_bump_tex_id);

    mat.m_base_tex_id = 0;
    mat.m_bump_tex_id = 0;
}

void Renderer::lighting(bool enable) const
{
    if(enable)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);
}

void Renderer::bindLight(Entity entity_id, uint32_t light_num) const
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
        float angle = glm::degrees(glm::acos(lgh.spot_cos_cutoff));

        glLightf(light_src_num, GL_SPOT_CUTOFF, angle);
        glLightfv(light_src_num, GL_SPOT_DIRECTION, glm::value_ptr(lgh.spot_direction));
        glLightf(light_src_num, GL_SPOT_EXPONENT, lgh.spot_exponent);
    }
    // Enable light source
    glEnable(light_src_num);
}

void Renderer::unbindLight(uint32_t light_num) const
{
    assert(light_num < 8);
    glDisable(GL_LIGHT0 + light_num);
}

void Renderer::uploadModel(Entity entity_id) const
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, msh.indexes.size() * sizeof(uint32_t), msh.indexes.data(),
                     GL_STATIC_DRAW);

        gl_mdl.model.push_back(cur_msh);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_reg.add_component<RenderModel>(entity_id, gl_mdl);
}

void Renderer::draw(Entity entity_id) const
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

void Renderer::unloadModel(Entity entity_id) const
{
    if(!m_reg.has<RenderModel>(entity_id))
        return;

    auto & mdl = m_reg.get<RenderModel>(entity_id);

    for(auto & msh : mdl.model)
    {
        glDeleteBuffers(1, &msh.m_vertexbuffer);
        glDeleteBuffers(1, &msh.m_normalbuffer);
        glDeleteBuffers(1, &msh.m_uvbuffer);
        glDeleteBuffers(1, &msh.m_elementbuffer);

        msh.m_vertexbuffer = msh.m_normalbuffer = msh.m_uvbuffer = msh.m_elementbuffer = msh.m_indices_size =
            0;
    }
}

void Renderer::drawBBox(Entity entity_id) const
{
    auto const & ent_scn = m_reg.get<SceneComponent>(entity_id);
    // auto const & ent_mdl = m_reg.get<ModelComponent>(entity_id);

    if(!ent_scn.transformed_bbox)
        return;

    glm::vec3 size   = ent_scn.transformed_bbox->max() - ent_scn.transformed_bbox->min();
    glm::vec3 center = (ent_scn.transformed_bbox->min() + ent_scn.transformed_bbox->max()) / 2.0f;
    glm::mat4 transform =
        glm::inverse(ent_scn.abs) * glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);

    glDisable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(transform));

    glColor3f(0.0f, 1.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, m_bbox_vbo_vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(4,          // number of elements per vertex, here (x,y,z,w));
                    GL_FLOAT,   // the type of each element
                    0,          // no extra data between each position
                    0           // offset of first element
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bbox_ibo_elements);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 0);
    glLineWidth(3);

    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(4 * sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(8 * sizeof(GLushort)));

    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPopMatrix();

    // model box
    //    size   = ent_mdl.base_bbox.max() - ent_mdl.base_bbox.min();
    //    center = (ent_mdl.base_bbox.min() + ent_mdl.base_bbox.max()) / 2.0f;
    //    transform =
    //        glm::inverse(ent_scn.abs) * glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1),
    //        size);

    //    glPushMatrix();
    //    glMultMatrixf(glm::value_ptr(transform));

    //    glColor3f(1.0f, 0.0f, 0.0f);

    //    glBindBuffer(GL_ARRAY_BUFFER, m_bbox_vbo_vertices);
    //    glEnableClientState(GL_VERTEX_ARRAY);
    //    glVertexPointer(4,          // number of elements per vertex, here (x,y,z,w));
    //                    GL_FLOAT,   // the type of each element
    //                    0,          // no extra data between each position
    //                    0           // offset of first element
    //    );
    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bbox_ibo_elements);

    //    glEnable(GL_POLYGON_OFFSET_FILL);
    //    glPolygonOffset(1, 0);
    //    glLineWidth(3);

    //    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    //    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(4 *
    //    sizeof(GLushort))); glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(8 *
    //    sizeof(GLushort)));

    //    glDisable(GL_POLYGON_OFFSET_FILL);

    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //    glDisableClientState(GL_VERTEX_ARRAY);
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //    glPopMatrix();

    glLineWidth(1);
    glEnable(GL_LIGHTING);
}

void Renderer::clearColorBuffer() const
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearDepthBuffer() const
{
    glClearDepth(m_clear_depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearStencilBuffer() const
{
    glClearStencil(m_clear_stencil);
    glClear(GL_STENCIL_BUFFER_BIT);
}

void Renderer::clearBuffers() const
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClearDepth(m_clear_depth);
    glClearStencil(m_clear_stencil);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::setViewport(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height) const
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

void Renderer::commitAlphaState() const
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

void Renderer::commitCullState() const
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

void Renderer::commitDepthState() const
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

void Renderer::commitOffsetState() const
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

void Renderer::commitStencilState() const
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

void Renderer::commitWireState() const
{
    if(m_wire.enabled)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::commitAllStates() const
{
    commitAlphaState();
    commitCullState();
    commitDepthState();
    commitOffsetState();
    commitStencilState();
    commitWireState();
}
