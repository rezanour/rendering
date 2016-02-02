#pragma once

class RenderTarget;
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

    virtual HRESULT RenderFrame(const std::shared_ptr<RenderTarget>& renderTarget, const RenderView& view) = 0;

protected:
    ComPtr<ID3D11Device> Device;
};
