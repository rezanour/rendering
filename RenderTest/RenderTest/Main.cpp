#include "Precomp.h"
#include "DxgiPresenter.h"
#include "LPFRenderer.h"
#include "Texture.h"
#include "RenderingCommon.h"
#include "RenderScene.h"
#include "RenderVisual.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"

using namespace Microsoft::WRL;

static const wchar_t AppClassName[] = L"RenderTest";
static const uint32_t ClientWidth = 1280;
static const uint32_t ClientHeight = 720;

static HWND AppWindow;
static ComPtr<IDXGIFactory2> Factory;
static ComPtr<IDXGIAdapter> Adapter;
static ComPtr<ID3D11Device> Device;
static ComPtr<ID3D11DeviceContext> Context;
static std::unique_ptr<BasePresenter> Presenter;
static std::unique_ptr<BaseRenderer> Renderer;
static std::shared_ptr<RenderScene> Scene;
static std::shared_ptr<RenderVisual> Visual;
static RenderTarget BackBufferRT;
static RenderView View;

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
            XMFLOAT4 orientation = Visual->GetOrientation();
            XMVECTOR rotation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), XMConvertToRadians(1.f));
            XMStoreFloat4(&orientation, XMQuaternionNormalize(XMQuaternionMultiply(XMLoadFloat4(&orientation), rotation)));
            Visual->SetOrientation(orientation);


            hr = Renderer->RenderFrame(BackBufferRT, View);
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

    Presenter.reset(new DxgiPresenter(Device, AppWindow));
    hr = Presenter->Initialize();
    CHECKHR(hr);

    Renderer.reset(new LPFRenderer(Device));
    hr = Renderer->Initialize();
    CHECKHR(hr);

    Scene = std::make_shared<RenderScene>();

    PositionNormalVertex vertices[3]{};
    vertices[0].Position = XMFLOAT3(-1.f, 0.f, 0.f);
    vertices[1].Position = XMFLOAT3(0.f, 2.f, 0.f);
    vertices[2].Position = XMFLOAT3(1.f, 0.f, 0.f);
    vertices[0].Normal = XMFLOAT3(0.f, 0.f, -1.f);
    vertices[1].Normal = XMFLOAT3(0.f, 0.f, -1.f);
    vertices[2].Normal = XMFLOAT3(0.f, 0.f, -1.f);

    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>();
    hr = vb->Initialize(Device, VertexFormat::PositionNormal, vertices, sizeof(vertices));
    CHECKHR(hr);

    Visual = std::make_shared<RenderVisual>();
    hr = Visual->Initialize(vb);
    CHECKHR(hr);

    Scene->AddVisual(Visual);

    Renderer->SetScene(Scene);

    BackBufferRT.Texture = Presenter->GetBackBuffer();
    BackBufferRT.Viewport.Width = static_cast<float>(BackBufferRT.Texture->GetDesc().Width);
    BackBufferRT.Viewport.Height = static_cast<float>(BackBufferRT.Texture->GetDesc().Height);
    BackBufferRT.Viewport.MaxDepth = 1.f;

    XMStoreFloat4x4(&View.WorldToView,
        XMMatrixLookAtLH(
            XMVectorSet(0.f, 1.f, -3.f, 1.f),
            XMVectorSet(0.f, 1.f, 0.f, 1.f),
            XMVectorSet(0.f, 1.f, 0.f, 0.f)));

    XMStoreFloat4x4(&View.ViewToProjection,
        XMMatrixPerspectiveFovLH(
            XMConvertToRadians(60.f),
            ClientWidth / (float)ClientHeight,
            0.1f,
            100.f));

    return hr;
}

void GfxShutdown()
{
    BackBufferRT.Texture = nullptr;
    Renderer = nullptr;
    Presenter = nullptr;
    Context = nullptr;
    Device = nullptr;
    Adapter = nullptr;
    Factory = nullptr;
}
