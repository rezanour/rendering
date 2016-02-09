#include "Precomp.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"

VertexBuffer::VertexBuffer()
    : Format(VertexFormat::Unknown)
    , Stride(0)
    , BaseVertex(0)
    , VertexCount(0)
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

    return hr;
}
