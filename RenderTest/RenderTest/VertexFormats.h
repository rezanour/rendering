#pragma once

enum class VertexFormat
{
    Unknown = 0,
    Position2D,
    PositionNormal,
    Basic3D,
};

struct Position2DVertex
{
    XMFLOAT2 Position;
};

struct PositionNormalVertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
};

struct Basic3DVertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT3 Tangent;
    XMFLOAT3 BiTangent;
    XMFLOAT2 TexCoord;
};

uint32_t GetVertexStride(VertexFormat format);
uint32_t GetVertexPositionOffset(VertexFormat format);
