#pragma once

#include "Object.h"

enum class LightType
{
    Unknown = 0,
    Directional,
    Point
};

class Light : public Object
{
public:
    virtual ~Light();

    LightType GetType() const
    {
        return Type;
    }

    const XMFLOAT3& GetColor() const
    {
        return Color;
    }

    void SetColor(const XMFLOAT3& color)
    {
        Color = color;
    }

protected:
    Light(LightType type);

private:
    LightType Type;
    XMFLOAT3 Color;
};

// Object's forward vector is light direction
class DirectionalLight : public Light
{
public:
    DirectionalLight();
    virtual ~DirectionalLight();
};

// Object's bounds radius is light radius
class PointLight : public Light
{
public:
    PointLight();
    virtual ~PointLight();

    void SetRadius(float radius)
    {
        BoundsRadius = radius;
    }
};
