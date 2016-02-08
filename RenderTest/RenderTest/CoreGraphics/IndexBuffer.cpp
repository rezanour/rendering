#include "Precomp.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
    : BaseIndex(0)
    , IndexCount(0)
{
}

IndexBuffer::~IndexBuffer()
{
}

HRESULT IndexBuffer::Initialize(const ComPtr<ID3D11Device>& device, const void* data, uint32_t dataBytes)
{
    IB = nullptr;

    D3D11_BUFFER_DESC desc{};
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.ByteWidth = dataBytes;
    desc.StructureByteStride = sizeof(uint32_t);
    desc.Usage = D3D11_USAGE_DEFAULT;

    assert(desc.ByteWidth % desc.StructureByteStride == 0);

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = data;
    init.SysMemPitch = desc.ByteWidth;

    HRESULT hr = device->CreateBuffer(&desc, &init, &IB);
    CHECKHR(hr);

    BaseIndex = 0;
    IndexCount = desc.ByteWidth / desc.StructureByteStride;

    return hr;
}
