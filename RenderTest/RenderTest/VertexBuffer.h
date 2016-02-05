#pragma once

enum class VertexFormat;

class VertexBuffer : NonCopyable
{
public:
    VertexBuffer();
    virtual ~VertexBuffer();

    HRESULT Initialize(const ComPtr<ID3D11Device>& device, VertexFormat format, const void* data, uint32_t dataBytes);

    VertexFormat GetFormat() const
    {
        return Format;
    }

    uint32_t GetStride() const
    {
        return Stride;
    }

    uint32_t GetBaseVertex() const
    {
        return BaseVertex;
    }

    uint32_t GetVertexCount() const
    {
        return VertexCount;
    }

    const ComPtr<ID3D11Buffer>& GetVB() const
    {
        return VB;
    }

    const XMFLOAT3& GetBoundsOffset() const
    {
        return BoundsOffset;
    }

    float GetBoundsRadius() const
    {
        return BoundsRadius;
    }

private:
    ComPtr<ID3D11Buffer> VB;
    VertexFormat Format;
    uint32_t Stride;
    uint32_t BaseVertex;
    uint32_t VertexCount;

    // Bounding Sphere
    XMFLOAT3 BoundsOffset;  // vertices aren't necessarily centered around origin
    float BoundsRadius;
};
