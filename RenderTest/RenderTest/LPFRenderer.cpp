#include "Precomp.h"
#include "LPFRenderer.h"
#include "Texture.h"
#include "RenderingCommon.h"
#include "RenderScene.h"
#include "RenderVisual.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexFormats.h"
#include "ShaderPass.h"

// shaders
#include "GBuffer_vs.h"
#include "GBuffer_ps.h"
#include "DirectionalLight_vs.h"
#include "DirectionalLight_ps.h"
#include "DirectionalLight_MSAAx4_ps.h"
#include "FinalPass_vs.h"
#include "FinalPass_ps.h"
#include "FinalPass_MSAAx4_ps.h"

LPFRenderer::LPFRenderer(const ComPtr<ID3D11Device>& device)
    : BaseRenderer(device)
{
    device->GetImmediateContext(&Context);
}

LPFRenderer::~LPFRenderer()
{
}

HRESULT LPFRenderer::Initialize()
{
    D3D11_SAMPLER_DESC sd{};
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = Device->CreateSamplerState(&sd, &LinearSampler);
    CHECKHR(hr);

    // GBuffer Pass
    GBufferPass = std::make_shared<ShaderPass>();
    hr = GBufferPass->InitializeGraphics(Device, VertexFormat::Basic3D, GBuffer_vs, sizeof(GBuffer_vs), GBuffer_ps, sizeof(GBuffer_ps));
    CHECKHR(hr);

    GBufferPass->SetPSSampler(0, LinearSampler);

    DLightPass = std::make_shared<ShaderPass>();
    hr = DLightPass->InitializeGraphics(Device, VertexFormat::Position2D, DirectionalLight_vs, sizeof(DirectionalLight_vs), DirectionalLight_ps, sizeof(DirectionalLight_ps));
    CHECKHR(hr);

    DLightPass->SetPSSampler(0, LinearSampler);

    DLightPassMsaa = std::make_shared<ShaderPass>();
    hr = DLightPassMsaa->InitializeGraphics(Device, VertexFormat::Position2D, DirectionalLight_vs, sizeof(DirectionalLight_vs), DirectionalLight_MSAAx4_ps, sizeof(DirectionalLight_MSAAx4_ps));
    CHECKHR(hr);

    DLightPassMsaa->SetPSSampler(0, LinearSampler);

    FinalPass = std::make_shared<ShaderPass>();
    hr = FinalPass->InitializeGraphics(Device, VertexFormat::Basic3D, FinalPass_vs, sizeof(FinalPass_vs), FinalPass_ps, sizeof(FinalPass_ps));
    CHECKHR(hr);

    FinalPass->SetPSSampler(0, LinearSampler);

    FinalPassMsaa = std::make_shared<ShaderPass>();
    hr = FinalPassMsaa->InitializeGraphics(Device, VertexFormat::Basic3D, FinalPass_vs, sizeof(FinalPass_vs), FinalPass_MSAAx4_ps, sizeof(FinalPass_MSAAx4_ps));
    CHECKHR(hr);

    FinalPassMsaa->SetPSSampler(0, LinearSampler);

    D3D11_BUFFER_DESC bd{};
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.ByteWidth = sizeof(GBufferVSConstants);
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.StructureByteStride = bd.ByteWidth;
    bd.Usage = D3D11_USAGE_DYNAMIC;

    hr = Device->CreateBuffer(&bd, nullptr, &GBufferVSCB);
    CHECKHR(hr);

    bd.ByteWidth = sizeof(DLightVSConstants);
    bd.StructureByteStride = bd.ByteWidth;

    hr = Device->CreateBuffer(&bd, nullptr, &DLightVSCB);
    CHECKHR(hr);

    bd.ByteWidth = sizeof(DLightPSConstants);
    bd.StructureByteStride = bd.ByteWidth;

    hr = Device->CreateBuffer(&bd, nullptr, &DLightPSCB);
    CHECKHR(hr);

    D3D11_DEPTH_STENCIL_DESC dsDesc{};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    hr = Device->CreateDepthStencilState(&dsDesc, &DepthWriteState);
    CHECKHR(hr);

    GBufferPass->SetDepthState(DepthWriteState);

    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    hr = Device->CreateDepthStencilState(&dsDesc, &DepthReadState);
    CHECKHR(hr);

    FinalPass->SetDepthState(DepthReadState);
    FinalPassMsaa->SetDepthState(DepthReadState);

    // Quad
    Position2DVertex verts[]
    {
        { XMFLOAT2(-1, 1), },
        { XMFLOAT2(1, 1), },
        { XMFLOAT2(1, -1), },
        { XMFLOAT2(-1, 1), },
        { XMFLOAT2(1, -1), },
        { XMFLOAT2(-1, -1), },
    };

    QuadVB = std::make_shared<VertexBuffer>();
    hr = QuadVB->Initialize(Device, VertexFormat::Position2D, verts, sizeof(verts));
    CHECKHR(hr);

    hr = RecreateSurfaces(MsaaEnabled ? 4 : 1);
    CHECKHR(hr);

    return hr;
}

HRESULT LPFRenderer::RenderFrame(const RenderTarget& renderTarget, const RenderView& view)
{
    // Scene traversal once, used for multiple passes
    Scene->GetVisibleVisuals(view, &Visuals);

    RenderGBuffer(view);
    RenderLights(view);
    RenderFinal(view, renderTarget);

    return S_OK;
}

HRESULT LPFRenderer::RecreateSurfaces(uint32_t sampleCount)
{
    GBufferViewNormalsRT = nullptr;
    GBufferLinearDepthRT = nullptr;
    GBufferDepthBuffer = nullptr;
    LightRT = nullptr;
    FinalRT = nullptr;

    D3D11_TEXTURE2D_DESC desc{};
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Width = 1280;
    desc.Height = 720;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = sampleCount;
    desc.Usage = D3D11_USAGE_DEFAULT;

    Viewport.Width = static_cast<float>(desc.Width);
    Viewport.Height = static_cast<float>(desc.Height);
    Viewport.MaxDepth = 1.f;

    GBufferViewNormalsRT = std::make_shared<Texture2D>();
    HRESULT hr = GBufferViewNormalsRT->Initialize(Device, desc);
    CHECKHR(hr);

    LightRT = std::make_shared<Texture2D>();
    hr = LightRT->Initialize(Device, desc);
    CHECKHR(hr);

    if (sampleCount > 1)
    {
        FinalRT = std::make_shared<Texture2D>();
        hr = FinalRT->Initialize(Device, desc);
        CHECKHR(hr);
    }

    desc.Format = DXGI_FORMAT_R32_FLOAT;
    GBufferLinearDepthRT = std::make_shared<Texture2D>();
    hr = GBufferLinearDepthRT->Initialize(Device, desc);
    CHECKHR(hr);

    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    GBufferDepthBuffer = std::make_shared<Texture2D>();
    hr = GBufferDepthBuffer->Initialize(Device, desc);
    CHECKHR(hr);

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
    FinalPass->SetRenderTarget(0, FinalRT->GetRTV());
    FinalPass->SetDepthBuffer(GBufferDepthBuffer->GetDSV());
    FinalPassMsaa->SetPSResource(0, LightRT->GetSRV());
    FinalPassMsaa->SetRenderTarget(0, FinalRT->GetRTV());
    FinalPassMsaa->SetDepthBuffer(GBufferDepthBuffer->GetDSV());

    return hr;
}

void LPFRenderer::RenderGBuffer(const RenderView& view)
{
    static const float ViewNormalsClear[] = { 0.f, 0.f, 0.f, 1.f };
    static const float LinearDepthClear[] = { 1.f, 1.f, 1.f, 1.f };
    Context->ClearRenderTargetView(GBufferViewNormalsRT->GetRTV().Get(), ViewNormalsClear);
    Context->ClearRenderTargetView(GBufferLinearDepthRT->GetRTV().Get(), LinearDepthClear);
    Context->ClearDepthStencilView(GBufferDepthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH, 1.f, 0);

    GBufferPass->Begin();

    Context->VSSetConstantBuffers(0, 1, GBufferVSCB.GetAddressOf());

    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        ID3D11ShaderResourceView* normalMap = visual->GetNormalTexture() ? visual->GetNormalTexture()->GetSRV().Get() : nullptr;
        Context->PSSetShaderResources(0, 1, &normalMap);

        D3D11_MAPPED_SUBRESOURCE mapped{};
        HRESULT hr = Context->Map(GBufferVSCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr))
        {
            assert(false);
            continue;
        }
        GBufferVSConstants* constants = (GBufferVSConstants*)mapped.pData;
        XMStoreFloat4x4(&constants->LocalToView, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), XMLoadFloat4x4(&view.WorldToView)));
        XMStoreFloat4x4(&constants->LocalToProjection, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), worldToProjection));
        Context->Unmap(GBufferVSCB.Get(), 0);

        GBufferPass->Draw(visual);
    }

    GBufferPass->End();
}

void LPFRenderer::RenderLights(const RenderView& view)
{
    UNREFERENCED_PARAMETER(view);

    static const float ClearColor[] = { 0.f, 0.f, 0.f, 0.f };
    Context->ClearRenderTargetView(LightRT->GetRTV().Get(), ClearColor);

    if (MsaaEnabled)
    {
        DLightPassMsaa->Begin();
    }
    else
    {
        DLightPass->Begin();
    }

    Context->VSSetConstantBuffers(0, 1, DLightVSCB.GetAddressOf());
    Context->PSSetConstantBuffers(0, 1, DLightPSCB.GetAddressOf());

    uint32_t stride = QuadVB->GetStride();
    uint32_t offset = 0;
    Context->IASetVertexBuffers(0, 1, QuadVB->GetVB().GetAddressOf(), &stride, &offset);

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = Context->Map(DLightVSCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        assert(false);
        return;
    }
    DLightVSConstants* vsConstants = (DLightVSConstants*)mapped.pData;
    vsConstants->ClipDistance = 100;
    Context->Unmap(DLightVSCB.Get(), 0);

    hr = Context->Map(DLightPSCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        assert(false);
        return;
    }
    DLightPSConstants* psConstants = (DLightPSConstants*)mapped.pData;
    psConstants->NumLights = 2;

    XMMATRIX worldToView = XMLoadFloat4x4(&view.WorldToView);

    psConstants->Lights[0].Color = XMFLOAT3(0.3f, 0.3f, 0.6f);
    XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(-1, 1, -1, 0));
    lightDir = XMVector3TransformNormal(lightDir, worldToView);
    XMStoreFloat3(&psConstants->Lights[0].Direction, lightDir);

    psConstants->Lights[1].Color = XMFLOAT3(0.7f, 0.7f, 0.5f);
    lightDir = XMVector3Normalize(XMVectorSet(1, 1, 1, 0));
    lightDir = XMVector3TransformNormal(lightDir, worldToView);
    XMStoreFloat3(&psConstants->Lights[1].Direction, lightDir);

    Context->Unmap(DLightPSCB.Get(), 0);

    Context->Draw(QuadVB->GetVertexCount(), QuadVB->GetBaseVertex());

    if (MsaaEnabled)
    {
        DLightPassMsaa->End();
    }
    else
    {
        DLightPass->End();
    }
}

void LPFRenderer::RenderFinal(const RenderView& view, const RenderTarget& renderTarget)
{
    static const float ClearColor[] = { 0.f, 0.f, 0.f, 1.f };
    Context->ClearRenderTargetView(renderTarget.Texture->GetRTV().Get(), ClearColor);

    std::shared_ptr<ShaderPass> pass;

    if (MsaaEnabled)
    {
        assert(FinalRT);
        pass = FinalPassMsaa;
    }
    else
    {
        FinalPass->SetRenderTarget(0, renderTarget.Texture->GetRTV());
        pass = FinalPass;
    }
    pass->Begin();

    Context->VSSetConstantBuffers(0, 1, GBufferVSCB.GetAddressOf());

    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        if (!visual->GetAlbedoTexture())
        {
            continue;
        }
        Context->PSSetShaderResources(1, 1, visual->GetAlbedoTexture()->GetSRV().GetAddressOf());

        D3D11_MAPPED_SUBRESOURCE mapped{};
        HRESULT hr = Context->Map(GBufferVSCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr))
        {
            assert(false);
            continue;
        }
        GBufferVSConstants* constants = (GBufferVSConstants*)mapped.pData;
        XMStoreFloat4x4(&constants->LocalToView, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), XMLoadFloat4x4(&view.WorldToView)));
        XMStoreFloat4x4(&constants->LocalToProjection, XMMatrixMultiply(XMLoadFloat4x4(&visual->GetLocalToWorld()), worldToProjection));
        Context->Unmap(GBufferVSCB.Get(), 0);

        pass->Draw(visual);
    }

    pass->End();
    if (MsaaEnabled)
    {
        Context->ResolveSubresource(renderTarget.Texture->GetTexture().Get(), 0, FinalRT->GetTexture().Get(), 0, renderTarget.Texture->GetDesc().Format);
    }
}
