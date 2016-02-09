#include "Precomp.h"
#include "Buffer.h"

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

HRESULT Buffer::Initialize(const ComPtr<ID3D11Device>& device, uint32_t elementSize, uint32_t byteWidth, bool isStructured, bool isCounter)
{
    Resource = nullptr;
    SRV = nullptr;
    UAV = nullptr;

    Desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    Desc.ByteWidth = byteWidth;
    Desc.MiscFlags = isStructured ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    Desc.StructureByteStride = elementSize;
    Desc.Usage = D3D11_USAGE_DEFAULT;

    HRESULT hr = device->CreateBuffer(&Desc, nullptr, &Resource);
    CHECKHR(hr);

    hr = CreateViews(device, isStructured, isCounter);
    CHECKHR(hr);

    return hr;
}

HRESULT Buffer::CreateViews(const ComPtr<ID3D11Device>& device, bool isStructured, bool isCounter)
{
    HRESULT hr = S_OK;

    if (Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
        srvd.ViewDimension = isStructured ? D3D11_SRV_DIMENSION_BUFFER : D3D11_SRV_DIMENSION_BUFFEREX;
        if (isStructured)
        {
            srvd.Format = DXGI_FORMAT_UNKNOWN;
            srvd.Buffer.NumElements = Desc.ByteWidth / Desc.StructureByteStride;
            srvd.Buffer.ElementWidth = Desc.StructureByteStride;
        }
        else
        {
            srvd.Format = DXGI_FORMAT_R32_TYPELESS;
            srvd.BufferEx.NumElements = Desc.ByteWidth / Desc.StructureByteStride;
            srvd.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        }
        hr = device->CreateShaderResourceView(Resource.Get(), &srvd, &SRV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavd{};
        uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavd.Format = isStructured ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_R32_UINT;
        uavd.Buffer.NumElements = Desc.ByteWidth / Desc.StructureByteStride;
        uavd.Buffer.Flags = isCounter ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0;
        hr = device->CreateUnorderedAccessView(Resource.Get(), &uavd, &UAV);
        CHECKHR(hr);
    }

    return hr;
}
