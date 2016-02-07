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
    return Initialize(device, desc, nullptr);
}

HRESULT Texture2D::Initialize(const ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& desc, const void* data)
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
