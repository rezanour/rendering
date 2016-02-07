#include "Precomp.h"
#include "ShaderPass.h"
#include "RenderVisual.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"
#include "IndexBuffer.h"

ShaderPass::ShaderPass()
{
}

ShaderPass::~ShaderPass()
{
    assert(_countof(VSSamplers) == _countof(PSSamplers));

    for (int i = 0; i < _countof(VSSamplers); ++i)
    {
        if (VSSamplers[i]) VSSamplers[i]->Release();
        VSSamplers[i] = nullptr;

        if (PSSamplers[i]) PSSamplers[i]->Release();
        PSSamplers[i] = nullptr;
    }

    assert(_countof(VSResources) == _countof(PSResources));

    for (int i = 0; i < _countof(VSSamplers); ++i)
    {
        if (VSResources[i]) VSResources[i]->Release();
        VSResources[i] = nullptr;

        if (PSResources[i]) PSResources[i]->Release();
        PSResources[i] = nullptr;
    }

    for (int i = 0; i < _countof(RenderTargets); ++i)
    {
        if (RenderTargets[i]) RenderTargets[i]->Release();
        RenderTargets[i] = nullptr;
    }
}

HRESULT ShaderPass::InitializeGraphics(
    const ComPtr<ID3D11Device>& device,
    VertexFormat format,
    const uint8_t* vertexShader, size_t vertexShaderNumBytes,
    const uint8_t* pixelShader, size_t pixelShaderNumBytes)
{
    Device = device;
    Device->GetImmediateContext(&Context);

    Type = ShaderPassType::Graphics;

    HRESULT hr = Device->CreateVertexShader(vertexShader, vertexShaderNumBytes, nullptr, &VertexShader);
    CHECKHR(hr);

    hr = Device->CreatePixelShader(pixelShader, pixelShaderNumBytes, nullptr, &PixelShader);
    CHECKHR(hr);

    hr = CreateInputLayout(format, vertexShader, vertexShaderNumBytes);
    CHECKHR(hr);

    CurrentInputBinding = format;

    return hr;
}

void ShaderPass::SetRenderTarget(int slot, const ComPtr<ID3D11RenderTargetView>& rtv)
{
    assert(slot >= 0 && slot < _countof(RenderTargets));

    if (RenderTargets[slot]) RenderTargets[slot]->Release();

    RenderTargets[slot] = rtv.Get();

    if (RenderTargets[slot]) RenderTargets[slot]->AddRef();
}

void ShaderPass::SetViewport(const D3D11_VIEWPORT* viewport)
{
    if (viewport)
    {
        Viewport = *viewport;
    }
    else
    {
        Viewport = {};
        if (RenderTargets[0])
        {
            ComPtr<ID3D11Resource> resource;
            RenderTargets[0]->GetResource(&resource);

            ComPtr<ID3D11Texture2D> texture;
            resource.As(&texture);

            D3D11_TEXTURE2D_DESC td{};
            texture->GetDesc(&td);

            Viewport.Width = static_cast<float>(td.Width);
            Viewport.Height = static_cast<float>(td.Height);
            Viewport.MaxDepth = 1.f;
        }
    }
}

void ShaderPass::SetDepthBuffer(const ComPtr<ID3D11DepthStencilView>& dsv)
{
    DepthBuffer = dsv;
}

void ShaderPass::SetDepthState(const ComPtr<ID3D11DepthStencilState>& depthState)
{
    DepthState = depthState;
}

void ShaderPass::SetVSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv)
{
    assert(slot >= 0 && slot < _countof(VSResources));

    if (VSResources[slot]) VSResources[slot]->Release();

    VSResources[slot] = srv.Get();

    if (VSResources[slot]) VSResources[slot]->AddRef();
}

void ShaderPass::SetPSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv)
{
    assert(slot >= 0 && slot < _countof(PSResources));

    if (PSResources[slot]) PSResources[slot]->Release();

    PSResources[slot] = srv.Get();

    if (PSResources[slot]) PSResources[slot]->AddRef();
}

void ShaderPass::SetVSSampler(int slot, const ComPtr<ID3D11SamplerState>& sampler)
{
    assert(slot >= 0 && slot < _countof(VSSamplers));

    if (VSSamplers[slot]) VSSamplers[slot]->Release();

    VSSamplers[slot] = sampler.Get();

    if (VSSamplers[slot]) VSSamplers[slot]->AddRef();
}

void ShaderPass::SetPSSampler(int slot, const ComPtr<ID3D11SamplerState>& sampler)
{
    assert(slot >= 0 && slot < _countof(VSSamplers));

    if (PSSamplers[slot]) PSSamplers[slot]->Release();

    PSSamplers[slot] = sampler.Get();

    if (PSSamplers[slot]) PSSamplers[slot]->AddRef();
}

void ShaderPass::Begin()
{
    switch (Type)
    {
    case ShaderPassType::Graphics:
        BeginGraphics();
        return;

    case ShaderPassType::Compute:
        BeginCompute();
        return;

    default:
        assert(false);
        break;
    }
}

void ShaderPass::Draw(const std::shared_ptr<RenderVisual>& visual)
{
    auto& vb = visual->GetVB();
    uint32_t stride = vb->GetStride();
    uint32_t offset = 0;

    if (vb->GetFormat() != CurrentInputBinding)
    {
        auto& il = InputLayouts[(uint32_t)vb->GetFormat()];
        if (!il)
        {
            assert(false);
            return;
        }

        CurrentInputBinding = vb->GetFormat();
        Context->IASetInputLayout(il.Get());
    }

    Context->IASetVertexBuffers(0, 1, vb->GetVB().GetAddressOf(), &stride, &offset);
    Context->IASetIndexBuffer(visual->GetIB()->GetIB().Get(), DXGI_FORMAT_R32_UINT, 0);

    Context->DrawIndexed(visual->GetIndexCount(), visual->GetBaseIndex(), vb->GetBaseVertex());
}

void ShaderPass::End()
{
    static ID3D11RenderTargetView* const nullRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    static ID3D11ShaderResourceView* const nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};

    Context->OMSetRenderTargets(_countof(nullRTVs), nullRTVs, nullptr);
    Context->VSSetShaderResources(0, _countof(nullSRVs), nullSRVs);
    Context->PSSetShaderResources(0, _countof(nullSRVs), nullSRVs);
}

void ShaderPass::BeginGraphics()
{
    Context->OMSetRenderTargets(_countof(RenderTargets), RenderTargets, DepthBuffer.Get());
    Context->OMSetDepthStencilState(DepthState.Get(), 0);
    Context->RSSetViewports(1, &Viewport);

    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(InputLayouts[(uint32_t)CurrentInputBinding].Get());

    Context->VSSetShader(VertexShader.Get(), nullptr, 0);
    Context->VSSetShaderResources(0, _countof(VSResources), VSResources);
    Context->VSSetSamplers(0, _countof(VSSamplers), VSSamplers);

    Context->PSSetShader(PixelShader.Get(), nullptr, 0);
    Context->PSSetShaderResources(0, _countof(PSResources), PSResources);
    Context->PSSetSamplers(0, _countof(PSSamplers), PSSamplers);
}

void ShaderPass::BeginCompute()
{
}

HRESULT ShaderPass::CreateInputLayout(VertexFormat format, const uint8_t* vertexShader, size_t vertexShaderBytes)
{
    D3D11_INPUT_ELEMENT_DESC elems[16]{};

    switch (format)
    {
    case VertexFormat::Position2D:
        elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
        elems[0].SemanticName = "POSITION";
        return Device->CreateInputLayout(elems, 1, vertexShader, vertexShaderBytes, &InputLayouts[(uint32_t)format]);

    case VertexFormat::PositionNormal:
        elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        elems[0].SemanticName = "POSITION";
        elems[1].AlignedByteOffset = sizeof(XMFLOAT3);
        elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        elems[1].SemanticName = "NORMAL";
        return Device->CreateInputLayout(elems, 2, vertexShader, vertexShaderBytes, &InputLayouts[(uint32_t)format]);

    case VertexFormat::Basic3D:
        elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        elems[0].SemanticName = "POSITION";
        elems[1].AlignedByteOffset = sizeof(XMFLOAT3);
        elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        elems[1].SemanticName = "NORMAL";
        elems[2].AlignedByteOffset = sizeof(XMFLOAT3) * 2;
        elems[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        elems[2].SemanticName = "TANGENT";
        elems[3].AlignedByteOffset = sizeof(XMFLOAT3) * 3;
        elems[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        elems[3].SemanticName = "BITANGENT";
        elems[4].AlignedByteOffset = sizeof(XMFLOAT3) * 4;
        elems[4].Format = DXGI_FORMAT_R32G32_FLOAT;
        elems[4].SemanticName = "TEXCOORD";
        return Device->CreateInputLayout(elems, 5, vertexShader, vertexShaderBytes, &InputLayouts[(uint32_t)format]);

    default:
        assert(false);
        return E_NOTIMPL;
    }
}
