#ifndef RENDERER_H
#define RENDERER_H

#include "render_states.h"
#include "../scene/sceneentitybuilder.h"
#include "../scene/AABB.h"

// simple openGL 1.5 renderer

struct RenderModel
{
    struct mesh
    {
        uint32_t m_vertexbuffer  = 0;
        uint32_t m_uvbuffer      = 0;
        uint32_t m_normalbuffer  = 0;
        uint32_t m_elementbuffer = 0;

        int32_t m_indices_size = 0;
    };

    std::vector<mesh> model;
};

class Renderer : public ISystem
{
public:
    enum class MatrixType
    {
        PROJECTION,
        MODELVIEW
    };

    Renderer(Registry & reg) : ISystem(reg) {}

    // ModelSystem must be updated before renderer
    void        update(double time) override;   /// if needed upload new data to GPU
    bool        init() override;
    std::string getName() const override { return "Renderer"; }
    void        terminate() override;

    void setMatrix(MatrixType type, glm::mat4 const & matrix) const;
    void loadIdentityMatrix(MatrixType type) const;

    void uploadMaterialData(Entity entity_id) const;
    void bindMaterial(Entity entity_id) const;
    void unloadMaterialData(Entity entity_id) const;

    void lighting(bool enable = true) const;
    void bindLight(Entity entity_id, uint32_t light_num = 0) const;
    void unbindLight(uint32_t light_num = 0) const;

    void uploadModel(Entity entity_id) const;
    void draw(Entity entity_id) const;
    void unloadModel(Entity entity_id) const;

    // debug draw
    void drawBBox(Entity entity_id) const;

    // Access to the current clearing parameters for the color, depth, and
    // stencil buffers.
    void              setClearColor(glm::vec4 const & clear_color) { m_clear_color = clear_color; }
    glm::vec4 const & getClearColor() const { return m_clear_color; }
    void              setClearDepth(float clear_depth) { m_clear_depth = clear_depth; }
    float             getClearDepth() const { return m_clear_depth; }
    void              setClearStencil(int32_t clear_stencil) { m_clear_stencil = clear_stencil; }
    int32_t           getClearStencil() const { return m_clear_stencil; }

    // Support for clearing the color, depth, and stencil buffers.
    void clearColorBuffer() const;
    void clearDepthBuffer() const;
    void clearStencilBuffer() const;
    void clearBuffers() const;

    void setViewport(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height) const;

    // Override the global state.  If overridden, this state is used instead
    // of the VisualPass state during a drawing call.  To undo the override,
    // pass a null pointer.
    void setAlphaState(AlphaState const & new_state);
    void setCullState(CullState const & new_state);
    void setDepthState(DepthState const & new_state);
    void setOffsetState(OffsetState const & new_state);
    void setStencilState(StencilState const & new_state);
    void setWireState(WireState const & new_state);

private:
    void commitAlphaState() const;
    void commitCullState() const;
    void commitDepthState() const;
    void commitOffsetState() const;
    void commitStencilState() const;
    void commitWireState() const;
    void commitAllStates() const;

    // states
    AlphaState   m_alpha;
    CullState    m_cull;
    DepthState   m_depth;
    OffsetState  m_offset;
    StencilState m_stencil;
    WireState    m_wire;

    glm::vec4 m_clear_color   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float     m_clear_depth   = 1.0f;
    int32_t   m_clear_stencil = 0;

    // bbox vbo
    uint32_t m_bbox_vbo_vertices = 0;
    uint32_t m_bbox_ibo_elements = 0;

    bool m_terminated = false;
};

#endif
