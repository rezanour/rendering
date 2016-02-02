#pragma once

#include "RenderingCommon.h"

class BaseRenderer
{
public:
    BaseRenderer(ID3D11Device* device)
        : Device(device)
    {}

    virtual ~BaseRenderer()
    {}

    virtual HRESULT Initialize() = 0;

    virtual HRESULT BeginFrame(const std::shared_ptr<RenderTarget>& renderTarget, const RenderView& view) = 0;
    virtual HRESULT EndFrame() = 0;

private:
    BaseRenderer(const BaseRenderer&) = delete;
    BaseRenderer& operator= (const BaseRenderer&) = delete;

protected:
    ComPtr<ID3D11Device> Device;
};
