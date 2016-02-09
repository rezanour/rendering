#include "Common.hlsli"

struct DLight
{
    float3 Direction;
    float Pad0;
    float3 Color;
    float Pad1;
};

#define MAX_DLIGHTS 8
cbuffer DLightPSConstants
{
    DLight DLights[MAX_DLIGHTS];
    uint NumDLights;
};

Texture2D AlbedoTexture;
Texture2D NormalTexture;
SamplerState Sampler;

float4 main(FPFinalPassVSOutput input) : SV_TARGET
{
    float4 albedo = AlbedoTexture.Sample(Sampler, input.TexCoord);
    float3 normal = NormalTexture.Sample(Sampler, input.TexCoord).xyz;
    if (!any(normal))
    {
        normal = float3(0.5f, 0.5f, 1.f);
    }

    normal = normal * 2 - 1;
    normal.y *= -1;

    normal = mul(normal, float3x3(normalize(input.ViewTangent), -normalize(input.ViewBiTangent), normalize(input.ViewNormal)));
    normal = normalize(normal);

    float3 light = float3(0, 0, 0);

    for (uint i = 0; i < NumDLights; ++i)
    {
        light += saturate(DLights[i].Color * dot(DLights[i].Direction, normal));
    }

    return float4(albedo.xyz * light, albedo.w);
}
