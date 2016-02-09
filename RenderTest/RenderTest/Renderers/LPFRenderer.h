#pragma once

#include "CoreGraphics/BaseRenderer.h"
#include "CoreGraphics/RenderingCommon.h"

class Visual;
class Light;
class VertexBuffer;
class ConstantBuffer;
class ShaderPass;

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(const std::shared_ptr<GraphicsDevice>& graphics);
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

    virtual void SetScene(const std::shared_ptr<Scene>& scene) override
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
    std::shared_ptr<Scene> Scene;

    // scratch buffers used to cache results throughout the frame
    std::vector<std::shared_ptr<Visual>> Visuals;
    std::vector<std::shared_ptr<Light>> Lights;

    bool MsaaEnabled = true;
    D3D11_VIEWPORT Viewport{};

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
    std::shared_ptr<Texture2D> FinalRT;
};
