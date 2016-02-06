#pragma once

class VertexBuffer;
class IndexBuffer;

class RenderVisual : NonCopyable
{
public:
    RenderVisual();
    virtual ~RenderVisual();

    HRESULT Initialize(
        const std::shared_ptr<VertexBuffer>& vertexBuffer,
        const std::shared_ptr<IndexBuffer>& indexBuffer, uint32_t baseIndex, uint32_t indexCount);

    const std::shared_ptr<VertexBuffer>& GetVB() const
    {
        return VB;
    }

    const std::shared_ptr<IndexBuffer>& GetIB() const
    {
        return IB;
    }

    uint32_t GetBaseIndex() const
    {
        return BaseIndex;
    }

    uint32_t GetIndexCount() const
    {
        return IndexCount;
    }

    const XMFLOAT3& GetPosition() const
    {
        return Position;
    }

    void SetPosition(const XMFLOAT3& position)
    {
        Position = position;
        LocalToWorldDirty = true;
        WorldBoundsCenterDirty = true;
    }

    const XMFLOAT4& GetOrientation() const
    {
        return Orientation;
    }

    void SetOrientation(const XMFLOAT4& orientation)
    {
        Orientation = orientation;
        LocalToWorldDirty = true;
    }

    const XMFLOAT4X4& GetLocalToWorld() const;

    const XMFLOAT3& GetWorldBoundsCenter() const;

    float GetWorldBoundsRadius() const;

private:
    std::shared_ptr<VertexBuffer> VB;
    std::shared_ptr<IndexBuffer> IB;
    uint32_t BaseIndex;
    uint32_t IndexCount;

    XMFLOAT3 Position;
    XMFLOAT4 Orientation;

    mutable bool LocalToWorldDirty;
    mutable XMFLOAT4X4 LocalToWorld;

    // world bounds center
    mutable bool WorldBoundsCenterDirty;
    mutable XMFLOAT3 WorldBoundsCenter;
};
