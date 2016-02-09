#include "Precomp.h"
#include "LPFRenderer.h"
#include "CoreGraphics/GraphicsDevice.h"
#include "CoreGraphics/Texture.h"
#include "CoreGraphics/RenderingCommon.h"
#include "CoreGraphics/VertexBuffer.h"
#include "CoreGraphics/IndexBuffer.h"
#include "CoreGraphics/ConstantBuffer.h"
#include "CoreGraphics/VertexFormats.h"
#include "CoreGraphics/ShaderPass.h"
#include "Scene/Scene.h"
#include "Scene/Visual.h"
#include "Scene/Light.h"

// shaders
#include "Shaders/GBuffer_vs.h"
#include "Shaders/GBuffer_ps.h"
#include "Shaders/DirectionalLight_vs.h"
#include "Shaders/DirectionalLight_ps.h"
#include "Shaders/DirectionalLight_MSAAx4_ps.h"
#include "Shaders/FinalPass_vs.h"
#include "Shaders/FinalPass_ps.h"
#include "Shaders/FinalPass_MSAAx4_ps.h"

LPFRenderer::LPFRenderer(const std::shared_ptr<GraphicsDevice>& graphics)
    : BaseRenderer(graphics, true)
{
    Context = graphics->GetContext();
}

LPFRenderer::~LPFRenderer()
{
}

HRESULT LPFRenderer::Initialize()
{
    HRESULT hr = Graphics->CreateShaderPassGraphics(VertexFormat::Basic3D, GBuffer_vs, sizeof(GBuffer_vs), GBuffer_ps, sizeof(GBuffer_ps), &GBufferPass);
    CHECKHR(hr);

    GBufferPass->SetPSSampler(0, Graphics->GetLinearWrapSampler());

    hr = Graphics->CreateShaderPassGraphics(VertexFormat::Position2D, DirectionalLight_vs, sizeof(DirectionalLight_vs), DirectionalLight_ps, sizeof(DirectionalLight_ps), &DLightPass);
    CHECKHR(hr);

    DLightPass->SetPSSampler(0, Graphics->GetLinearWrapSampler());

    hr = Graphics->CreateShaderPassGraphics(VertexFormat::Position2D, DirectionalLight_vs, sizeof(DirectionalLight_vs), DirectionalLight_MSAAx4_ps, sizeof(DirectionalLight_MSAAx4_ps), &DLightPassMsaa);
    CHECKHR(hr);

    DLightPassMsaa->SetPSSampler(0, Graphics->GetLinearWrapSampler());

    hr = Graphics->CreateShaderPassGraphics(VertexFormat::Basic3D, FinalPass_vs, sizeof(FinalPass_vs), FinalPass_ps, sizeof(FinalPass_ps), &FinalPass);
    CHECKHR(hr);

    FinalPass->SetPSSampler(0, Graphics->GetLinearWrapSampler());

    hr = Graphics->CreateShaderPassGraphics(VertexFormat::Basic3D, FinalPass_vs, sizeof(FinalPass_vs), FinalPass_MSAAx4_ps, sizeof(FinalPass_MSAAx4_ps), &FinalPassMsaa);
    CHECKHR(hr);

    FinalPassMsaa->SetPSSampler(0, Graphics->GetLinearWrapSampler());

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(GBufferVSConstants), &GBufferVSCB);
    CHECKHR(hr);

    GBufferPass->SetVSConstantBuffer(0, GBufferVSCB->GetCB());
    FinalPass->SetVSConstantBuffer(0, GBufferVSCB->GetCB());
    FinalPassMsaa->SetVSConstantBuffer(0, GBufferVSCB->GetCB());

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(DLightVSConstants), &DLightVSCB);
    CHECKHR(hr);

    DLightPass->SetVSConstantBuffer(0, DLightVSCB->GetCB());
    DLightPassMsaa->SetVSConstantBuffer(0, DLightVSCB->GetCB());

    hr = Graphics->CreateConstantBuffer(nullptr, sizeof(DLightPSConstants), &DLightPSCB);
    CHECKHR(hr);

    DLightPass->SetPSConstantBuffer(0, DLightPSCB->GetCB());
    DLightPassMsaa->SetPSConstantBuffer(0, DLightPSCB->GetCB());

    GBufferPass->SetDepthState(Graphics->GetDepthWriteState());
    FinalPass->SetDepthState(Graphics->GetDepthReadState());
    FinalPassMsaa->SetDepthState(Graphics->GetDepthReadState());

    // Quad
    Position2DVertex verts[]
    {
        { XMFLOAT2(-1, 1), },
        { XMFLOAT2(1, 1), },
        { XMFLOAT2(1, -1), },
        { XMFLOAT2(-1, -1), },
    };

    uint32_t indices[]
    {
        0, 1, 2, 0, 2, 3,
    };

    std::shared_ptr<VertexBuffer> vb;
    hr = Graphics->CreateVertexBuffer(VertexFormat::Position2D, verts, sizeof(verts), &vb);
    CHECKHR(hr);

    std::shared_ptr<IndexBuffer> ib;
    hr = Graphics->CreateIndexBuffer(indices, sizeof(indices), &ib);
    CHECKHR(hr);

    QuadVisual = std::make_shared<Visual>();
    hr = QuadVisual->Initialize(vb, ib, 0, _countof(indices));
    CHECKHR(hr);

    return hr;
}

HRESULT LPFRenderer::RenderFrame(const RenderTarget& renderTarget, const RenderView& view)
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

    RenderGBuffer(view);
    RenderLights(view);
    RenderFinal(view, renderTarget);

    return S_OK;
}

void LPFRenderer::RenderGBuffer(const RenderView& view)
{
    static const float ViewNormalsClear[] = { 0.f, 0.f, 0.f, 1.f };
    static const float LinearDepthClear[] = { 1.f, 1.f, 1.f, 1.f };
    Context->ClearRenderTargetView(GBufferViewNormalsRT->GetRTV().Get(), ViewNormalsClear);
    Context->ClearRenderTargetView(GBufferLinearDepthRT->GetRTV().Get(), LinearDepthClear);
    Context->ClearDepthStencilView(GBufferDepthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH, 1.f, 0);

    GBufferPass->Begin();

    GBufferVSConstants constants{};
    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        XMStoreFloat4x4(&constants.LocalToView, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), XMLoadFloat4x4(&view.WorldToView)));
        XMStoreFloat4x4(&constants.LocalToProjection, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), worldToProjection));

        HRESULT hr = GBufferVSCB->Update(&constants, sizeof(constants));
        if (FAILED(hr))
        {
            assert(false);
            continue;
        }

        ComPtr<ID3D11ShaderResourceView> srv = visual->GetNormalTexture() ? visual->GetNormalTexture()->GetSRV().Get() : nullptr;
        GBufferPass->SetPSResource(0, srv);

        GBufferPass->Draw(visual);
    }

    GBufferPass->End();
}

void LPFRenderer::RenderLights(const RenderView& view)
{
    UNREFERENCED_PARAMETER(view);

    static const float ClearColor[] = { 0.f, 0.f, 0.f, 0.f };
    Context->ClearRenderTargetView(LightRT->GetRTV().Get(), ClearColor);

    auto& pass = (MsaaEnabled) ? DLightPassMsaa : DLightPass;

    XMMATRIX worldToView = XMLoadFloat4x4(&view.WorldToView);

    pass->Begin();

    DLightVSConstants vsConstants{};
    vsConstants.ClipDistance = 100.f;

    DLightVSCB->Update(&vsConstants, sizeof(vsConstants));

    DLightPSConstants psConstants{};
    psConstants.NumLights = 0;

    for (auto& light : Lights)
    {
        if (light->GetType() == LightType::Directional)
        {
            psConstants.Lights[psConstants.NumLights].Color = light->GetColor();

            XMFLOAT4X4 localToWorld = light->GetLocalToWorld();
            XMVECTOR lightDir = XMVectorNegate(XMVectorSet(localToWorld.m[2][0], localToWorld.m[2][1], localToWorld.m[2][2], 0.f));
            lightDir = XMVector3TransformNormal(lightDir, worldToView);
            XMStoreFloat3(&psConstants.Lights[psConstants.NumLights].Direction, lightDir);

            ++psConstants.NumLights;
        }
    }

    DLightPSCB->Update(&psConstants, sizeof(psConstants));

    pass->Draw(QuadVisual);

    pass->End();
}

void LPFRenderer::RenderFinal(const RenderView& view, const RenderTarget& renderTarget)
{
    static const float ClearColor[] = { 0.f, 0.f, 0.f, 1.f };
    Context->ClearRenderTargetView(renderTarget.Texture->GetRTV().Get(), ClearColor);

    auto& pass = (MsaaEnabled) ? FinalPassMsaa : FinalPass;

    if (!MsaaEnabled)
    {
        pass->SetRenderTarget(0, renderTarget.Texture->GetRTV());
    }

    pass->Begin();

    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));
    GBufferVSConstants constants{};

    for (auto& visual : Visuals)
    {
        if (!visual->GetAlbedoTexture())
        {
            continue;
        }
        pass->SetPSResource(1, visual->GetAlbedoTexture()->GetSRV());

        XMStoreFloat4x4(&constants.LocalToView, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), XMLoadFloat4x4(&view.WorldToView)));
        XMStoreFloat4x4(&constants.LocalToProjection, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), worldToProjection));

        GBufferVSCB->Update(&constants, sizeof(constants));

        pass->Draw(visual);
    }

    pass->End();
    if (MsaaEnabled)
    {
        Context->ResolveSubresource(renderTarget.Texture->GetTexture().Get(), 0, FinalRTMsaa->GetTexture().Get(), 0, renderTarget.Texture->GetDesc().Format);
    }
}

HRESULT LPFRenderer::RecreateSurfaces(uint32_t width, uint32_t height, uint32_t sampleCount)
{
    GBufferViewNormalsRT = nullptr;
    GBufferLinearDepthRT = nullptr;
    GBufferDepthBuffer = nullptr;
    LightRT = nullptr;
    FinalRTMsaa = nullptr;

    D3D11_TEXTURE2D_DESC desc{};
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = sampleCount;
    desc.Usage = D3D11_USAGE_DEFAULT;

    Viewport.Width = static_cast<float>(desc.Width);
    Viewport.Height = static_cast<float>(desc.Height);
    Viewport.MaxDepth = 1.f;

    HRESULT hr = Graphics->CreateTexture2D(desc, &GBufferViewNormalsRT);
    CHECKHR(hr);

    hr = Graphics->CreateTexture2D(desc, &LightRT);
    CHECKHR(hr);

    if (sampleCount > 1)
    {
        hr = Graphics->CreateTexture2D(desc, &FinalRTMsaa);
        CHECKHR(hr);
    }

    desc.Format = DXGI_FORMAT_R32_FLOAT;
    hr = Graphics->CreateTexture2D(desc, &GBufferLinearDepthRT);
    CHECKHR(hr);

    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    hr = Graphics->CreateTexture2D(desc, &GBufferDepthBuffer);
    CHECKHR(hr);

    RTWidth = width;
    RTHeight = height;

    GBufferPass->SetRenderTarget(0, GBufferViewNormalsRT->GetRTV());
    GBufferPass->SetRenderTarget(1, GBufferLinearDepthRT->GetRTV());
    GBufferPass->SetDepthBuffer(GBufferDepthBuffer->GetDSV());
    DLightPass->SetPSResource(0, GBufferViewNormalsRT->GetSRV());
    DLightPass->SetPSResource(1, GBufferLinearDepthRT->GetSRV());
    DLightPass->SetRenderTarget(0, LightRT->GetRTV());
    DLightPassMsaa->SetPSResource(0, GBufferViewNormalsRT->GetSRV());
    DLightPassMsaa->SetPSResource(1, GBufferLinearDepthRT->GetSRV());
    DLightPassMsaa->SetRenderTarget(0, LightRT->GetRTV());
    FinalPass->SetPSResource(0, LightRT->GetSRV());
    FinalPass->SetDepthBuffer(GBufferDepthBuffer->GetDSV());
    FinalPassMsaa->SetPSResource(0, LightRT->GetSRV());
    FinalPassMsaa->SetDepthBuffer(GBufferDepthBuffer->GetDSV());
    FinalPassMsaa->SetRenderTarget(0, FinalRTMsaa ? FinalRTMsaa->GetRTV() : nullptr);

    return hr;
}

