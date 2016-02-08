#include "Precomp.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
    : SizeInBytes(0)
{
}

ConstantBuffer::~ConstantBuffer()
{
}

HRESULT ConstantBuffer::Initialize(const ComPtr<ID3D11Device>& device, const void* data, uint32_t dataBytes)
{
    CB = nullptr;

    if (dataBytes % 16 != 0)
    {
        assert(false);
        return E_INVALIDARG;
    }

    SizeInBytes = dataBytes;

    D3D11_BUFFER_DESC desc{};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = dataBytes;
    desc.StructureByteStride = dataBytes;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = S_OK;
    if (data)
    {
        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = data;
        init.SysMemPitch = desc.ByteWidth;

        hr = device->CreateBuffer(&desc, &init, &CB);
        CHECKHR(hr);
    }
    else
    {
        hr = device->CreateBuffer(&desc, nullptr, &CB);
        CHECKHR(hr);
    }

    return hr;
}

HRESULT ConstantBuffer::Update(const void* data, uint32_t dataBytes)
{
    if (dataBytes != SizeInBytes)
    {
        assert(false);
        return E_INVALIDARG;
    }

    ComPtr<ID3D11Device> device;
    CB->GetDevice(&device);

    ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(&context);

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = context->Map(CB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    CHECKHR(hr);

    if (mapped.RowPitch == SizeInBytes)
    {
        memcpy_s(mapped.pData, mapped.RowPitch, data, dataBytes);
    }
    else
    {
        // Don't return here, since we need to unmap the CB first
        assert(false);
        hr = E_UNEXPECTED;
    }

    context->Unmap(CB.Get(), 0);

    CHECKHR(hr);
    return hr;
}
