#pragma once

class Object : NonCopyable
{
public:
    virtual ~Object();

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

    void SetPosition(FXMVECTOR position)
    {
        XMStoreFloat3(&Position, position);
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

    void SetOrientation(FXMVECTOR orientation)
    {
        XMStoreFloat4(&Orientation, orientation);
        LocalToWorldDirty = true;
    }

    const XMFLOAT4X4& GetLocalToWorld() const;

    const XMFLOAT3& GetWorldBoundsCenter() const;

    float GetWorldBoundsRadius() const
    {
        return BoundsRadius;
    }

protected:
    Object();

    virtual void RecomputeWorldBoundsCenter() const {}

protected:
    XMFLOAT3 Position;
    XMFLOAT4 Orientation;
    float BoundsRadius;

    mutable bool LocalToWorldDirty;
    mutable XMFLOAT4X4 LocalToWorld;

    // world bounds center
    mutable bool WorldBoundsCenterDirty;
    mutable XMFLOAT3 WorldBoundsCenter;
};
