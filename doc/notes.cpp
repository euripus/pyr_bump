

struct AlphaState
{
    enum class SrcBlendMode
    {
        ZERO,
        ONE,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        SRC_ALPHA_SATURATE,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        QUANTITY
    };

    enum class DstBlendMode
    {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        QUANTITY
    };

    enum class CompareMode
    {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS,
        QUANTITY
    };

    bool         blend_enabled   = false;
    SrcBlendMode src_blend       = SrcBlendMode::SRC_ALPHA;
    DstBlendMode dst_blend       = DstBlendMode::ONE_MINUS_SRC_ALPHA;
    bool         compare_enabled = false;
    CompareMode  compare         = CompareMode::ALWAYS;
    float        reference       = 0.0f;   // always in [0,1]
    glm::vec4    constant_color  = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    bool operator==(AlphaState const & other)
    {
        return (blend_enabled == other.blend_enabled) && (src_blend == other.src_blend)
               && (dst_blend == other.dst_blend) && (compare_enabled == other.compare_enabled)
               && (compare == other.compare) && (reference == other.reference) &&   // TODO: epsilon compare
               (constant_color == other.constant_color);
    }
};

struct CullState
{
    bool enabled   = true;
    bool ccw_order = true;

    bool operator==(CullState const & other)
    {
        return (enabled == other.enabled) && (ccw_order == other.ccw_order);
    }
};

struct DepthState
{
    enum class CompareMode
    {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS,
        QUANTITY
    };

    bool        enabled  = true;
    bool        writable = true;
    CompareMode compare  = CompareMode::LEQUAL;

    bool operator==(DepthState const & other)
    {
        return (enabled == other.enabled) && (writable == other.writable) && (compare == other.compare);
    }
};

struct OffsetState
{
    // Set whether offset should be enabled for the various polygon drawing
    // modes (fill, line, point).
    bool fill_enabled  = false;
    bool line_enabled  = false;
    bool point_enabled = false;

    // The offset is Scale*dZ + Bias*r where dZ is the change in depth
    // relative to the screen space area of the poly, and r is the smallest
    // resolvable depth difference.  Negative values move polygons closer to
    // the eye.
    float scale = 0.0f;
    float bias  = 0.0f;

    bool operator==(OffsetState const & other)
    {
        return (fill_enabled == other.fill_enabled) && (line_enabled == other.line_enabled)
               && (point_enabled == other.point_enabled) && (scale == other.scale)
               &&   // TODO: epsilon compare
               (bias == other.bias);
    }
};

struct StencilState
{
    enum class CompareMode
    {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS,
        QUANTITY
    };

    enum class OperationType
    {
        KEEP,
        ZERO,
        REPLACE,
        INCREMENT,
        DECREMENT,
        INVERT,
        QUANTITY
    };

    bool          enabled    = false;
    CompareMode   compare    = CompareMode::NEVER;
    uint32_t      reference  = 0;
    uint32_t      mask       = max_uint;
    uint32_t      write_mask = max_uint;
    OperationType on_fail    = OperationType::KEEP;
    OperationType on_z_fail  = OperationType::KEEP;
    OperationType on_z_pass  = OperationType::KEEP;

    bool operator==(StencilState const & other)
    {
        return (enabled == other.enabled) && (compare == other.compare) && (reference == other.reference)
               && (mask == other.mask) && (write_mask == other.write_mask) && (on_fail == other.on_fail)
               && (on_z_fail == other.on_z_fail) && (on_z_pass == other.on_z_pass);
    }
};

struct WireState
{
    bool enabled = false;

    bool operator==(WireState const & other) { return (enabled == other.enabled); }
};

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
