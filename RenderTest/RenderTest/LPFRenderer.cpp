#include "Precomp.h"
#include "LPFRenderer.h"
#include "RenderTarget.h"
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

HRESULT LPFRenderer::RenderFrame(const std::shared_ptr<RenderTarget>& renderTarget, const RenderView& view)
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(view);

    if (MsaaEnabled)
    {
        hr = EnsureMsaaRenderTarget(renderTarget);
        CHECKHR(hr);

        MsaaRenderTarget->SetViewport(renderTarget->GetViewport());
        CurrentRenderTarget = MsaaRenderTarget;
    }
    else
    {
        CurrentRenderTarget = renderTarget;
    }

    static const float clearColor[] = { 0.f, 0.f, 0.5f, 1.f };
    Context->ClearRenderTargetView(CurrentRenderTarget->GetRTV().Get(), clearColor);
    Context->OMSetRenderTargets(1, CurrentRenderTarget->GetRTV().GetAddressOf(), nullptr);
    Context->RSSetViewports(1, &CurrentRenderTarget->GetViewport());

    // TODO: Render scene

    if (MsaaEnabled)
    {
        Context->ResolveSubresource(renderTarget->GetTexture().Get(), 0, MsaaRenderTarget->GetTexture().Get(), 0, renderTarget->GetDesc().Format);
    }

    return S_OK;
}

HRESULT LPFRenderer::EnsureMsaaRenderTarget(const std::shared_ptr<RenderTarget>& finalRenderTarget)
{
    bool needToCreate = true;
    if (MsaaRenderTarget != nullptr)
    {
        if (MsaaRenderTarget->GetDesc().Width == finalRenderTarget->GetDesc().Width &&
            MsaaRenderTarget->GetDesc().Height == finalRenderTarget->GetDesc().Height &&
            MsaaRenderTarget->GetDesc().Format == finalRenderTarget->GetDesc().Format)
        {
            needToCreate = false;
        }
    }

    if (!needToCreate)
    {
        return S_OK;
    }

    D3D11_TEXTURE2D_DESC desc = finalRenderTarget->GetDesc();
    desc.SampleDesc.Count = 4;

    MsaaRenderTarget = std::make_shared<RenderTarget>();
    HRESULT hr = MsaaRenderTarget->Initialize(Device.Get(), desc);
    CHECKHR(hr);

    return hr;
}
