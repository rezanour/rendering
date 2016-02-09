#include "Precomp.h"
#include "GraphicsDevice.h"
#include "ShaderPass.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"

GraphicsDevice::GraphicsDevice()
{
}

GraphicsDevice::~GraphicsDevice()
{
}

HRESULT GraphicsDevice::Initialize(const ComPtr<IDXGIFactory2>& factory, const ComPtr<IDXGIAdapter>& adapter, bool createDebug)
{
    Factory = factory;
    Adapter = adapter;

    uint32_t flags = 0;
    if (createDebug)
    {
        flags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDevice(Adapter.Get(), Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
        nullptr, flags, &featureLevel, 1, D3D11_SDK_VERSION, &Device, nullptr, &Context);
    CHECKHR(hr);

    // create common states
    D3D11_SAMPLER_DESC sd{};
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device->CreateSamplerState(&sd, &LinearWrapSampler);
    CHECKHR(hr);

    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    hr = Device->CreateSamplerState(&sd, &PointClampSampler);
    CHECKHR(hr);

    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.Filter = D3D11_FILTER_ANISOTROPIC;
    sd.MaxAnisotropy = 8;
    hr = Device->CreateSamplerState(&sd, &AnisoWrapSampler);
    CHECKHR(hr);

    D3D11_DEPTH_STENCIL_DESC dsd{};
    dsd.DepthEnable = TRUE;
    dsd.DepthFunc = D3D11_COMPARISON_LESS;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    hr = Device->CreateDepthStencilState(&dsd, &DepthWriteState);
    CHECKHR(hr);

    dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    hr = Device->CreateDepthStencilState(&dsd, &DepthReadState);
    CHECKHR(hr);

    return hr;
}

const ComPtr<ID3D11Device>& GraphicsDevice::GetDevice() const
{
    return Device;
}

const ComPtr<ID3D11DeviceContext>& GraphicsDevice::GetContext() const
{
    return Context;
}

// Shader creation
HRESULT GraphicsDevice::CreateShaderPassGraphics(VertexFormat format, const uint8_t* vertexShader, size_t vertexShaderNumBytes,
    const uint8_t* pixelShader, size_t pixelShaderNumBytes, std::shared_ptr<ShaderPass>* shaderPass)
{
    if (!shaderPass)
    {
        assert(false);
        return E_POINTER;
    }

    *shaderPass = std::make_shared<ShaderPass>();
    HRESULT hr = (*shaderPass)->InitializeGraphics(Device, format, vertexShader, vertexShaderNumBytes, pixelShader, pixelShaderNumBytes);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateShaderPassCompute(const uint8_t* computeShader, size_t computeShaderNumBytes, std::shared_ptr<ShaderPass>* shaderPass)
{
    if (!shaderPass)
    {
        assert(false);
        return E_POINTER;
    }

    *shaderPass = std::make_shared<ShaderPass>();
    HRESULT hr = (*shaderPass)->InitializeCompute(Device, computeShader, computeShaderNumBytes);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateVertexBuffer(VertexFormat format, const void* data, uint32_t dataSizeInBytes, std::shared_ptr<VertexBuffer>* vertexBuffer)
{
    if (!vertexBuffer)
    {
        assert(false);
        return E_POINTER;
    }

    *vertexBuffer = std::make_shared<VertexBuffer>();
    HRESULT hr = (*vertexBuffer)->Initialize(Device, format, data, dataSizeInBytes);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateIndexBuffer(const void* data, uint32_t dataSizeInBytes, std::shared_ptr<IndexBuffer>* indexBuffer)
{
    if (!indexBuffer)
    {
        assert(false);
        return E_POINTER;
    }

    *indexBuffer = std::make_shared<IndexBuffer>();
    HRESULT hr = (*indexBuffer)->Initialize(Device, data, dataSizeInBytes);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateConstantBuffer(const void* data, uint32_t dataSizeInBytes, std::shared_ptr<ConstantBuffer>* constantBuffer)
{
    if (!constantBuffer)
    {
        assert(false);
        return E_POINTER;
    }

    *constantBuffer = std::make_shared<ConstantBuffer>();
    HRESULT hr = (*constantBuffer)->Initialize(Device, data, dataSizeInBytes);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, std::shared_ptr<Texture2D>* texture)
{
    if (!texture)
    {
        assert(false);
        return E_POINTER;
    }

    *texture = std::make_shared<Texture2D>();
    HRESULT hr = (*texture)->Initialize(Device, desc, desc.Format, desc.Format, desc.Format);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT rtvFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT dsvFormat, std::shared_ptr<Texture2D>* texture)
{
    if (!texture)
    {
        assert(false);
        return E_POINTER;
    }

    *texture = std::make_shared<Texture2D>();
    HRESULT hr = (*texture)->Initialize(Device, desc, rtvFormat, srvFormat, dsvFormat);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, const void* initData, std::shared_ptr<Texture2D>* texture)
{
    if (!texture)
    {
        assert(false);
        return E_POINTER;
    }

    *texture = std::make_shared<Texture2D>();
    HRESULT hr = (*texture)->Initialize(Device, desc, desc.Format, desc.Format, desc.Format, initData);
    CHECKHR(hr);

    return hr;
}

HRESULT GraphicsDevice::CreateTexture2D(const ComPtr<ID3D11Texture2D>& existing, std::shared_ptr<Texture2D>* texture)
{
    if (!texture)
    {
        assert(false);
        return E_POINTER;
    }

    *texture = std::make_shared<Texture2D>();
    HRESULT hr = (*texture)->WrapExisting(existing);
    CHECKHR(hr);

    return hr;
}

const ComPtr<ID3D11SamplerState>& GraphicsDevice::GetLinearWrapSampler() const
{
    return LinearWrapSampler;
}

const ComPtr<ID3D11SamplerState>& GraphicsDevice::GetAnisoWrapSampler() const
{
    return AnisoWrapSampler;
}

const ComPtr<ID3D11SamplerState>& GraphicsDevice::GetPointClampSampler() const
{
    return PointClampSampler;
}

const ComPtr<ID3D11DepthStencilState>& GraphicsDevice::GetDepthWriteState() const
{
    return DepthWriteState;
}

const ComPtr<ID3D11DepthStencilState>& GraphicsDevice::GetDepthReadState() const
{
    return DepthReadState;
}
