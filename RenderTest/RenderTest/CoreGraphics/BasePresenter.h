#pragma once

class GraphicsDevice;
class Texture2D;

// base class for presenters
class BasePresenter : NonCopyable
{
public:
    BasePresenter(const std::shared_ptr<GraphicsDevice>& graphics)
        : Graphics(graphics)
    {}

    virtual ~BasePresenter()
    {}

    virtual HRESULT Initialize() = 0;

    virtual const std::shared_ptr<Texture2D>& GetBackBuffer() const = 0;

    virtual HRESULT Present() = 0;

protected:
    std::shared_ptr<GraphicsDevice> Graphics;
};
