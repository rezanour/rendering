#include "Precomp.h"
#include "Visual.h"
#include "CoreGraphics/VertexBuffer.h"
#include "CoreGraphics/IndexBuffer.h"
#include "CoreGraphics/Texture.h"

Visual::Visual()
    : BaseIndex(0)
    , IndexCount(0)
{
    // Not implemented yet
    BoundsRadius = 1.f;
}

Visual::~Visual()
{
}

HRESULT Visual::Initialize(
    const std::shared_ptr<VertexBuffer>& vertexBuffer,
    const std::shared_ptr<IndexBuffer>& indexBuffer, uint32_t baseIndex, uint32_t indexCount)
{
    VB = vertexBuffer;
    IB = indexBuffer;
    BaseIndex = baseIndex;
    IndexCount = indexCount;
    return S_OK;
}

const std::shared_ptr<Texture2D>& Visual::GetAlbedoTexture() const
{
    return AlbedoTexture;
}

void Visual::SetAlbedoTexture(const std::shared_ptr<Texture2D>& texture)
{
    AlbedoTexture = texture;
}

const std::shared_ptr<Texture2D>& Visual::GetNormalTexture() const
{
    return NormalTexture;
}

void Visual::SetNormalTexture(const std::shared_ptr<Texture2D>& texture)
{
    NormalTexture = texture;
}

const std::shared_ptr<Texture2D>& Visual::GetSpecularTexture() const
{
    return SpecularTexture;
}

void Visual::SetSpecularTexture(const std::shared_ptr<Texture2D>& texture)
{
    SpecularTexture = texture;
}

void Visual::RecomputeWorldBoundsCenter() const
{
    XMFLOAT3 offset = XMFLOAT3(0.f, 0.f, 0.f);
    WorldBoundsCenter.x = Position.x + offset.x;
    WorldBoundsCenter.y = Position.y + offset.y;
    WorldBoundsCenter.z = Position.z + offset.z;
}
