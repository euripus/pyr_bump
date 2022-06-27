#ifndef RENDERER_H
#define RENDERER_H

#include "render_states.h"

struct RenderModel
{
    uint32_t m_vertexbuffer  = 0;
    uint32_t m_uvbuffer      = 0;
    uint32_t m_normalbuffer  = 0;
    uint32_t m_elementbuffer = 0;

    int32_t m_indices_size = 0;
};

// tag structure
struct BuffersUpdated
{
    bool val = true;
};

class Renderer : public ISystem
{
public:
    enum class MatrixType
    {
        PERSPECTIVE,
        MODELVIEW
    }

    Renderer(Registry & reg);

    // ModelSystem must be updated before renderer
    void update(Registry & reg, float time_delta) override;   /// if needed upload new data to GPU

    void setMatrix(MatrixType type, glm::mat4 const & matrix);
    void loadIdentityMatrix(MatrixType type);

    void createMaterial(MaterialComponent & mat_cmp, entity_id);
    void bindMaterial(entity_id);
    void unbindMaterial(entity_id);

    void createModel(ModelComponent & mdl, entity_id);
    void bindModel(entity_id);
    void unbindModel(entity_id);

    void lighting(bool disable = false);
    void setLight(LightComponent & lgh);

    // Access to the current clearing parameters for the color, depth, and
    // stencil buffers.  The color buffer is the back buffer.
    void              setClearColor(glm::vec4 const & clear_color);
    glm::vec4 const & getClearColor() const;
    void              setClearDepth(float clear_depth);
    float             getClearDepth() const;

    // Override the global state.  If overridden, this state is used instead
    // of the VisualPass state during a drawing call.  To undo the override,
    // pass a null pointer.
    void setAlphaState(AlphaState const & new_state);
    void setCullState(CullState const & new_state);
    void setDepthState(DepthState const & new_state);
    void setOffsetState(OffsetState const & new_state);
    void setStencilState(StencilState const & new_state);
    void setWireState(WireState const & new_state);
};


#endif
