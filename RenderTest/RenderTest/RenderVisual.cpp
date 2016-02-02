#include "Precomp.h"
#include "RenderVisual.h"
#include "VertexBuffer.h"

RenderVisual::RenderVisual()
    : Position(0.f, 0.f, 0.f)
    , Orientation(0.f, 0.f, 0.f, 1.f)
    , LocalToWorldDirty(true)
    , WorldBoundsCenterDirty(true)
{
}

RenderVisual::~RenderVisual()
{
}

HRESULT RenderVisual::Initialize(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
    VB = vertexBuffer;
    return S_OK;
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
