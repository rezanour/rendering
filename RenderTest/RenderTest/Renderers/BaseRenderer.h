#pragma once

class GraphicsDevice;
struct RenderTarget;
class Scene;
struct RenderView;

class BaseRenderer : NonCopyable
{
public:
    virtual ~BaseRenderer()
    {}

    virtual HRESULT Initialize() = 0;

    bool IsMsaaEnabled() const
    {
        return MsaaEnabled;
    }

    HRESULT EnableMsaa(bool enable)
    {
        if (!SupportsMsaa)
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }

        if (MsaaEnabled != enable)
        {
            MsaaEnabled = enable;
            RTWidth = RTHeight = 0;
        }
        return S_OK;
    }

    void SetScene(const std::shared_ptr<Scene>& scene)
    {
        Scene = scene;
    }

    virtual HRESULT RenderFrame(const RenderTarget& renderTarget, const RenderView& view) = 0;

protected:
    BaseRenderer(const std::shared_ptr<GraphicsDevice>& graphics, bool supportsMsaa)
        : Graphics(graphics)
        , SupportsMsaa(supportsMsaa)
    {}

protected:
    std::shared_ptr<GraphicsDevice> Graphics;
    std::shared_ptr<Scene> Scene;
    bool MsaaEnabled = false;
    bool SupportsMsaa = false;
    uint32_t RTWidth = 0;
    uint32_t RTHeight = 0;
};
