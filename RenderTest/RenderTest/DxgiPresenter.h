#pragma once

#include "BasePresenter.h"

// DXGI swapchain based presenter
class DxgiPresenter : public BasePresenter
{
public:
    DxgiPresenter(ID3D11Device* device, HWND targetWindow);
    virtual ~DxgiPresenter();

    virtual HRESULT Initialize() override;

    virtual const std::shared_ptr<Texture2D>& GetBackBuffer() const override
    {
        return BackBuffer;
    }

    virtual HRESULT Present() override;

private:
    DxgiPresenter(const DxgiPresenter&) = delete;
    DxgiPresenter& operator= (const DxgiPresenter&) = delete;

private:
    HWND TargetWindow;
    ComPtr<IDXGISwapChain1> SwapChain;
    std::shared_ptr<Texture2D> BackBuffer;
};
