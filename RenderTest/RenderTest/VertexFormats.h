#pragma once

enum class VertexFormat
{
    Unknown = 0,
    PositionNormal,
};

struct PositionNormalVertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
};

uint32_t GetVertexStride(VertexFormat format);
uint32_t GetVertexPositionOffset(VertexFormat format);
