#include "Precomp.h"
#include "LPFRenderer.h"
#include "Texture.h"
#include "RenderingCommon.h"

LPFRenderer::LPFRenderer(ID3D11Device* device)
    : BaseRenderer(device)
    , MsaaEnabled(true)
{
    device->GetImmediateContext(&Context);
}

LPFRenderer::~LPFRenderer()
{
}

HRESULT LPFRenderer::Initialize()
{
    return S_OK;
}

HRESULT LPFRenderer::RenderFrame(const RenderTarget& renderTarget, const RenderView& view)
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(view);

    if (MsaaEnabled)
    {
        hr = EnsureMsaaRenderTarget(renderTarget.Texture);
        CHECKHR(hr);

        MsaaRenderTarget.Viewport = renderTarget.Viewport;
        CurrentRenderTarget = &MsaaRenderTarget;
        CurrentDepthBuffer = MsaaDepthBuffer;
    }
    else
    {
        hr = EnsureDepthBuffer(renderTarget.Texture);
        CHECKHR(hr);

        CurrentRenderTarget = &renderTarget;
        CurrentDepthBuffer = DepthBuffer;
    }

    static const float clearColor[] = { 0.f, 0.f, 0.5f, 1.f };
    Context->ClearRenderTargetView(CurrentRenderTarget->Texture->GetRTV().Get(), clearColor);
    Context->OMSetRenderTargets(1, CurrentRenderTarget->Texture->GetRTV().GetAddressOf(), CurrentDepthBuffer->GetDSV().Get());
    Context->RSSetViewports(1, &CurrentRenderTarget->Viewport);

    // TODO: Render scene

    if (MsaaEnabled)
    {
        Context->ResolveSubresource(renderTarget.Texture->GetTexture().Get(), 0, MsaaRenderTarget.Texture->GetTexture().Get(), 0, renderTarget.Texture->GetDesc().Format);
    }

    return S_OK;
}

HRESULT LPFRenderer::EnsureMsaaRenderTarget(const std::shared_ptr<Texture2D>& resolveTarget)
{
    bool needToCreate = true;
    if (MsaaRenderTarget.Texture != nullptr)
    {
        if (MsaaRenderTarget.Texture->GetDesc().Width == resolveTarget->GetDesc().Width &&
            MsaaRenderTarget.Texture->GetDesc().Height == resolveTarget->GetDesc().Height &&
            MsaaRenderTarget.Texture->GetDesc().Format == resolveTarget->GetDesc().Format)
        {
            needToCreate = false;
        }
    }

    if (!needToCreate)
    {
        return S_OK;
    }

    MsaaRenderTarget.Texture = nullptr;
    MsaaDepthBuffer = nullptr;

    D3D11_TEXTURE2D_DESC desc = resolveTarget->GetDesc();
    desc.SampleDesc.Count = 4;

    MsaaRenderTarget.Texture = std::make_shared<Texture2D>();
    HRESULT hr = MsaaRenderTarget.Texture->Initialize(Device, desc);
    CHECKHR(hr);

    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.Format = DXGI_FORMAT_D32_FLOAT;

    MsaaDepthBuffer = std::make_shared<Texture2D>();
    hr = MsaaDepthBuffer->Initialize(Device, desc);
    CHECKHR(hr);

    return hr;
}

HRESULT LPFRenderer::EnsureDepthBuffer(const std::shared_ptr<Texture2D>& renderTarget)
{
    bool needToCreate = true;
    if (DepthBuffer != nullptr)
    {
        if (DepthBuffer->GetDesc().Width == renderTarget->GetDesc().Width &&
            DepthBuffer->GetDesc().Height == renderTarget->GetDesc().Height)
        {
            needToCreate = false;
        }
    }

    if (!needToCreate)
    {
        return S_OK;
    }

    DepthBuffer = nullptr;

    D3D11_TEXTURE2D_DESC desc = renderTarget->GetDesc();
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.Format = DXGI_FORMAT_D32_FLOAT;

    DepthBuffer = std::make_shared<Texture2D>();
    HRESULT hr = DepthBuffer->Initialize(Device, desc);
    CHECKHR(hr);

    return hr;
}
