#pragma once

class Buffer : NonCopyable
{
public:
    Buffer();
    virtual ~Buffer();

    HRESULT Initialize(const ComPtr<ID3D11Device>& device, uint32_t elementSize, uint32_t byteWidth, bool isStructured, bool isCounter);

    const D3D11_BUFFER_DESC& GetDesc() const
    {
        return Desc;
    }

    const ComPtr<ID3D11Buffer>& GetResource() const
    {
        return Resource;
    }

    const ComPtr<ID3D11ShaderResourceView>& GetSRV() const
    {
        return SRV;
    }

    const ComPtr<ID3D11UnorderedAccessView>& GetUAV() const
    {
        return UAV;
    }

private:
    HRESULT CreateViews(const ComPtr<ID3D11Device>& device, bool isCounter);

private:
    D3D11_BUFFER_DESC Desc{};
    ComPtr<ID3D11Buffer> Resource;
    ComPtr<ID3D11ShaderResourceView> SRV;
    ComPtr<ID3D11UnorderedAccessView> UAV;
};
