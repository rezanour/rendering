#include "Precomp.h"
#include "CoreGraphics/GraphicsDevice.h"
#include "CoreGraphics/DxgiPresenter.h"
#include "CoreGraphics/Texture.h"
#include "CoreGraphics/RenderingCommon.h"
#include "CoreGraphics/VertexBuffer.h"
#include "CoreGraphics/VertexFormats.h"
#include "Renderers/LPFRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Visual.h"
#include "Scene/Light.h"
#include "AssetLoader.h"
#include "MathUtil.h"

using namespace Microsoft::WRL;

static const wchar_t AppClassName[] = L"RenderTest";
static const uint32_t ClientWidth = 1280;
static const uint32_t ClientHeight = 720;
static const float Fov = XMConvertToRadians(70.f);
static const float NearClip = 0.5f;
static const float FarClip = 10000.f;
static const float CameraMoveSpeed = 4.f;
static const float CameraTurnSpeed = 0.025f;
static const float MouseTurnSpeed = 0.005f;

static HWND AppWindow;
static ComPtr<IDXGIFactory2> Factory;
static std::shared_ptr<GraphicsDevice> Graphics;
static std::unique_ptr<BasePresenter> Presenter;
static std::unique_ptr<BaseRenderer> Renderer;
static std::shared_ptr<Scene> TheScene;
static std::shared_ptr<AssetLoader> Assets;
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

    // Timing info
    LARGE_INTEGER lastTime{};
    LARGE_INTEGER currTime{};
    LARGE_INTEGER frequency{};
    QueryPerformanceFrequency(&frequency);

    // TODO: Replace with something better as needed

    // Camera info
    XMVECTOR position = XMVectorSet(0.f, 10.f, 0.f, 1.f);
    XMVECTOR forward = XMVectorSet(1.f, 0.f, 0.f, 0.f);
    XMVECTOR right = XMVectorSet(0.f, 0.f, -1.f, 0.f);
    XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    float yaw = 0.f;
    float pitch = 0.f;
    POINT lastMousePos{};
    POINT curMousePos{};

    XMMATRIX projection = XMMatrixPerspectiveFovRH(
        Fov,
        ClientWidth / (float)ClientHeight,  // Aspect ratio of window client (rendering) area
        NearClip,
        FarClip);

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
            // Idle, measure time and produce a frame
            QueryPerformanceCounter(&currTime);
            if (lastTime.QuadPart == 0)
            {
                lastTime.QuadPart = currTime.QuadPart;
                continue;
            }

            // Compute time step from last frame until now
            double timeStep = (double)(currTime.QuadPart - lastTime.QuadPart) / (double)frequency.QuadPart;

            // Compute fps
            float frameRate = 1.0f / (float)timeStep;
            lastTime = currTime;

            UNREFERENCED_PARAMETER(frameRate);

            // Handle input
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                // Exit
                break;
            }

            XMVECTOR movement = XMVectorZero();
            if (GetAsyncKeyState('W') & 0x8000)
            {
                movement += forward;
            }
            if (GetAsyncKeyState('A') & 0x8000)
            {
                movement += -right;
            }
            if (GetAsyncKeyState('S') & 0x8000)
            {
                movement += -forward;
            }
            if (GetAsyncKeyState('D') & 0x8000)
            {
                movement += right;
            }

            position += XMVector3Normalize(movement) * CameraMoveSpeed;

            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                yaw -= CameraTurnSpeed;
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                yaw += CameraTurnSpeed;
            }
            if (GetAsyncKeyState(VK_UP) & 0x8000)
            {
                pitch -= CameraTurnSpeed;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                pitch += CameraTurnSpeed;
            }

            forward = XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMMatrixRotationY(yaw));
            right = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), forward);
            up = XMVector3TransformNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMMatrixRotationAxis(right, pitch));
            forward = XMVector3Cross(right, up);
            XMStoreFloat4x4(&View.WorldToView, XMMatrixLookToLH(position, forward, up));

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
        if (wParam == VK_F4)
        {
            bool enabled = Renderer->IsMsaaEnabled();
            HRESULT hr = Renderer->EnableMsaa(!enabled);
            CHECKHR(hr);
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

static void AddQuad(PositionNormalVertex* verts, XMVECTOR normal, XMVECTOR up, float d, float width, float height)
{
    XMStoreFloat3(&verts[0].Normal, normal);
    XMStoreFloat3(&verts[1].Normal, normal);
    XMStoreFloat3(&verts[2].Normal, normal);
    XMStoreFloat3(&verts[3].Normal, normal);
    XMStoreFloat3(&verts[4].Normal, normal);
    XMStoreFloat3(&verts[5].Normal, normal);

    width *= 0.5f;
    height *= 0.5f;

    XMVECTOR right = XMVector3Cross(normal, up);
    XMStoreFloat3(&verts[0].Position, right * -width + up * height + normal * d);
    XMStoreFloat3(&verts[1].Position, right * width + up * height + normal * d);
    XMStoreFloat3(&verts[2].Position, right * width + up * -height + normal * d);
    verts[3] = verts[0];
    verts[4] = verts[2];
    XMStoreFloat3(&verts[5].Position, right * -width + up * -height + normal * d);
}

HRESULT GfxInitialize()
{
    UINT dxgiFlags = 0;
#ifdef _DEBUG
    dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    HRESULT hr = CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&Factory));
    CHECKHR(hr);

    bool createDebug = false;
#ifdef _DEBUG
    createDebug = true;
#endif

    Graphics = std::make_shared<GraphicsDevice>();
    hr = Graphics->Initialize(Factory, nullptr, createDebug);
    CHECKHR(hr);

    Presenter = std::make_unique<DxgiPresenter>(Graphics, AppWindow);
    hr = Presenter->Initialize();
    CHECKHR(hr);

    Renderer = std::make_unique<LPFRenderer>(Graphics);
    hr = Renderer->Initialize();
    CHECKHR(hr);

    TheScene = std::make_shared<Scene>();

    std::vector<std::shared_ptr<Visual>> visuals;
    Assets = std::make_shared<AssetLoader>(Graphics, L"..\\ProcessedContent");
    hr = Assets->LoadModel(L"crytek-sponza\\sponza.model", &visuals);
    CHECKHR(hr);

    for (auto& visual : visuals)
    {
        TheScene->AddVisual(visual);
    }

    // Add some lights
    std::shared_ptr<Light> light = std::make_shared<DirectionalLight>();
    light->SetColor(XMFLOAT3(0.6f, 0.6f, 0.4f));
    light->SetOrientation(QuaternionFromViewDirection(XMVector3Normalize(XMVectorSet(-1, -1, -1, 0)), XMVectorSet(0, 1, 0, 0)));
    TheScene->AddLight(light);

    light = std::make_shared<DirectionalLight>();
    light->SetColor(XMFLOAT3(0.4f, 0.4f, 0.6f));
    light->SetOrientation(QuaternionFromViewDirection(XMVector3Normalize(XMVectorSet(1, -1, 1, 0)), XMVectorSet(0, 1, 0, 0)));
    TheScene->AddLight(light);

    Renderer->SetScene(TheScene);

    BackBufferRT.Texture = Presenter->GetBackBuffer();
    BackBufferRT.Viewport.Width = static_cast<float>(BackBufferRT.Texture->GetDesc().Width);
    BackBufferRT.Viewport.Height = static_cast<float>(BackBufferRT.Texture->GetDesc().Height);
    BackBufferRT.Viewport.MaxDepth = 1.f;

    XMStoreFloat4x4(&View.ViewToProjection,
        XMMatrixPerspectiveFovLH(
            XMConvertToRadians(60.f),
            ClientWidth / (float)ClientHeight,
            0.1f,
            10000.f));

    return hr;
}

void GfxShutdown()
{
    TheScene = nullptr;
    BackBufferRT.Texture = nullptr;
    Renderer = nullptr;
    Presenter = nullptr;
    Graphics = nullptr;
    Factory = nullptr;
}
