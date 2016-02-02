#include "Precomp.h"
#include "VertexFormats.h"

uint32_t GetVertexStride(VertexFormat format)
{
    switch (format)
    {
    case VertexFormat::PositionNormal:
        return sizeof(PositionNormalVertex);

    case VertexFormat::Unknown:
        __fallthrough;

    default:
        assert(false);
        return 0;
    }
}

uint32_t GetVertexPositionOffset(VertexFormat format)
{
    switch (format)
    {
    case VertexFormat::PositionNormal:
        return FIELD_OFFSET(PositionNormalVertex, Position);

    case VertexFormat::Unknown:
        __fallthrough;

    default:
        assert(false);
        return 0;
    }
}

