#include "Precomp.h"
#include "LPFRenderer.h"
#include "Texture.h"
#include "RenderingCommon.h"
#include "RenderScene.h"
#include "RenderVisual.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexFormats.h"

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
    // Input layouts
    D3D11_INPUT_ELEMENT_DESC gbufferInputElems[5]{};
    gbufferInputElems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    gbufferInputElems[0].SemanticName = "POSITION";
    gbufferInputElems[1].AlignedByteOffset = sizeof(XMFLOAT3);
    gbufferInputElems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    gbufferInputElems[1].SemanticName = "NORMAL";
    gbufferInputElems[2].AlignedByteOffset = sizeof(XMFLOAT3) * 2;
    gbufferInputElems[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    gbufferInputElems[2].SemanticName = "TANGENT";
    gbufferInputElems[3].AlignedByteOffset = sizeof(XMFLOAT3) * 3;
    gbufferInputElems[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    gbufferInputElems[3].SemanticName = "BITANGENT";
    gbufferInputElems[4].AlignedByteOffset = sizeof(XMFLOAT3) * 4;
    gbufferInputElems[4].Format = DXGI_FORMAT_R32G32_FLOAT;
    gbufferInputElems[4].SemanticName = "TEXCOORD";

    HRESULT hr = Device->CreateInputLayout(gbufferInputElems, _countof(gbufferInputElems), GBuffer_vs, sizeof(GBuffer_vs), &GBufferIL);
    CHECKHR(hr);

    D3D11_INPUT_ELEMENT_DESC dlightInputElems[1]{};
    dlightInputElems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    dlightInputElems[0].SemanticName = "POSITION";

    hr = Device->CreateInputLayout(dlightInputElems, _countof(dlightInputElems), DirectionalLight_vs, sizeof(DirectionalLight_vs), &DLightIL);
    CHECKHR(hr);

    // Shaders
    hr = Device->CreateVertexShader(GBuffer_vs, sizeof(GBuffer_vs), nullptr, &GBufferVS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(GBuffer_ps, sizeof(GBuffer_ps), nullptr, &GBufferPS);
    CHECKHR(hr);

    hr = Device->CreateVertexShader(DirectionalLight_vs, sizeof(DirectionalLight_vs), nullptr, &DLightVS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(DirectionalLight_ps, sizeof(DirectionalLight_ps), nullptr, &DLightPS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(DirectionalLight_MSAAx4_ps, sizeof(DirectionalLight_MSAAx4_ps), nullptr, &DLightMsaaPS);
    CHECKHR(hr);

    hr = Device->CreateVertexShader(FinalPass_vs, sizeof(FinalPass_vs), nullptr, &FinalVS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(FinalPass_ps, sizeof(FinalPass_ps), nullptr, &FinalPS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(FinalPass_MSAAx4_ps, sizeof(FinalPass_MSAAx4_ps), nullptr, &FinalMsaaPS);
    CHECKHR(hr);

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

    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    hr = Device->CreateDepthStencilState(&dsDesc, &DepthReadState);
    CHECKHR(hr);

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

    D3D11_SAMPLER_DESC sd{};
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device->CreateSamplerState(&sd, &LinearSampler);
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

    return hr;
}

void LPFRenderer::UnbindAllRTVsAndSRVs()
{
    ID3D11RenderTargetView* nullRTVs[]{ nullptr, nullptr, nullptr };
    ID3D11ShaderResourceView* nullSRVs[]{ nullptr, nullptr, nullptr };
    Context->OMSetRenderTargets(_countof(nullRTVs), nullRTVs, nullptr);
    Context->PSSetShaderResources(0, _countof(nullSRVs), nullSRVs);
}

void LPFRenderer::RenderGBuffer(const RenderView& view)
{
    UnbindAllRTVsAndSRVs();

    static const float ViewNormalsClear[] = { 0.f, 0.f, 0.f, 1.f };
    static const float LinearDepthClear[] = { 1.f, 1.f, 1.f, 1.f };
    Context->ClearRenderTargetView(GBufferViewNormalsRT->GetRTV().Get(), ViewNormalsClear);
    Context->ClearRenderTargetView(GBufferLinearDepthRT->GetRTV().Get(), LinearDepthClear);
    Context->ClearDepthStencilView(GBufferDepthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH, 1.f, 0);

    ID3D11RenderTargetView* rtvs[]{ GBufferViewNormalsRT->GetRTV().Get(), GBufferLinearDepthRT->GetRTV().Get() };
    Context->OMSetRenderTargets(_countof(rtvs), rtvs, GBufferDepthBuffer->GetDSV().Get());
    Context->OMSetDepthStencilState(DepthWriteState.Get(), 0);
    Context->RSSetViewports(1, &Viewport);

    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(GBufferIL.Get());
    Context->VSSetConstantBuffers(0, 1, GBufferVSCB.GetAddressOf());
    Context->VSSetShader(GBufferVS.Get(), nullptr, 0);
    Context->PSSetShader(GBufferPS.Get(), nullptr, 0);

    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        auto& vb = visual->GetVB();
        uint32_t stride = vb->GetStride();
        uint32_t offset = 0;
        Context->IASetVertexBuffers(0, 1, vb->GetVB().GetAddressOf(), &stride, &offset);
        Context->IASetIndexBuffer(visual->GetIB()->GetIB().Get(), DXGI_FORMAT_R32_UINT, 0);
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

        Context->DrawIndexed(visual->GetIndexCount(), visual->GetBaseIndex(), vb->GetBaseVertex());
    }
}

void LPFRenderer::RenderLights(const RenderView& view)
{
    UNREFERENCED_PARAMETER(view);

    UnbindAllRTVsAndSRVs();

    static const float ClearColor[] = { 0.f, 0.f, 0.f, 0.f };
    Context->ClearRenderTargetView(LightRT->GetRTV().Get(), ClearColor);

    ID3D11RenderTargetView* rtvs[]{ LightRT->GetRTV().Get(), nullptr };
    Context->OMSetRenderTargets(_countof(rtvs), rtvs, nullptr);
    Context->RSSetViewports(1, &Viewport);

    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(DLightIL.Get());

    Context->VSSetShader(DLightVS.Get(), nullptr, 0);
    Context->VSSetConstantBuffers(0, 1, DLightVSCB.GetAddressOf());

    Context->PSSetShader(MsaaEnabled ? DLightMsaaPS.Get() : DLightPS.Get(), nullptr, 0);
    Context->PSSetConstantBuffers(0, 1, DLightPSCB.GetAddressOf());

    ID3D11ShaderResourceView* srvs[]{ GBufferViewNormalsRT->GetSRV().Get(), GBufferLinearDepthRT->GetSRV().Get() };
    Context->PSSetShaderResources(0, _countof(srvs), srvs);
    Context->PSSetSamplers(0, 1, LinearSampler.GetAddressOf());

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
}

void LPFRenderer::RenderFinal(const RenderView& view, const RenderTarget& renderTarget)
{
    UnbindAllRTVsAndSRVs();

    static const float ClearColor[] = { 0.f, 0.f, 0.f, 1.f };
    Context->ClearRenderTargetView(renderTarget.Texture->GetRTV().Get(), ClearColor);

    ComPtr<ID3D11RenderTargetView> rtv = MsaaEnabled ? FinalRT->GetRTV() : renderTarget.Texture->GetRTV();
    assert(MsaaEnabled == !!FinalRT);

    ID3D11RenderTargetView* rtvs[]{ rtv.Get(), nullptr };
    Context->OMSetRenderTargets(_countof(rtvs), rtvs, GBufferDepthBuffer->GetDSV().Get());
    Context->OMSetDepthStencilState(DepthReadState.Get(), 0);
    Context->RSSetViewports(1, &Viewport);

    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(GBufferIL.Get());
    Context->VSSetConstantBuffers(0, 1, GBufferVSCB.GetAddressOf());
    Context->VSSetShader(FinalVS.Get(), nullptr, 0);
    Context->PSSetShader(MsaaEnabled ? FinalMsaaPS.Get() : FinalPS.Get(), nullptr, 0);

    Context->PSSetShaderResources(0, 1, LightRT->GetSRV().GetAddressOf());
    Context->PSSetSamplers(0, 1, LinearSampler.GetAddressOf());

    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        auto& vb = visual->GetVB();
        uint32_t stride = vb->GetStride();
        uint32_t offset = 0;
        Context->IASetVertexBuffers(0, 1, vb->GetVB().GetAddressOf(), &stride, &offset);
        Context->IASetIndexBuffer(visual->GetIB()->GetIB().Get(), DXGI_FORMAT_R32_UINT, 0);

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

        Context->DrawIndexed(visual->GetIndexCount(), visual->GetBaseIndex(), vb->GetBaseVertex());
    }

    if (MsaaEnabled)
    {
        Context->ResolveSubresource(renderTarget.Texture->GetTexture().Get(), 0, FinalRT->GetTexture().Get(), 0, renderTarget.Texture->GetDesc().Format);
    }
}
