#pragma once

#include "VertexFormats.h"

class Visual;

enum class ShaderPassType
{
    Unknown = 0,
    Graphics,       // VS/PS
    Compute
};

class ShaderPass : NonCopyable
{
public:
    ShaderPass();
    virtual ~ShaderPass();

    ShaderPassType GetType() const
    {
        return Type;
    }

    HRESULT InitializeGraphics(
        const ComPtr<ID3D11Device>& device,
        VertexFormat format,
        const uint8_t* vertexShader, size_t vertexShaderNumBytes,
        const uint8_t* pixelShader, size_t pixelShaderNumBytes);

    HRESULT InitializeCompute(const ComPtr<ID3D11Device>& device, const uint8_t* computeShader, size_t computeShaderNumBytes);

    void SetRenderTarget(int slot, const ComPtr<ID3D11RenderTargetView>& rtv);
    // Passing nullptr clears viewport to "full size"
    void SetViewport(const D3D11_VIEWPORT* viewport);

    void SetDepthBuffer(const ComPtr<ID3D11DepthStencilView>& dsv);
    void SetDepthState(const ComPtr<ID3D11DepthStencilState>& depthState);

    void SetCSBuffer(int slot, const ComPtr<ID3D11UnorderedAccessView>& uav, bool resetCounterEachPass);
    void SetCSConstantBuffer(int slot, const ComPtr<ID3D11Buffer>& cb);
    void SetCSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv);

    void SetVSConstantBuffer(int slot, const ComPtr<ID3D11Buffer>& cb);
    void SetVSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv);

    void SetPSConstantBuffer(int slot, const ComPtr<ID3D11Buffer>& cb);
    void SetPSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv);

    void SetVSSampler(int slot, const ComPtr<ID3D11SamplerState>& sampler);
    void SetPSSampler(int slot, const ComPtr<ID3D11SamplerState>& sampler);

    void Begin();
    void Draw(const std::shared_ptr<Visual>& visual);
    void Dispatch(uint32_t numThreadsX, uint32_t numThreadsY, uint32_t numThreadsZ);
    void End();

private:
    void BeginGraphics();
    void BeginCompute();

    HRESULT CreateInputLayout(VertexFormat format, const uint8_t* vertexShader, size_t vertexShaderBytes);

private:
    ShaderPassType Type = ShaderPassType::Unknown;
    bool Rendering = false;

    ComPtr<ID3D11Device> Device;
    ComPtr<ID3D11DeviceContext> Context;

    ComPtr<ID3D11InputLayout> InputLayouts[(uint32_t)VertexFormat::Max];
    VertexFormat CurrentInputBinding = VertexFormat::Unknown;

    ComPtr<ID3D11VertexShader> VertexShader;
    ComPtr<ID3D11PixelShader> PixelShader;
    ComPtr<ID3D11ComputeShader> ComputeShader;

    ID3D11RenderTargetView* RenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    ComPtr<ID3D11DepthStencilView> DepthBuffer;
    ComPtr<ID3D11DepthStencilState> DepthState;
    D3D11_VIEWPORT Viewport{};

    ID3D11UnorderedAccessView* CSBuffers[D3D11_PS_CS_UAV_REGISTER_COUNT]{};
    uint32_t CSBufferCounters[D3D11_PS_CS_UAV_REGISTER_COUNT]{};

    ID3D11Buffer* VSCSConstants[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
    ID3D11ShaderResourceView* VSCSResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};

    ID3D11Buffer* PSConstants[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
    ID3D11ShaderResourceView* PSResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};

    ID3D11SamplerState* VSSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT]{};
    ID3D11SamplerState* PSSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT]{};
};
