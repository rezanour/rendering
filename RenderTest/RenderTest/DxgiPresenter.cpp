#include "Precomp.h"
#include "DxgiPresenter.h"
#include "Texture.h"

DxgiPresenter::DxgiPresenter(ID3D11Device* device, HWND targetWindow)
    : BasePresenter(device)
    , TargetWindow(targetWindow)
{
}

DxgiPresenter::~DxgiPresenter()
{
}

HRESULT DxgiPresenter::Initialize()
{
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = Device.As(&dxgiDevice);
    CHECKHR(hr);

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    CHECKHR(hr);

    ComPtr<IDXGIFactory2> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(&factory));
    CHECKHR(hr);

    RECT clientRect{};
    GetClientRect(TargetWindow, &clientRect);

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.BufferCount = 2;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Width = clientRect.right - clientRect.left;
    desc.Height = clientRect.bottom - clientRect.top;
    desc.SampleDesc.Count = 1;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.Scaling = DXGI_SCALING_STRETCH;

    hr = factory->CreateSwapChainForHwnd(Device.Get(), TargetWindow,
        &desc, nullptr, nullptr, &SwapChain);
    CHECKHR(hr);

    ComPtr<ID3D11Texture2D> texture;
    hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(&texture));
    CHECKHR(hr);

    BackBuffer = std::make_shared<Texture2D>();
    hr = BackBuffer->WrapExisting(texture);
    CHECKHR(hr);

    return hr;
}

HRESULT DxgiPresenter::Present()
{
    HRESULT hr = SwapChain->Present(1, 0);
    CHECKHR(hr);

    return hr;
}
