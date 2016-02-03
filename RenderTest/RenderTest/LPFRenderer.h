#pragma once

#include "BaseRenderer.h"

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(ID3D11Device* device);
    virtual ~LPFRenderer();

    virtual HRESULT Initialize() override;

    virtual void SetScene(const std::shared_ptr<RenderScene>& scene) override
    {
        Scene = scene;
    }

    virtual HRESULT RenderFrame(const std::shared_ptr<RenderTarget>& renderTarget, const RenderView& view) override;

private:
    HRESULT EnsureMsaaRenderTarget(const std::shared_ptr<RenderTarget>& finalRenderTarget);

private:
    ComPtr<ID3D11DeviceContext> Context;

    bool MsaaEnabled;
    std::shared_ptr<RenderTarget> MsaaRenderTarget;
    std::shared_ptr<RenderTarget> CurrentRenderTarget;

    std::shared_ptr<RenderScene> Scene;
};
