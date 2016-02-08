#pragma once

class GraphicsDevice;
struct RenderTarget;
class RenderScene;
struct RenderView;

class BaseRenderer : NonCopyable
{
public:
    BaseRenderer(const std::shared_ptr<GraphicsDevice>& graphics)
        : Graphics(graphics)
    {}

    virtual ~BaseRenderer()
    {}

    virtual HRESULT Initialize() = 0;

    virtual bool IsMsaaEnabled() const = 0;
    virtual HRESULT EnableMsaa(bool enable) = 0;

    virtual void SetScene(const std::shared_ptr<RenderScene>& scene) = 0;

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) = 0;

protected:
    std::shared_ptr<GraphicsDevice> Graphics;
};
