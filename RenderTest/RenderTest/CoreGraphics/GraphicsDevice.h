#pragma once

enum class VertexFormat;

class ShaderPass;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Texture2D;

class GraphicsDevice : NonCopyable
{
public:
    GraphicsDevice();
    virtual ~GraphicsDevice();

    HRESULT Initialize(const ComPtr<IDXGIFactory2>& factory, const ComPtr<IDXGIAdapter>& adapter, bool createDebug);

    const ComPtr<ID3D11Device>& GetDevice() const;
    const ComPtr<ID3D11DeviceContext>& GetContext() const;

    // Shader creation
    HRESULT CreateShaderPassGraphics(VertexFormat format, const uint8_t* vertexShader, size_t vertexShaderNumBytes,
        const uint8_t* pixelShader, size_t pixelShaderNumBytes, std::shared_ptr<ShaderPass>* shaderPass);

    HRESULT CreateShaderPassCompute(const uint8_t* computeShader, size_t computeShaderNumBytes, std::shared_ptr<ShaderPass>* shaderPass);

    // Buffer creation
    HRESULT CreateVertexBuffer(VertexFormat format, const void* data, uint32_t dataSizeInBytes, std::shared_ptr<VertexBuffer>* vertexBuffer);
    HRESULT CreateIndexBuffer(const void* data, uint32_t dataSizeInBytes, std::shared_ptr<IndexBuffer>* indexBuffer);
    HRESULT CreateConstantBuffer(const void* data, uint32_t dataSizeInBytes, std::shared_ptr<ConstantBuffer>* constantBuffer);

    // Texture creation
    HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, std::shared_ptr<Texture2D>* texture);
    HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, DXGI_FORMAT rtvFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT dsvFormat, std::shared_ptr<Texture2D>* texture);
    HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, const void* initData, std::shared_ptr<Texture2D>* texture);
    HRESULT CreateTexture2D(const ComPtr<ID3D11Texture2D>& existing, std::shared_ptr<Texture2D>* texture);

    // Common pipeline state objects
    const ComPtr<ID3D11SamplerState>& GetLinearWrapSampler() const;
    const ComPtr<ID3D11SamplerState>& GetAnisoWrapSampler() const;
    const ComPtr<ID3D11SamplerState>& GetPointClampSampler() const;
    const ComPtr<ID3D11DepthStencilState>& GetDepthWriteState() const;
    const ComPtr<ID3D11DepthStencilState>& GetDepthReadState() const;

private:
    ComPtr<IDXGIFactory2> Factory;
    ComPtr<IDXGIAdapter> Adapter;
    ComPtr<ID3D11Device> Device;
    ComPtr<ID3D11DeviceContext> Context;

    // Common states
    ComPtr<ID3D11SamplerState> LinearWrapSampler;
    ComPtr<ID3D11SamplerState> AnisoWrapSampler;
    ComPtr<ID3D11SamplerState> PointClampSampler;
    ComPtr<ID3D11DepthStencilState> DepthWriteState;
    ComPtr<ID3D11DepthStencilState> DepthReadState;
};
