#pragma once

struct RenderTarget;
class RenderScene;
struct RenderView;

class BaseRenderer : NonCopyable
{
public:
    BaseRenderer(ID3D11Device* device)
        : Device(device)
    {}

    virtual ~BaseRenderer()
    {}

    virtual HRESULT Initialize() = 0;

    virtual bool IsMsaaEnabled() const = 0;
    virtual bool EnableMsaa(bool enable) = 0;

    virtual void SetScene(const std::shared_ptr<RenderScene>& scene) = 0;

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) = 0;

protected:
    ComPtr<ID3D11Device> Device;
};
