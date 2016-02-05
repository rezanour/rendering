#pragma once

#include "BaseRenderer.h"
#include "RenderingCommon.h"

class RenderVisual;
class VertexBuffer;

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(const ComPtr<ID3D11Device>& device);
    virtual ~LPFRenderer();

    virtual HRESULT Initialize() override;

    virtual void SetScene(const std::shared_ptr<RenderScene>& scene) override
    {
        Scene = scene;
    }

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) override;

private:
    void UnbindAllRTVsAndSRVs();
    void RenderGBuffer(const RenderView& view);
    void RenderLights(const RenderView& view);
    void RenderFinal(const RenderView& view, const RenderTarget& renderTarget);

private:
    ComPtr<ID3D11DeviceContext> Context;
    std::shared_ptr<RenderScene> Scene;

    std::vector<std::shared_ptr<RenderVisual>> Visuals;
    D3D11_VIEWPORT Viewport{};

    ComPtr<ID3D11DepthStencilState> DepthWriteState;
    ComPtr<ID3D11DepthStencilState> DepthReadState;

    // GBuffer pass
    struct GBufferVSConstants
    {
        XMFLOAT4X4 LocalToView;   // local -> world -> View
        XMFLOAT4X4 LocalToProjection; // local -> world -> view -> projection
    };

    ComPtr<ID3D11InputLayout> GBufferIL;
    ComPtr<ID3D11Buffer> GBufferVSCB;
    ComPtr<ID3D11VertexShader> GBufferVS;
    ComPtr<ID3D11PixelShader> GBufferPS;
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

    ComPtr<ID3D11InputLayout> DLightIL;
    ComPtr<ID3D11VertexShader> DLightVS;
    ComPtr<ID3D11PixelShader> DLightPS;
    ComPtr<ID3D11Buffer> DLightVSCB;
    std::shared_ptr<Texture2D> LightRT;

    // Final pass (TODO: should really be per-object materials)
    ComPtr<ID3D11InputLayout> FinalIL;
    ComPtr<ID3D11VertexShader> FinalVS;
    ComPtr<ID3D11PixelShader> FinalPS;
};
