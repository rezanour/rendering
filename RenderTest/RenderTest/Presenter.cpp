#include "Precomp.h"
#include "Presenter.h"

using namespace Microsoft::WRL;

Presenter::Presenter(ID3D11Device* device, HWND targetWindow)
    : TargetWindow(targetWindow)
    , Device(device)
{
}

Presenter::~Presenter()
{
}

HRESULT Presenter::Initialize()
{
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = Device.As(&dxgiDevice);
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    ComPtr<IDXGIFactory2> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    RECT clientRect{};
    GetClientRect(TargetWindow, &clientRect);

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.BufferCount = 2;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.Width = clientRect.right - clientRect.left;
    desc.Height = clientRect.bottom - clientRect.top;
    desc.SampleDesc.Count = 4;  // 4x MSAA
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.Scaling = DXGI_SCALING_STRETCH;

    hr = factory->CreateSwapChainForHwnd(Device.Get(), TargetWindow,
        &desc, nullptr, nullptr, &SwapChain);
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    hr = Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, &BackBufferRTV);
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    return hr;
}
