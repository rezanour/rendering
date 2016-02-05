#pragma once

enum class VertexFormat
{
    Unknown = 0,
    Position2D,
    PositionNormal,
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

uint32_t GetVertexStride(VertexFormat format);
uint32_t GetVertexPositionOffset(VertexFormat format);
