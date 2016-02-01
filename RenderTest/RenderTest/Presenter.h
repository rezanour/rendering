#pragma once

// DXGI swapchain based presenter
class Presenter
{
public:
    Presenter(ID3D11Device* device, HWND targetWindow);
    virtual ~Presenter();

    HRESULT Initialize();

private:
    Presenter(const Presenter&) = delete;
    Presenter& operator= (const Presenter&) = delete;

private:
    HWND TargetWindow;
    Microsoft::WRL::ComPtr<ID3D11Device> Device;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRTV;
};
