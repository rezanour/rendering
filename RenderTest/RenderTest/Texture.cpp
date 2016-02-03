#include "Precomp.h"
#include "Texture.h"

Texture::Texture()
{
}

Texture::~Texture()
{
}

Texture2D::Texture2D()
{
    ZeroMemory(&Desc, sizeof(Desc));
}

Texture2D::~Texture2D()
{
}

HRESULT Texture2D::Initialize(const ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& desc)
{
    Texture = nullptr;
    SRV = nullptr;
    RTV = nullptr;
    UAV = nullptr;
    DSV = nullptr;

    Desc = desc;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &Texture);
    CHECKHR(hr);

    hr = CreateViews(device);
    CHECKHR(hr);

    return hr;
}

HRESULT Texture2D::WrapExisting(const ComPtr<ID3D11Texture2D>& texture)
{
    Texture = texture;
    SRV = nullptr;
    RTV = nullptr;
    UAV = nullptr;
    DSV = nullptr;

    texture->GetDesc(&Desc);

    ComPtr<ID3D11Device> device;
    texture->GetDevice(&device);

    HRESULT hr = CreateViews(device);
    CHECKHR(hr);

    return hr;
}

HRESULT Texture2D::CreateViews(const ComPtr<ID3D11Device>& device)
{
    HRESULT hr = S_OK;

    if (Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
        hr = device->CreateShaderResourceView(Texture.Get(), nullptr, &SRV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_RENDER_TARGET)
    {
        hr = device->CreateRenderTargetView(Texture.Get(), nullptr, &RTV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        hr = device->CreateUnorderedAccessView(Texture.Get(), nullptr, &UAV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
    {
        hr = device->CreateDepthStencilView(Texture.Get(), nullptr, &DSV);
        CHECKHR(hr);
    }

    return hr;
}
