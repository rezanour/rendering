#include "Precomp.h"
#include "VertexFormats.h"

uint32_t GetVertexStride(VertexFormat format)
{
    switch (format)
    {
    case VertexFormat::Position2D:
        return sizeof(Position2DVertex);

    case VertexFormat::PositionNormal:
        return sizeof(PositionNormalVertex);

    case VertexFormat::Basic3D:
        return sizeof(Basic3DVertex);

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
    case VertexFormat::Position2D:
        return FIELD_OFFSET(Position2DVertex, Position);

    case VertexFormat::PositionNormal:
        return FIELD_OFFSET(PositionNormalVertex, Position);

    case VertexFormat::Basic3D:
        return FIELD_OFFSET(Basic3DVertex, Position);

    case VertexFormat::Unknown:
        __fallthrough;

    default:
        assert(false);
        return 0;
    }
}

