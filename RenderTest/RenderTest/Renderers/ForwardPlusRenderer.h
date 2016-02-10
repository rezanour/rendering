#pragma once

#include "CoreGraphics/RenderingCommon.h"
#include "BaseRenderer.h"

class Visual;
class Light;
class ConstantBuffer;
class Buffer;
class ShaderPass;

// Forward+ Renderer
class ForwardPlusRenderer : public BaseRenderer
{
public:
    ForwardPlusRenderer(const std::shared_ptr<GraphicsDevice>& graphics);
    virtual ~ForwardPlusRenderer();

    virtual HRESULT Initialize() override;

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) override;

private:
    void RenderZPrePass(const RenderView& view);
    void CullLights(const RenderView& view);
    void RenderFinal(const RenderView& view, const RenderTarget& renderTarget);

    HRESULT RecreateSurfaces(uint32_t width, uint32_t height, uint32_t sampleCount);

private:
    ComPtr<ID3D11DeviceContext> Context;
    D3D11_VIEWPORT Viewport{};

    // scratch buffers used to cache results throughout the frame
    std::vector<std::shared_ptr<Visual>> Visuals;
    std::vector<std::shared_ptr<Light>> Lights;

    std::shared_ptr<Texture2D> DepthBuffer;

    struct PointLight
    {
        XMFLOAT3 Position;
        float Radius;
        XMFLOAT3 Color;
        float Pad;
    };

    static const uint32_t MaxPointLights = 64 * 1024 * 1024;
    std::shared_ptr<Buffer> LightBuffer;
    std::vector<PointLight> PointLightsScratch;

    static const uint32_t LinkedListMaxElements = 128 * 1024 * 1024;
    struct LightLinkedListNode
    {
        uint32_t LightIndex;
        uint32_t NextLight;
    };
    std::shared_ptr<Buffer> LightLinkedListNodes;
    std::shared_ptr<Buffer> LightLinkedListHeads;

    // ZPrePass
    struct ZPrePassVSConstants
    {
        XMFLOAT4X4 LocalToProjection; // local -> world -> view -> projection
    };

    std::shared_ptr<ShaderPass> ZPrePass;
    std::shared_ptr<ConstantBuffer> ZPrePassVSCB;

    // Light culling pass
    struct LightCullConstants
    {
        uint32_t TileSize;
        uint32_t NumTilesX;
        uint32_t NumLights;
        uint32_t Pad;
    };

    std::shared_ptr<ShaderPass> LightCullPass;
    std::shared_ptr<ConstantBuffer> LightCullCB;

    // Final pass (TODO: should really be per-object materials)
    struct FinalPassVSConstants
    {
        XMFLOAT4X4 LocalToView;         // local -> world -> view
        XMFLOAT4X4 LocalToProjection;   // local -> world -> view -> projection
    };

    struct DLight
    {
        XMFLOAT3 Direction;
        float Pad0;
        XMFLOAT3 Color;
        float Pad1;
    };

    static const uint32_t MaxDLights = 8;
    struct FinalPassPSConstants
    {
        DLight Lights[MaxDLights];
        uint32_t NumLights;
        uint32_t TileSize;
        uint32_t NumTilesX;
        uint32_t Pad;
    };

    std::shared_ptr<ShaderPass> FinalPass;
    std::shared_ptr<ShaderPass> FinalPassMsaa;
    std::shared_ptr<ConstantBuffer> FinalPassVSCB;
    std::shared_ptr<ConstantBuffer> FinalPassPSCB;
    std::shared_ptr<Texture2D> FinalRTMsaa;
};
