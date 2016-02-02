#pragma once

// base class for presenters
class BasePresenter : NonCopyable
{
public:
    BasePresenter(ID3D11Device* device)
        : Device(device)
    {}

    virtual ~BasePresenter()
    {}

    virtual HRESULT Initialize() = 0;

    virtual const ComPtr<ID3D11RenderTargetView>& GetBackBufferRTV() const = 0;

    virtual HRESULT Present() = 0;

protected:
    ComPtr<ID3D11Device> Device;
};
