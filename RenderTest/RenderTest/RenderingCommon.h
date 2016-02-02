#pragma once

// RenderTarget
struct RenderTarget
{
    ComPtr<ID3D11Texture2D> Texture;
    ComPtr<ID3D11RenderTargetView> RTV;
    D3D11_VIEWPORT Viewport;
    D3D11_TEXTURE2D_DESC Desc;

    RenderTarget()
    {
        ZeroMemory(&Viewport, sizeof(Viewport));
        ZeroMemory(&Desc, sizeof(Desc));
    }

    RenderTarget(ID3D11RenderTargetView* rtv)
        : RTV(rtv)
    {
        ComPtr<ID3D11Resource> resource;
        rtv->GetResource(&resource);
        resource.As(&Texture);
        Texture->GetDesc(&Desc);

        Viewport.TopLeftX = Viewport.TopLeftY = Viewport.MinDepth = 0.f;
        Viewport.Width = (float)Desc.Width;
        Viewport.Height = (float)Desc.Height;
        Viewport.MaxDepth = 1.f;
    }
};

// View information for rendering
struct RenderView
{
    XMFLOAT4X4 WorldToView;
    XMFLOAT4X4 ViewToProjection;
};
