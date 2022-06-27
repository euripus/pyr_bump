#ifndef RENDER_STATES_H
#define RENDER_STATES_H

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

#endif