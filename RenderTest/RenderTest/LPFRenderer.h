#pragma once

#include "BaseRenderer.h"
#include "RenderingCommon.h"

class RenderVisual;
class VertexBuffer;
class ShaderPass;

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(const ComPtr<ID3D11Device>& device);
    virtual ~LPFRenderer();

    virtual HRESULT Initialize() override;

    virtual bool IsMsaaEnabled() const override
    {
        return MsaaEnabled;
    }

    virtual HRESULT EnableMsaa(bool enable) override
    {
        MsaaEnabled = enable;
        return RecreateSurfaces(MsaaEnabled ? 4 : 1);
    }

    virtual void SetScene(const std::shared_ptr<RenderScene>& scene) override
    {
        Scene = scene;
    }

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) override;

private:
    HRESULT RecreateSurfaces(uint32_t sampleCount);

    void RenderGBuffer(const RenderView& view);
    void RenderLights(const RenderView& view);
    void RenderFinal(const RenderView& view, const RenderTarget& renderTarget);

private:
    ComPtr<ID3D11DeviceContext> Context;
    std::shared_ptr<RenderScene> Scene;

    std::vector<std::shared_ptr<RenderVisual>> Visuals;
    D3D11_VIEWPORT Viewport{};

    bool MsaaEnabled = true;

    ComPtr<ID3D11DepthStencilState> DepthWriteState;
    ComPtr<ID3D11DepthStencilState> DepthReadState;
    ComPtr<ID3D11SamplerState> LinearSampler;

    // GBuffer pass
    struct GBufferVSConstants
    {
        XMFLOAT4X4 LocalToView;   // local -> world -> View
        XMFLOAT4X4 LocalToProjection; // local -> world -> view -> projection
    };

    std::shared_ptr<ShaderPass> GBufferPass;
    ComPtr<ID3D11Buffer> GBufferVSCB;
    std::shared_ptr<Texture2D> GBufferViewNormalsRT;
    std::shared_ptr<Texture2D> GBufferLinearDepthRT;
    std::shared_ptr<Texture2D> GBufferDepthBuffer;

    // Light pass

    std::shared_ptr<VertexBuffer> QuadVB;

    struct DLightVSConstants
    {
        float ClipDistance;
        XMFLOAT3 Padding;
    };

    static const uint32_t MaxDlightsPerPass = 8;
    struct DLight
    {
        XMFLOAT3 Direction;
        float Pad0;
        XMFLOAT3 Color;
        float Pad1;
    };

    struct DLightPSConstants
    {
        DLight Lights[MaxDlightsPerPass];
        uint32_t NumLights;
        XMFLOAT3 Pad0;
    };

    std::shared_ptr<ShaderPass> DLightPass;
    std::shared_ptr<ShaderPass> DLightPassMsaa;
    ComPtr<ID3D11Buffer> DLightVSCB;
    ComPtr<ID3D11Buffer> DLightPSCB;
    std::shared_ptr<Texture2D> LightRT;

    // Final pass (TODO: should really be per-object materials)
    std::shared_ptr<ShaderPass> FinalPass;
    std::shared_ptr<ShaderPass> FinalPassMsaa;
    std::shared_ptr<Texture2D> FinalRT;
};
