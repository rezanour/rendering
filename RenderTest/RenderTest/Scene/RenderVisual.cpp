#include "Precomp.h"
#include "RenderVisual.h"
#include "CoreGraphics/VertexBuffer.h"
#include "CoreGraphics/IndexBuffer.h"
#include "CoreGraphics/Texture.h"

RenderVisual::RenderVisual()
    : BaseIndex(0)
    , IndexCount(0)
    , Position(0.f, 0.f, 0.f)
    , Orientation(0.f, 0.f, 0.f, 1.f)
    , LocalToWorldDirty(true)
    , WorldBoundsCenterDirty(true)
{
}

RenderVisual::~RenderVisual()
{
}

HRESULT RenderVisual::Initialize(
    const std::shared_ptr<VertexBuffer>& vertexBuffer,
    const std::shared_ptr<IndexBuffer>& indexBuffer, uint32_t baseIndex, uint32_t indexCount)
{
    VB = vertexBuffer;
    IB = indexBuffer;
    BaseIndex = baseIndex;
    IndexCount = indexCount;
    return S_OK;
}

const std::shared_ptr<Texture2D>& RenderVisual::GetAlbedoTexture() const
{
    return AlbedoTexture;
}

void RenderVisual::SetAlbedoTexture(const std::shared_ptr<Texture2D>& texture)
{
    AlbedoTexture = texture;
}

const std::shared_ptr<Texture2D>& RenderVisual::GetNormalTexture() const
{
    return NormalTexture;
}

void RenderVisual::SetNormalTexture(const std::shared_ptr<Texture2D>& texture)
{
    NormalTexture = texture;
}

const std::shared_ptr<Texture2D>& RenderVisual::GetSpecularTexture() const
{
    return SpecularTexture;
}

void RenderVisual::SetSpecularTexture(const std::shared_ptr<Texture2D>& texture)
{
    SpecularTexture = texture;
}

const XMFLOAT4X4& RenderVisual::GetLocalToWorld() const
{
    if (LocalToWorldDirty)
    {
        XMStoreFloat4x4(&LocalToWorld,
            XMMatrixAffineTransformation(
                XMVectorSet(1.f, 1.f, 1.f, 1.f),    // Scaling
                XMVectorZero(),                     // Rotation Origin
                XMLoadFloat4(&Orientation),         // Rotation
                XMLoadFloat3(&Position)));          // Translation
        LocalToWorldDirty = false;
    }

    return LocalToWorld;
}

const XMFLOAT3& RenderVisual::GetWorldBoundsCenter() const
{
    if (WorldBoundsCenterDirty)
    {
        const XMFLOAT3& offset = VB->GetBoundsOffset();
        WorldBoundsCenter.x = Position.x + offset.x;
        WorldBoundsCenter.y = Position.y + offset.y;
        WorldBoundsCenter.z = Position.z + offset.z;
        WorldBoundsCenterDirty = false;
    }

    return WorldBoundsCenter;
}

float RenderVisual::GetWorldBoundsRadius() const
{
    // we don't support scale, so this never changes
    return VB->GetBoundsRadius();
}
