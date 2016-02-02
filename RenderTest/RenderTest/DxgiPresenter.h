#pragma once

#include "BasePresenter.h"

// DXGI swapchain based presenter
class DxgiPresenter : public BasePresenter
{
public:
    DxgiPresenter(ID3D11Device* device, HWND targetWindow);
    virtual ~DxgiPresenter();

    virtual HRESULT Initialize() override;

    virtual const ComPtr<ID3D11RenderTargetView>& GetBackBufferRTV() const override
    {
        return BackBufferRTV;
    }

    virtual HRESULT Present() override;

private:
    DxgiPresenter(const DxgiPresenter&) = delete;
    DxgiPresenter& operator= (const DxgiPresenter&) = delete;

private:
    HWND TargetWindow;
    ComPtr<IDXGISwapChain1> SwapChain;
    ComPtr<ID3D11Texture2D> BackBuffer;
    ComPtr<ID3D11RenderTargetView> BackBufferRTV;
};
