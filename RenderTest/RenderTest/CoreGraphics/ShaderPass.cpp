#include "Precomp.h"
#include "ShaderPass.h"
#include "Scene/Visual.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"
#include "IndexBuffer.h"

ShaderPass::ShaderPass()
{
}

ShaderPass::~ShaderPass()
{
    for (int i = 0; i < _countof(Buffers); ++i)
    {
        if (Buffers[i]) Buffers[i]->Release();
        Buffers[i] = nullptr;
    }

    for (int i = 0; i < _countof(RenderTargets); ++i)
    {
        if (RenderTargets[i]) RenderTargets[i]->Release();
        RenderTargets[i] = nullptr;
    }

    assert(_countof(VSCSConstants) == _countof(PSConstants));

    for (int i = 0; i < _countof(VSCSConstants); ++i)
    {
        if (VSCSConstants[i]) VSCSConstants[i]->Release();
        VSCSConstants[i] = nullptr;

        if (PSConstants[i]) PSConstants[i]->Release();
        PSConstants[i] = nullptr;
    }

    assert(_countof(VSCSResources) == _countof(PSResources));

    for (int i = 0; i < _countof(VSCSResources); ++i)
    {
        if (VSCSResources[i]) VSCSResources[i]->Release();
        VSCSResources[i] = nullptr;

        if (PSResources[i]) PSResources[i]->Release();
        PSResources[i] = nullptr;
    }

    assert(_countof(VSSamplers) == _countof(PSSamplers));

    for (int i = 0; i < _countof(VSSamplers); ++i)
    {
        if (VSSamplers[i]) VSSamplers[i]->Release();
        VSSamplers[i] = nullptr;

        if (PSSamplers[i]) PSSamplers[i]->Release();
        PSSamplers[i] = nullptr;
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

    if (pixelShader)
    {
        hr = Device->CreatePixelShader(pixelShader, pixelShaderNumBytes, nullptr, &PixelShader);
        CHECKHR(hr);
    }

    hr = CreateInputLayout(format, vertexShader, vertexShaderNumBytes);
    CHECKHR(hr);

    CurrentInputBinding = format;

    return hr;
}

HRESULT ShaderPass::InitializeCompute(const ComPtr<ID3D11Device>& device, const uint8_t* computeShader, size_t computeShaderNumBytes)
{
    Device = device;
    Device->GetImmediateContext(&Context);

    Type = ShaderPassType::Compute;

    HRESULT hr = Device->CreateComputeShader(computeShader, computeShaderNumBytes, nullptr, &ComputeShader);
    CHECKHR(hr);

    return hr;
}

void ShaderPass::SetBuffer(int slot, const ComPtr<ID3D11UnorderedAccessView>& uav, bool resetCounterEachPass)
{
    // can't change buffer while rendering
    assert(!Rendering);

    assert(slot >= 0 && slot < _countof(Buffers));

    if (Buffers[slot]) Buffers[slot]->Release();

    Buffers[slot] = uav.Get();
    BufferCounters[slot] = resetCounterEachPass ? 0 : -1;

    if (Buffers[slot]) Buffers[slot]->AddRef();
}

void ShaderPass::SetRenderTarget(int slot, const ComPtr<ID3D11RenderTargetView>& rtv)
{
    // can't change RT while rendering
    assert(!Rendering);

    assert(slot >= 0 && slot < _countof(RenderTargets));

    if (RenderTargets[slot]) RenderTargets[slot]->Release();

    RenderTargets[slot] = rtv.Get();

    if (RenderTargets[slot]) RenderTargets[slot]->AddRef();
}

void ShaderPass::SetViewport(const D3D11_VIEWPORT* viewport)
{
    // can't change viewport while rendering
    assert(!Rendering);

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
    // can't change depth buffer while rendering
    assert(!Rendering);

    DepthBuffer = dsv;
}

void ShaderPass::SetDepthState(const ComPtr<ID3D11DepthStencilState>& depthState)
{
    // can't change depth state while rendering
    assert(!Rendering);

    DepthState = depthState;
}

void ShaderPass::SetCSConstantBuffer(int slot, const ComPtr<ID3D11Buffer>& cb)
{
    assert(slot >= 0 && slot < _countof(VSCSConstants));

    if (VSCSConstants[slot]) VSCSConstants[slot]->Release();

    VSCSConstants[slot] = cb.Get();

    if (VSCSConstants[slot]) VSCSConstants[slot]->AddRef();

    if (Rendering)
    {
        Context->CSSetConstantBuffers(slot, 1, cb.GetAddressOf());
    }
}

void ShaderPass::SetCSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv)
{
    assert(slot >= 0 && slot < _countof(VSCSResources));

    if (VSCSResources[slot]) VSCSResources[slot]->Release();

    VSCSResources[slot] = srv.Get();

    if (VSCSResources[slot]) VSCSResources[slot]->AddRef();

    if (Rendering)
    {
        Context->CSSetShaderResources(slot, 1, srv.GetAddressOf());
    }
}

void ShaderPass::SetVSConstantBuffer(int slot, const ComPtr<ID3D11Buffer>& cb)
{
    assert(slot >= 0 && slot < _countof(VSCSConstants));

    if (VSCSConstants[slot]) VSCSConstants[slot]->Release();

    VSCSConstants[slot] = cb.Get();

    if (VSCSConstants[slot]) VSCSConstants[slot]->AddRef();

    if (Rendering)
    {
        Context->VSSetConstantBuffers(slot, 1, cb.GetAddressOf());
    }
}

void ShaderPass::SetPSConstantBuffer(int slot, const ComPtr<ID3D11Buffer>& cb)
{
    assert(slot >= 0 && slot < _countof(PSConstants));

    if (PSConstants[slot]) PSConstants[slot]->Release();

    PSConstants[slot] = cb.Get();

    if (PSConstants[slot]) PSConstants[slot]->AddRef();

    if (Rendering)
    {
        Context->PSSetConstantBuffers(slot, 1, cb.GetAddressOf());
    }
}

void ShaderPass::SetVSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv)
{
    assert(slot >= 0 && slot < _countof(VSCSResources));

    if (VSCSResources[slot]) VSCSResources[slot]->Release();

    VSCSResources[slot] = srv.Get();

    if (VSCSResources[slot]) VSCSResources[slot]->AddRef();

    if (Rendering)
    {
        Context->VSSetShaderResources(slot, 1, srv.GetAddressOf());
    }
}

void ShaderPass::SetPSResource(int slot, const ComPtr<ID3D11ShaderResourceView>& srv)
{
    assert(slot >= 0 && slot < _countof(PSResources));

    if (PSResources[slot]) PSResources[slot]->Release();

    PSResources[slot] = srv.Get();

    if (PSResources[slot]) PSResources[slot]->AddRef();

    if (Rendering)
    {
        Context->PSSetShaderResources(slot, 1, srv.GetAddressOf());
    }
}

void ShaderPass::SetVSSampler(int slot, const ComPtr<ID3D11SamplerState>& sampler)
{
    assert(slot >= 0 && slot < _countof(VSSamplers));

    if (VSSamplers[slot]) VSSamplers[slot]->Release();

    VSSamplers[slot] = sampler.Get();

    if (VSSamplers[slot]) VSSamplers[slot]->AddRef();

    if (Rendering)
    {
        Context->VSSetSamplers(slot, 1, sampler.GetAddressOf());
    }
}

void ShaderPass::SetPSSampler(int slot, const ComPtr<ID3D11SamplerState>& sampler)
{
    assert(slot >= 0 && slot < _countof(VSSamplers));

    if (PSSamplers[slot]) PSSamplers[slot]->Release();

    PSSamplers[slot] = sampler.Get();

    if (PSSamplers[slot]) PSSamplers[slot]->AddRef();

    if (Rendering)
    {
        Context->PSSetSamplers(slot, 1, sampler.GetAddressOf());
    }
}

void ShaderPass::Begin()
{
    switch (Type)
    {
    case ShaderPassType::Graphics:
        BeginGraphics();
        break;

    case ShaderPassType::Compute:
        BeginCompute();
        break;

    default:
        assert(false);
        return;
    }

    Rendering = true;
}

void ShaderPass::Draw(const std::shared_ptr<Visual>& visual)
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

void ShaderPass::Dispatch(uint32_t numThreadsX, uint32_t numThreadsY, uint32_t numThreadsZ)
{
    Context->Dispatch(numThreadsX, numThreadsY, numThreadsZ);
}

void ShaderPass::End()
{
    static ID3D11RenderTargetView* const nullRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    static ID3D11ShaderResourceView* const nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    static ID3D11UnorderedAccessView* const nullUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT]{};
    static uint32_t emptyCounters[D3D11_PS_CS_UAV_REGISTER_COUNT]{};

    Context->CSSetUnorderedAccessViews(0, _countof(nullUAVs), nullUAVs, emptyCounters);
    Context->OMSetRenderTargets(_countof(nullRTVs), nullRTVs, nullptr);
    Context->VSSetShaderResources(0, _countof(nullSRVs), nullSRVs);
    Context->PSSetShaderResources(0, _countof(nullSRVs), nullSRVs);

    Rendering = false;
}

void ShaderPass::BeginGraphics()
{
    if (Viewport.Width == 0 || Viewport.Height == 0)
    {
        // Reset based on RenderTarget
        SetViewport(nullptr);
    }

    Context->OMSetRenderTargets(_countof(RenderTargets), RenderTargets, DepthBuffer.Get());
    Context->OMSetDepthStencilState(DepthState.Get(), 0);
    Context->RSSetViewports(1, &Viewport);

    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(InputLayouts[(uint32_t)CurrentInputBinding].Get());

    Context->VSSetShader(VertexShader.Get(), nullptr, 0);
    Context->VSSetConstantBuffers(0, _countof(VSCSConstants), VSCSConstants);
    Context->VSSetShaderResources(0, _countof(VSCSResources), VSCSResources);
    Context->VSSetSamplers(0, _countof(VSSamplers), VSSamplers);

    Context->PSSetShader(PixelShader.Get(), nullptr, 0);
    if (PixelShader)
    {
        Context->PSSetConstantBuffers(0, _countof(PSConstants), PSConstants);
        Context->PSSetShaderResources(0, _countof(PSResources), PSResources);
        Context->PSSetSamplers(0, _countof(PSSamplers), PSSamplers);
    }

    Context->CSSetShader(nullptr, nullptr, 0);
}

void ShaderPass::BeginCompute()
{
    Context->VSSetShader(nullptr, nullptr, 0);
    Context->PSSetShader(nullptr, nullptr, 0);

    Context->CSSetShader(ComputeShader.Get(), nullptr, 0);
    Context->CSSetUnorderedAccessViews(0, _countof(Buffers), Buffers, BufferCounters);
    Context->CSSetConstantBuffers(0, _countof(VSCSConstants), VSCSConstants);
    Context->CSSetShaderResources(0, _countof(VSCSResources), VSCSResources);
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
