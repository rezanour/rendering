#pragma once

class Texture : NonCopyable
{
public:
    virtual ~Texture();

    const ComPtr<ID3D11ShaderResourceView>& GetSRV() const
    {
        return SRV;
    }

    const ComPtr<ID3D11RenderTargetView>& GetRTV() const
    {
        return RTV;
    }

    const ComPtr<ID3D11UnorderedAccessView>& GetUAV() const
    {
        return UAV;
    }

    const ComPtr<ID3D11DepthStencilView>& GetDSV() const
    {
        return DSV;
    }

protected:
    Texture();

protected:
    ComPtr<ID3D11ShaderResourceView> SRV;
    ComPtr<ID3D11RenderTargetView> RTV;
    ComPtr<ID3D11UnorderedAccessView> UAV;
    ComPtr<ID3D11DepthStencilView> DSV;
};

class Texture2D : public Texture
{
public:
    Texture2D();
    virtual ~Texture2D();

    virtual HRESULT Initialize(const ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& desc);
    virtual HRESULT WrapExisting(const ComPtr<ID3D11Texture2D>& existing);

    const D3D11_TEXTURE2D_DESC& GetDesc() const
    {
        return Desc;
    }

    const ComPtr<ID3D11Texture2D>& GetTexture() const
    {
        return Texture;
    }

private:
    HRESULT CreateViews(const ComPtr<ID3D11Device>& device);

private:
    D3D11_TEXTURE2D_DESC Desc;
    ComPtr<ID3D11Texture2D> Texture;
};
