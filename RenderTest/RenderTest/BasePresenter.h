#pragma once

class Texture2D;

// base class for presenters
class BasePresenter : NonCopyable
{
public:
    BasePresenter(const ComPtr<ID3D11Device>& device)
        : Device(device)
    {}

    virtual ~BasePresenter()
    {}

    virtual HRESULT Initialize() = 0;

    virtual const std::shared_ptr<Texture2D>& GetBackBuffer() const = 0;

    virtual HRESULT Present() = 0;

protected:
    ComPtr<ID3D11Device> Device;
};
