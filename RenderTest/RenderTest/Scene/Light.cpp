#include "Precomp.h"
#include "Light.h"

Light::~Light()
{
}

Light::Light(LightType type)
    : Type(type)
    , Color(1.f, 1.f, 1.f)
{
}

DirectionalLight::DirectionalLight()
    : Light(LightType::Directional)
{
}

DirectionalLight::~DirectionalLight()
{
}

PointLight::PointLight()
    : Light(LightType::Point)
{
}

PointLight::~PointLight()
{
}
