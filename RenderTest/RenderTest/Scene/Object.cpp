#include "Precomp.h"
#include "Object.h"

Object::Object()
    : Position(0.f, 0.f, 0.f)
    , Orientation(0.f, 0.f, 0.f, 1.f)
    , BoundsRadius(0.f)
    , LocalToWorldDirty(true)
    , WorldBoundsCenterDirty(true)
{
}

Object::~Object()
{
}

const XMFLOAT4X4& Object::GetLocalToWorld() const
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

const XMFLOAT3& Object::GetWorldBoundsCenter() const
{
    if (WorldBoundsCenterDirty)
    {
        RecomputeWorldBoundsCenter();
        WorldBoundsCenterDirty = false;
    }

    return WorldBoundsCenter;
}
