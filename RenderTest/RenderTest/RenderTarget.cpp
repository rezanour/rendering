#include "Precomp.h"
#include "RenderTarget.h"

RenderTarget::RenderTarget()
{
    ZeroMemory(&Desc, sizeof(Desc));
    ZeroMemory(&Viewport, sizeof(Viewport));
}

RenderTarget::~RenderTarget()
{
}

HRESULT RenderTarget::Initialize(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc)
{
    assert(desc.BindFlags & D3D11_BIND_RENDER_TARGET);

    Texture = nullptr;
    Rtv = nullptr;

    Desc = desc;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &Texture);
    CHECKHR(hr);

    hr = device->CreateRenderTargetView(Texture.Get(), nullptr, &Rtv);
    CHECKHR(hr);

    ZeroMemory(&Viewport, sizeof(Viewport));
    Viewport.Width = static_cast<float>(Desc.Width);
    Viewport.Height = static_cast<float>(Desc.Height);
    Viewport.MaxDepth = 1.f;

    return hr;
}

HRESULT RenderTarget::Initialize(ID3D11RenderTargetView* rtv)
{
    Texture = nullptr;
    Rtv = rtv;

    ComPtr<ID3D11Resource> resource;
    rtv->GetResource(&resource);

    HRESULT hr = resource.As(&Texture);
    CHECKHR(hr);

    Texture->GetDesc(&Desc);

    ZeroMemory(&Viewport, sizeof(Viewport));
    Viewport.Width = static_cast<float>(Desc.Width);
    Viewport.Height = static_cast<float>(Desc.Height);
    Viewport.MaxDepth = 1.f;

    return hr;
}
