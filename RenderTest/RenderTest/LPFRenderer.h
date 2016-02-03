#pragma once

#include "BaseRenderer.h"
#include "RenderingCommon.h"

// Light-prepass forward renderer
class LPFRenderer : public BaseRenderer
{
public:
    LPFRenderer(ID3D11Device* device);
    virtual ~LPFRenderer();

    virtual HRESULT Initialize() override;

    virtual bool IsMsaaEnabled() const override
    {
        return MsaaEnabled;
    }

    virtual bool EnableMsaa(bool enable) override
    {
        MsaaEnabled = enable;
        return true;
    }

    virtual void SetScene(const std::shared_ptr<RenderScene>& scene) override
    {
        Scene = scene;
    }

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) override;

private:
    HRESULT EnsureMsaaRenderTarget(const std::shared_ptr<Texture2D>& resolveTarget);
    HRESULT EnsureDepthBuffer(const std::shared_ptr<Texture2D>& renderTarget);

private:
    ComPtr<ID3D11DeviceContext> Context;
    std::shared_ptr<RenderScene> Scene;

    bool MsaaEnabled;
    RenderTarget MsaaRenderTarget;
    std::shared_ptr<Texture2D> MsaaDepthBuffer;
    std::shared_ptr<Texture2D> DepthBuffer;

    const RenderTarget* CurrentRenderTarget;
    std::shared_ptr<Texture2D> CurrentDepthBuffer;
};
