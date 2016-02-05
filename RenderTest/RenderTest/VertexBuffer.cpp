#include "Precomp.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"

VertexBuffer::VertexBuffer()
    : Format(VertexFormat::Unknown)
    , Stride(0)
    , BaseVertex(0)
    , VertexCount(0)
    , BoundsOffset(0.f, 0.f, 0.f)
    , BoundsRadius(0.f)
{
}

VertexBuffer::~VertexBuffer()
{
}

HRESULT VertexBuffer::Initialize(const ComPtr<ID3D11Device>& device, VertexFormat format, const void* data, uint32_t dataBytes)
{
    VB = nullptr;

    D3D11_BUFFER_DESC desc{};
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = dataBytes;
    desc.StructureByteStride = GetVertexStride(format);
    desc.Usage = D3D11_USAGE_DEFAULT;

    assert(desc.ByteWidth % desc.StructureByteStride == 0);

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = data;
    init.SysMemPitch = desc.ByteWidth;

    HRESULT hr = device->CreateBuffer(&desc, &init, &VB);
    CHECKHR(hr);

    Format = format;
    Stride = desc.StructureByteStride;
    BaseVertex = 0;
    VertexCount = desc.ByteWidth / desc.StructureByteStride;

    // Find extrema
    XMFLOAT3 Min(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    p += GetVertexPositionOffset(format);

    for (uint32_t i = 0; i < VertexCount; ++i, p += Stride)
    {
        const XMFLOAT3* pos = reinterpret_cast<const XMFLOAT3*>(p);
        if (pos->x < Min.x) Min.x = pos->x;
        if (pos->y < Min.y) Min.y = pos->y;
        if (pos->z < Min.z) Min.z = pos->z;
        if (pos->x > Max.x) Max.x = pos->x;
        if (pos->y > Max.y) Max.y = pos->y;
        if (pos->z > Max.z) Max.z = pos->z;
    }

    // find center of bounds
    BoundsOffset.x = 0.5f * (Min.x + Max.x);
    BoundsOffset.y = 0.5f * (Min.y + Max.y);
    BoundsOffset.z = 0.5f * (Min.z + Max.z);

    // radius of fitted sphere is distance from offset to an extremum (min & max are equidistant from offset)
    XMFLOAT3 diff(Max.x - BoundsOffset.x, Max.y - BoundsOffset.y, Max.z - BoundsOffset.z);
    BoundsRadius = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    return hr;
}
