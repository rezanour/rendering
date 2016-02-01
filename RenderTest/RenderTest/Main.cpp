#include "Precomp.h"
#include "Presenter.h"

using namespace Microsoft::WRL;

static const wchar_t AppClassName[] = L"RenderTest";
static const uint32_t ClientWidth = 1280;
static const uint32_t ClientHeight = 720;

static HRESULT AppInitialize(HINSTANCE instance, HWND* appWindow);
static LRESULT CALLBACK AppWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    HWND appWindow = nullptr;
    HRESULT hr = AppInitialize(instance, &appWindow);
    if (SUCCEEDED(hr))
    {
        ComPtr<IDXGIFactory2> factory;
        ComPtr<IDXGIAdapter> adapter;
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (SUCCEEDED(hr))
        {
            hr = factory->EnumAdapters(0, &adapter);
        }
        if (SUCCEEDED(hr))
        {
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
            UINT flags = 0;
#ifdef _DEBUG
            flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
            hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                flags, &featureLevel, 1, D3D11_SDK_VERSION, &device, nullptr, &context);
        }

        if (SUCCEEDED(hr))
        {
            MSG msg{};
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
                }
            }
        }

        DestroyWindow(appWindow);
    }

    return 0;
}

HRESULT AppInitialize(HINSTANCE instance, HWND* appWindow)
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

    *appWindow = CreateWindow(AppClassName, AppClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);

    if (*appWindow == nullptr)
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
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
