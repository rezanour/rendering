#include "Precomp.h"
#include "LPFRenderer.h"
#include "Texture.h"
#include "RenderingCommon.h"
#include "RenderScene.h"
#include "RenderVisual.h"
#include "VertexBuffer.h"

// shaders
#include "GBuffer_vs.h"
#include "GBuffer_ps.h"
#include "FinalPass_vs.h"
#include "FinalPass_ps.h"

LPFRenderer::LPFRenderer(ID3D11Device* device)
    : BaseRenderer(device)
{
    device->GetImmediateContext(&Context);
}

LPFRenderer::~LPFRenderer()
{
}

HRESULT LPFRenderer::Initialize()
{
    D3D11_INPUT_ELEMENT_DESC gbufferInputElems[2]{};
    gbufferInputElems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    gbufferInputElems[0].SemanticName = "POSITION";
    gbufferInputElems[1].AlignedByteOffset = sizeof(XMFLOAT3);
    gbufferInputElems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    gbufferInputElems[1].SemanticName = "NORMAL";

    HRESULT hr = Device->CreateInputLayout(gbufferInputElems, _countof(gbufferInputElems), GBuffer_vs, sizeof(GBuffer_vs), &GBufferIL);
    CHECKHR(hr);

    hr = Device->CreateVertexShader(GBuffer_vs, sizeof(GBuffer_vs), nullptr, &GBufferVS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(GBuffer_ps, sizeof(GBuffer_ps), nullptr, &GBufferPS);
    CHECKHR(hr);

    hr = Device->CreateVertexShader(FinalPass_vs, sizeof(FinalPass_vs), nullptr, &FinalVS);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(FinalPass_ps, sizeof(FinalPass_ps), nullptr, &FinalPS);
    CHECKHR(hr);

    D3D11_BUFFER_DESC bd{};
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.ByteWidth = sizeof(GBufferVSConstants);
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.StructureByteStride = bd.ByteWidth;
    bd.Usage = D3D11_USAGE_DYNAMIC;

    hr = Device->CreateBuffer(&bd, nullptr, &GBufferVSCB);
    CHECKHR(hr);

    D3D11_TEXTURE2D_DESC desc{};
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Width = 1280;
    desc.Height = 720;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;

    Viewport.Width = static_cast<float>(desc.Width);
    Viewport.Height = static_cast<float>(desc.Height);
    Viewport.MaxDepth = 1.f;

    GBufferViewNormalsRT = std::make_shared<Texture2D>();
    hr = GBufferViewNormalsRT->Initialize(Device, desc);
    CHECKHR(hr);

    desc.Format = DXGI_FORMAT_R32_FLOAT;
    GBufferLinearDepthRT = std::make_shared<Texture2D>();
    hr = GBufferLinearDepthRT->Initialize(Device, desc);
    CHECKHR(hr);

    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    GBufferDepthBuffer = std::make_shared<Texture2D>();
    hr = GBufferDepthBuffer->Initialize(Device, desc);
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

    return S_OK;
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

void LPFRenderer::RenderGBuffer(const RenderView& view)
{
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

        Context->Draw(vb->GetVertexCount(), vb->GetBaseVertex());
    }
}

void LPFRenderer::RenderLights(const RenderView& view)
{
    UNREFERENCED_PARAMETER(view);
}

void LPFRenderer::RenderFinal(const RenderView& view, const RenderTarget& renderTarget)
{
    static const float ClearColor[] = { 0.f, 0.f, 0.f, 1.f };
    Context->ClearRenderTargetView(renderTarget.Texture->GetRTV().Get(), ClearColor);

    ID3D11RenderTargetView* rtvs[]{ renderTarget.Texture->GetRTV().Get(), nullptr };
    Context->OMSetRenderTargets(_countof(rtvs), rtvs, GBufferDepthBuffer->GetDSV().Get());
    Context->OMSetDepthStencilState(DepthReadState.Get(), 0);
    Context->RSSetViewports(1, &Viewport);

    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(GBufferIL.Get());
    Context->VSSetConstantBuffers(0, 1, GBufferVSCB.GetAddressOf());
    Context->VSSetShader(FinalVS.Get(), nullptr, 0);
    Context->PSSetShader(FinalPS.Get(), nullptr, 0);

    Context->PSSetShaderResources(0, 1, GBufferViewNormalsRT->GetSRV().GetAddressOf());

    XMMATRIX worldToProjection = XMMatrixMultiply(XMLoadFloat4x4(&view.WorldToView), XMLoadFloat4x4(&view.ViewToProjection));

    for (auto& visual : Visuals)
    {
        auto& vb = visual->GetVB();
        uint32_t stride = vb->GetStride();
        uint32_t offset = 0;
        Context->IASetVertexBuffers(0, 1, vb->GetVB().GetAddressOf(), &stride, &offset);

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

        Context->Draw(vb->GetVertexCount(), vb->GetBaseVertex());
    }
}