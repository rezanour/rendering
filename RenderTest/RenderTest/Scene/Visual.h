#pragma once

#include "Object.h"

class VertexBuffer;
class IndexBuffer;
class Texture2D;

class Visual : public Object
{
public:
    Visual();
    virtual ~Visual();

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

    const std::shared_ptr<Texture2D>& GetAlbedoTexture() const;
    void SetAlbedoTexture(const std::shared_ptr<Texture2D>& texture);
    const std::shared_ptr<Texture2D>& GetNormalTexture() const;
    void SetNormalTexture(const std::shared_ptr<Texture2D>& texture);
    const std::shared_ptr<Texture2D>& GetSpecularTexture() const;
    void SetSpecularTexture(const std::shared_ptr<Texture2D>& texture);

private:
    virtual void RecomputeWorldBoundsCenter() const override;

private:
    std::shared_ptr<VertexBuffer> VB;
    std::shared_ptr<IndexBuffer> IB;
    uint32_t BaseIndex;
    uint32_t IndexCount;

    std::shared_ptr<Texture2D> AlbedoTexture;
    std::shared_ptr<Texture2D> NormalTexture;
    std::shared_ptr<Texture2D> SpecularTexture;
};
