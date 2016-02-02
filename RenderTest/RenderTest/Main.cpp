#include "Precomp.h"
#include "DxgiPresenter.h"
#include "LPFRenderer.h"
#include "RenderTarget.h"
#include "RenderingCommon.h"

using namespace Microsoft::WRL;

static const wchar_t AppClassName[] = L"RenderTest";
static const uint32_t ClientWidth = 1280;
static const uint32_t ClientHeight = 720;

static HWND AppWindow;
static ComPtr<IDXGIFactory2> Factory;
static ComPtr<IDXGIAdapter> Adapter;
static ComPtr<ID3D11Device> Device;
static ComPtr<ID3D11DeviceContext> Context;
static std::shared_ptr<RenderTarget> TheRenderTarget;
static std::unique_ptr<BasePresenter> Presenter;
static std::unique_ptr<BaseRenderer> Renderer;

static HRESULT AppInitialize(HINSTANCE instance);
static LRESULT CALLBACK AppWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void AppShutdown();

static HRESULT GfxInitialize();
static void GfxShutdown();

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    HRESULT hr = AppInitialize(instance);
    if (FAILED(hr))
    {
        assert(false);
        return -1;
    }

    hr = GfxInitialize();
    if (FAILED(hr))
    {
        assert(false);
        AppShutdown();
        return -1;
    }

    ShowWindow(AppWindow, SW_SHOW);
    UpdateWindow(AppWindow);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Idle
            RenderView view{};

            hr = Renderer->RenderFrame(TheRenderTarget, view);
            if (FAILED(hr))
            {
                assert(false);
                break;
            }

            hr = Presenter->Present();
            if (FAILED(hr))
            {
                assert(false);
                break;
            }
        }
    }

    GfxShutdown();
    AppShutdown();
    return 0;
}

HRESULT AppInitialize(HINSTANCE instance)
{
    WNDCLASSEX wcx{};
    wcx.cbSize = sizeof(wcx);
    wcx.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wcx.hInstance = instance;
    wcx.lpfnWndProc = AppWindowProc;
    wcx.lpszClassName = AppClassName;

    if (RegisterClassEx(&wcx) == INVALID_ATOM)
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    RECT rc{};
    rc.right = ClientWidth;
    rc.bottom = ClientHeight;
    AdjustWindowRect(&rc, style, FALSE);

    AppWindow = CreateWindow(AppClassName, AppClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);

    if (AppWindow == nullptr)
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

LRESULT CALLBACK AppWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void AppShutdown()
{
    if (AppWindow)
    {
        DestroyWindow(AppWindow);
        AppWindow = nullptr;
    }
}

HRESULT GfxInitialize()
{
    UINT dxgiFlags = 0;
#ifdef _DEBUG
    dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    HRESULT hr = CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&Factory));
    CHECKHR(hr);

    hr = Factory->EnumAdapters(0, &Adapter);
    CHECKHR(hr);

    UINT d3dFlags = 0;
#ifdef _DEBUG
    d3dFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    hr = D3D11CreateDevice(Adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
        d3dFlags, &featureLevel, 1, D3D11_SDK_VERSION, &Device, nullptr, &Context);
    CHECKHR(hr);

    Presenter.reset(new DxgiPresenter(Device.Get(), AppWindow));
    hr = Presenter->Initialize();
    CHECKHR(hr);

    TheRenderTarget = std::make_shared<RenderTarget>();
    hr = TheRenderTarget->Initialize(Presenter->GetBackBufferRTV().Get());
    CHECKHR(hr);

    Renderer.reset(new LPFRenderer(Device.Get()));
    hr = Renderer->Initialize();
    CHECKHR(hr);

    return hr;
}

void GfxShutdown()
{
    TheRenderTarget = nullptr;
    Renderer = nullptr;
    Presenter = nullptr;
    Context = nullptr;
    Device = nullptr;
    Adapter = nullptr;
    Factory = nullptr;
}
