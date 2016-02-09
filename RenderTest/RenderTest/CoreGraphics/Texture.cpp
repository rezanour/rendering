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

HRESULT Texture2D::Initialize(const ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT rtvFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT dsvFormat)
{
    return Initialize(device, desc, rtvFormat, srvFormat, dsvFormat, nullptr);
}

HRESULT Texture2D::Initialize(const ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT rtvFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT dsvFormat, const void* data)
{
    Texture = nullptr;
    SRV = nullptr;
    RTV = nullptr;
    UAV = nullptr;
    DSV = nullptr;

    Desc = desc;

    HRESULT hr = S_OK;
    if (data)
    {
        if (desc.MipLevels > 1 && desc.Width == desc.Height)
        {
            uint32_t width = desc.Width;
            uint32_t height = desc.Height;
            const uint8_t* pData = (const uint8_t*)data;
            std::unique_ptr<D3D11_SUBRESOURCE_DATA[]> inits(new D3D11_SUBRESOURCE_DATA[desc.MipLevels]);
            for (uint32_t i = 0; i < desc.MipLevels; ++i)
            {
                inits[i].pSysMem = pData;
                inits[i].SysMemPitch = width * 4;
                inits[i].SysMemSlicePitch = width * height * 4;

                pData += inits[i].SysMemSlicePitch;
                width >>= 1;
                height >>= 1;
            }

            hr = device->CreateTexture2D(&desc, inits.get(), &Texture);
            CHECKHR(hr);
        }
        else
        {
            D3D11_TEXTURE2D_DESC d = desc;
            d.MipLevels = 1;
            D3D11_SUBRESOURCE_DATA init{};
            init.pSysMem = data;
            init.SysMemPitch = desc.Width * 4; // HACK (4 byte pixels only)
            init.SysMemSlicePitch = desc.Width * desc.Height * 4;
            hr = device->CreateTexture2D(&d, &init, &Texture);
            CHECKHR(hr);
        }
    }
    else
    {
        hr = device->CreateTexture2D(&desc, nullptr, &Texture);
        CHECKHR(hr);
    }

    hr = CreateViews(device, rtvFormat, srvFormat, dsvFormat);
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

    HRESULT hr = CreateViews(device, Desc.Format, Desc.Format, Desc.Format);
    CHECKHR(hr);

    return hr;
}

HRESULT Texture2D::CreateViews(const ComPtr<ID3D11Device>& device, DXGI_FORMAT rtvFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT dsvFormat)
{
    HRESULT hr = S_OK;

    if (Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
        srvd.ViewDimension = Desc.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
        srvd.Format = srvFormat;
        srvd.Texture2D.MipLevels = (UINT)-1;
        hr = device->CreateShaderResourceView(Texture.Get(), &srvd, &SRV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_RENDER_TARGET)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvd{};
        rtvd.ViewDimension = Desc.SampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvd.Format = rtvFormat;
        hr = device->CreateRenderTargetView(Texture.Get(), &rtvd, &RTV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        hr = device->CreateUnorderedAccessView(Texture.Get(), nullptr, &UAV);
        CHECKHR(hr);
    }

    if (Desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
        dsvd.ViewDimension = Desc.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvd.Format = dsvFormat;
        hr = device->CreateDepthStencilView(Texture.Get(), &dsvd, &DSV);
        CHECKHR(hr);
    }

    return hr;
}
