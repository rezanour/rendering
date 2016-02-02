#pragma once

// base class for presenters
class BasePresenter
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

private:
    BasePresenter(const BasePresenter&) = delete;
    BasePresenter& operator= (const BasePresenter&) = delete;

protected:
    ComPtr<ID3D11Device> Device;
};
