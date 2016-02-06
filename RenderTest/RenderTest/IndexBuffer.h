#pragma once

// TODO: Combine with VertexBuffer into single Geometry or Mesh class
class IndexBuffer : NonCopyable
{
public:
    IndexBuffer();
    virtual ~IndexBuffer();

    HRESULT Initialize(const ComPtr<ID3D11Device>& device, const void* data, uint32_t dataBytes);

    uint32_t GetBaseIndex() const
    {
        return BaseIndex;
    }

    uint32_t GetIndexCount() const
    {
        return IndexCount;
    }

    const ComPtr<ID3D11Buffer>& GetIB() const
    {
        return IB;
    }

private:
    ComPtr<ID3D11Buffer> IB;
    uint32_t BaseIndex;
    uint32_t IndexCount;
};
