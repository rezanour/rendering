#pragma once

#include "CoreGraphics/RenderingCommon.h"
#include "BaseRenderer.h"

class Visual;
class Light;
class ConstantBuffer;
class ShaderPass;

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(const std::shared_ptr<GraphicsDevice>& graphics);
    virtual ~LPFRenderer();

    virtual HRESULT Initialize() override;

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) override;

private:
    void RenderGBuffer(const RenderView& view);
    void RenderLights(const RenderView& view);
    void RenderFinal(const RenderView& view, const RenderTarget& renderTarget);

    HRESULT RecreateSurfaces(uint32_t width, uint32_t height, uint32_t sampleCount);

private:
    ComPtr<ID3D11DeviceContext> Context;
    D3D11_VIEWPORT Viewport{};

    // scratch buffers used to cache results throughout the frame
    std::vector<std::shared_ptr<Visual>> Visuals;
    std::vector<std::shared_ptr<Light>> Lights;

    // GBuffer pass
    struct GBufferVSConstants
    {
        XMFLOAT4X4 LocalToView;   // local -> world -> View
        XMFLOAT4X4 LocalToProjection; // local -> world -> view -> projection
    };

    std::shared_ptr<ShaderPass> GBufferPass;
    std::shared_ptr<ConstantBuffer> GBufferVSCB;
    std::shared_ptr<Texture2D> GBufferViewNormalsRT;
    std::shared_ptr<Texture2D> GBufferLinearDepthRT;
    std::shared_ptr<Texture2D> GBufferDepthBuffer;

    // Light pass

    std::shared_ptr<Visual> QuadVisual;

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
    std::shared_ptr<ConstantBuffer> DLightVSCB;
    std::shared_ptr<ConstantBuffer> DLightPSCB;
    std::shared_ptr<Texture2D> LightRT;

    // Final pass (TODO: should really be per-object materials)
    std::shared_ptr<ShaderPass> FinalPass;
    std::shared_ptr<ShaderPass> FinalPassMsaa;
    std::shared_ptr<Texture2D> FinalRTMsaa;
};
