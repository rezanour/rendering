#include "Precomp.h"
#include "ForwardPlusRenderer.h"
#include "CoreGraphics/GraphicsDevice.h"
#include "CoreGraphics/ShaderPass.h"
#include "CoreGraphics/ConstantBuffer.h"
#include "CoreGraphics/Buffer.h"
#include "CoreGraphics/Texture.h"
#include "Scene/Scene.h"
#include "Scene/Visual.h"
#include "Scene/Light.h"

// Shaders
#include "Shaders/ZPrePass_vs.h"
#include "Shaders/FP_LightCull_cs.h"
#include "Shaders/FP_FinalPass_vs.h"
#include "Shaders/FP_FinalPass_ps.h"

ForwardPlusRenderer::ForwardPlusRenderer(const std::shared_ptr<GraphicsDevice>& graphics)
    : BaseRenderer(graphics, true)
{
    Context = graphics->GetContext();
}

ForwardPlusRenderer::~ForwardPlusRenderer()
{
}

HRESULT ForwardPlusRenderer::Initialize()
{
    // Z PrePass
    HRESULT hr = Graphics->CreateShaderPassGraphics(VertexFormat::Basic3D, ZPrePass_vs, sizeof(ZPrePass_vs), nullptr, 0, &ZPrePass);
    CHECKHR(hr);

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(ZPrePassVSConstants), &ZPrePassVSCB);
    CHECKHR(hr);

    ZPrePass->SetVSConstantBuffer(0, ZPrePassVSCB->GetCB());
    ZPrePass->SetDepthState(Graphics->GetDepthWriteState());

    // Light culling
    hr = Graphics->CreateShaderPassCompute(FP_LightCull_cs, sizeof(FP_LightCull_cs), &LightCullPass);
    CHECKHR(hr);

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(LightCullConstants), &LightCullCB);
    CHECKHR(hr);

    LightLinkedListNodes = std::make_shared<Buffer>();
    hr = LightLinkedListNodes->Initialize(Graphics->GetDevice(), sizeof(LightLinkedListNode), sizeof(LightLinkedListNode) * LinkedListMaxElements, true, true);
    CHECKHR(hr);

    // Final pass
    hr = Graphics->CreateShaderPassGraphics(VertexFormat::Basic3D, FP_FinalPass_vs, sizeof(FP_FinalPass_vs), FP_FinalPass_ps, sizeof(FP_FinalPass_ps), &FinalPass);
    CHECKHR(hr);

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(FinalPassVSConstants), &FinalPassVSCB);
    CHECKHR(hr);

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(FinalPassPSDLightConstants), &FinalPassPSDLightCB);
    CHECKHR(hr);

    FinalPass->SetVSConstantBuffer(0, FinalPassVSCB->GetCB());
    FinalPass->SetPSConstantBuffer(0, FinalPassPSDLightCB->GetCB());
    FinalPass->SetPSSampler(0, Graphics->GetAnisoWrapSampler());
    FinalPass->SetDepthState(Graphics->GetDepthReadState());

    hr = Graphics->CreateShaderPassGraphics(VertexFormat::Basic3D, FP_FinalPass_vs, sizeof(FP_FinalPass_vs), FP_FinalPass_ps, sizeof(FP_FinalPass_ps), &FinalPassMsaa);
    CHECKHR(hr);

    FinalPassMsaa->SetVSConstantBuffer(0, FinalPassVSCB->GetCB());
    FinalPassMsaa->SetPSConstantBuffer(0, FinalPassPSDLightCB->GetCB());
    FinalPassMsaa->SetPSSampler(0, Graphics->GetAnisoWrapSampler());
    FinalPassMsaa->SetDepthState(Graphics->GetDepthReadState());

    return hr;
}

HRESULT ForwardPlusRenderer::RenderFrame(const RenderTarget& renderTarget, const RenderView& view)
{
    HRESULT hr = S_OK;

    if (RTWidth != renderTarget.Texture->GetDesc().Width ||
        RTHeight != renderTarget.Texture->GetDesc().Height)
    {
        hr = RecreateSurfaces(renderTarget.Texture->GetDesc().Width, renderTarget.Texture->GetDesc().Height, MsaaEnabled ? 4 : 1);
        CHECKHR(hr);
    }

    // Scene traversal once, used for multiple passes
    Scene->GetVisibleVisuals(view, &Visuals);
    Scene->GetVisibleLights(view, &Lights);

    RenderZPrePass(view);
    CullLights(view);
    RenderFinal(view, renderTarget);

    return S_OK;
}

void ForwardPlusRenderer::RenderZPrePass(const RenderView& view)
{
    Context->ClearDepthStencilView(DepthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH, 1.f, 0);

    ZPrePass->Begin();

    ZPrePassVSConstants constants{};
    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        XMStoreFloat4x4(&constants.LocalToProjection, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), worldToProjection));

        HRESULT hr = ZPrePassVSCB->Update(&constants, sizeof(constants));
        if (FAILED(hr))
        {
            assert(false);
            continue;
        }

        ZPrePass->Draw(visual);
    }

    ZPrePass->End();
}

void ForwardPlusRenderer::CullLights(const RenderView& view)
{
    UNREFERENCED_PARAMETER(view);
}

void ForwardPlusRenderer::RenderFinal(const RenderView& view, const RenderTarget& renderTarget)
{
    UNREFERENCED_PARAMETER(view);

    auto& pass = (MsaaEnabled) ? FinalPassMsaa : FinalPass;

    if (!MsaaEnabled)
    {
        pass->SetRenderTarget(0, renderTarget.Texture->GetRTV());
    }

    pass->Begin();

    XMMATRIX worldToView = XMLoadFloat4x4(&view.WorldToView);
    XMMATRIX worldToProjection = XMMatrixMultiply(worldToView, XMLoadFloat4x4(&view.ViewToProjection));

    // Directional Lights are shared for all objects, so fill up front
    FinalPassPSDLightConstants dlightConstants{};
    dlightConstants.NumLights = 0;

    for (auto& light : Lights)
    {
        if (light->GetType() == LightType::Directional)
        {
            dlightConstants.Lights[dlightConstants.NumLights].Color = light->GetColor();

            XMFLOAT4X4 localToWorld = light->GetLocalToWorld();
            XMVECTOR lightDir = XMVectorNegate(XMVectorSet(localToWorld.m[2][0], localToWorld.m[2][1], localToWorld.m[2][2], 0.f));
            lightDir = XMVector3TransformNormal(lightDir, worldToView);
            XMStoreFloat3(&dlightConstants.Lights[dlightConstants.NumLights].Direction, lightDir);

            ++dlightConstants.NumLights;
        }
    }

    FinalPassPSDLightCB->Update(&dlightConstants, sizeof(dlightConstants));

    // Remainder of data is object-specific
    FinalPassVSConstants constants{};

    for (auto& visual : Visuals)
    {
        XMMATRIX localToWorld = XMLoadFloat4x4(&visual->GetLocalToWorld());
        XMStoreFloat4x4(&constants.LocalToView, XMMatrixMultiply(localToWorld, worldToView));
        XMStoreFloat4x4(&constants.LocalToProjection, XMMatrixMultiply(localToWorld, worldToProjection));

        HRESULT hr = FinalPassVSCB->Update(&constants, sizeof(constants));
        if (FAILED(hr))
        {
            assert(false);
            continue;
        }

        pass->SetPSResource(0, visual->GetAlbedoTexture() ? visual->GetAlbedoTexture()->GetSRV() : nullptr);
        pass->SetPSResource(1, visual->GetNormalTexture() ? visual->GetNormalTexture()->GetSRV() : nullptr);

        pass->Draw(visual);
    }

    pass->End();

    if (MsaaEnabled)
    {
        Context->ResolveSubresource(renderTarget.Texture->GetTexture().Get(), 0, FinalRTMsaa->GetTexture().Get(), 0, FinalRTMsaa->GetDesc().Format);
    }
}

HRESULT ForwardPlusRenderer::RecreateSurfaces(uint32_t width, uint32_t height, uint32_t sampleCount)
{
    DepthBuffer = nullptr;
    FinalRTMsaa = nullptr;

    D3D11_TEXTURE2D_DESC desc{};
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = sampleCount;
    desc.Usage = D3D11_USAGE_DEFAULT;

    Viewport.Width = static_cast<float>(desc.Width);
    Viewport.Height = static_cast<float>(desc.Height);
    Viewport.MaxDepth = 1.f;

    HRESULT hr = S_OK;

    if (MsaaEnabled)
    {
        hr = Graphics->CreateTexture2D(desc, &FinalRTMsaa);
        CHECKHR(hr);
    }

    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    //desc.Format = DXGI_FORMAT_D32_FLOAT;
    //hr = Graphics->CreateTexture2D(desc, &DepthBuffer);
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    hr = Graphics->CreateTexture2D(desc, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT, &DepthBuffer);
    CHECKHR(hr);

    RTWidth = width;
    RTHeight = height;

    LightLinkedListHeads = std::make_shared<Buffer>();
    hr = LightLinkedListHeads->Initialize(Graphics->GetDevice(), sizeof(uint32_t), sizeof(uint32_t) * RTWidth * RTHeight, false, false);
    CHECKHR(hr);

    ZPrePass->SetViewport(&Viewport);
    ZPrePass->SetDepthBuffer(DepthBuffer->GetDSV());

    FinalPass->SetViewport(&Viewport);
    FinalPass->SetDepthBuffer(DepthBuffer->GetDSV());

    FinalPassMsaa->SetViewport(&Viewport);
    FinalPassMsaa->SetDepthBuffer(DepthBuffer->GetDSV());
    FinalPassMsaa->SetRenderTarget(0, FinalRTMsaa ? FinalRTMsaa->GetRTV() : nullptr);

    return hr;
}
