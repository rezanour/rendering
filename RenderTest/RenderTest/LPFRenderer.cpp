#include "Precomp.h"
#include "LPFRenderer.h"

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

HRESULT LPFRenderer::BeginFrame(const std::shared_ptr<RenderTarget>& renderTarget, const RenderView& view)
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(view);

    FinalRenderTarget = renderTarget;

    if (MsaaEnabled)
    {
        hr = EnsureMsaaRenderTarget();
        if (FAILED(hr))
        {
            assert(false);
            return hr;
        }

        MsaaRenderTarget->Viewport = FinalRenderTarget->Viewport;
        CurrentRenderTarget = MsaaRenderTarget;
    }
    else
    {
        CurrentRenderTarget = FinalRenderTarget;
    }

    static const float clearColor[] = { 0.f, 0.f, 0.5f, 1.f };
    Context->ClearRenderTargetView(CurrentRenderTarget->RTV.Get(), clearColor);
    Context->OMSetRenderTargets(1, CurrentRenderTarget->RTV.GetAddressOf(), nullptr);
    Context->RSSetViewports(1, &CurrentRenderTarget->Viewport);

    return S_OK;
}

HRESULT LPFRenderer::EndFrame()
{
    if (MsaaEnabled)
    {
        Context->ResolveSubresource(FinalRenderTarget->Texture.Get(), 0, MsaaRenderTarget->Texture.Get(), 0, FinalRenderTarget->Desc.Format);
    }

    // Don't hold a reference to final output buffer past EndFrame.
    FinalRenderTarget = nullptr;

    return S_OK;
}

HRESULT LPFRenderer::EnsureMsaaRenderTarget()
{
    bool needToCreate = true;
    if (MsaaRenderTarget != nullptr)
    {
        if (MsaaRenderTarget->Desc.Width == FinalRenderTarget->Desc.Width &&
            MsaaRenderTarget->Desc.Height == FinalRenderTarget->Desc.Height &&
            MsaaRenderTarget->Desc.Format == FinalRenderTarget->Desc.Format)
        {
            needToCreate = false;
        }
    }

    if (!needToCreate)
    {
        return S_OK;
    }

    MsaaRenderTarget = std::make_shared<RenderTarget>();
    MsaaRenderTarget->Desc = FinalRenderTarget->Desc;
    MsaaRenderTarget->Desc.SampleDesc.Count = 4;

    HRESULT hr = Device->CreateTexture2D(&MsaaRenderTarget->Desc, nullptr, &MsaaRenderTarget->Texture);
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    hr = Device->CreateRenderTargetView(MsaaRenderTarget->Texture.Get(), nullptr, &MsaaRenderTarget->RTV);
    if (FAILED(hr))
    {
        assert(false);
        return hr;
    }

    return hr;
}
