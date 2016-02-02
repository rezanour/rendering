#pragma once

class RenderTarget : NonCopyable
{
public:
    RenderTarget();
    virtual ~RenderTarget();

    HRESULT Initialize(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc);
    HRESULT Initialize(ID3D11RenderTargetView* rtv);

    const D3D11_TEXTURE2D_DESC& GetDesc() const
    {
        return Desc;
    }

    const ComPtr<ID3D11Texture2D>& GetTexture() const
    {
        return Texture;
    }

    const ComPtr<ID3D11RenderTargetView>& GetRTV() const
    {
        return Rtv;
    }

    const D3D11_VIEWPORT& GetViewport() const
    {
        return Viewport;
    }

    void SetViewport(const D3D11_VIEWPORT& viewport)
    {
        Viewport = viewport;
    }

private:
    ComPtr<ID3D11Texture2D> Texture;
    ComPtr<ID3D11RenderTargetView> Rtv;
    D3D11_TEXTURE2D_DESC Desc;
    D3D11_VIEWPORT Viewport;
};
