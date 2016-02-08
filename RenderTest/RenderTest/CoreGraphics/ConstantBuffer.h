#pragma once

class ConstantBuffer : NonCopyable
{
public:
    ConstantBuffer();
    virtual ~ConstantBuffer();

    HRESULT Initialize(const ComPtr<ID3D11Device>& device, const void* data, uint32_t dataBytes);

    uint32_t GetSizeInBytes() const
    {
        return SizeInBytes;
    }

    const ComPtr<ID3D11Buffer>& GetCB() const
    {
        return CB;
    }

    HRESULT Update(const void* data, uint32_t dataBytes);

private:
    ComPtr<ID3D11Buffer> CB;
    uint32_t SizeInBytes;
};
