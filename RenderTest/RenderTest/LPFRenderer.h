#pragma once

#include "BaseRenderer.h"

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(ID3D11Device* device);
    virtual ~LPFRenderer();

    virtual HRESULT Initialize() override;

    virtual HRESULT BeginFrame(const std::shared_ptr<RenderTarget>& renderTarget, const RenderView& view) override;
    virtual HRESULT EndFrame() override;

private:
    LPFRenderer(const LPFRenderer&) = delete;
    LPFRenderer& operator= (const LPFRenderer&) = delete;

    HRESULT EnsureMsaaRenderTarget();

private:
    ComPtr<ID3D11DeviceContext> Context;

    bool MsaaEnabled;
    std::shared_ptr<RenderTarget> MsaaRenderTarget;
    std::shared_ptr<RenderTarget> FinalRenderTarget;
    std::shared_ptr<RenderTarget> CurrentRenderTarget;
};
