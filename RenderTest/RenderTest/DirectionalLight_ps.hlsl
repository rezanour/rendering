// Apply directional light(s) to light buffer

#include "Common.hlsli"

struct DLight
{
    float3 Direction;
    float Pad0;
    float3 Color;
    float Pad1;
};

#define MAX_DLIGHTS_PER_PASS 8
cbuffer DLightPSConstants
{
    DLight Lights[MAX_DLIGHTS_PER_PASS];
    uint NumLights;
};

Texture2D ViewNormals;
Texture2D LinearDepths;

float4 main(DLightVSOutput input) : SV_TARGET
{
    float3 viewNormal = ViewNormals[input.Position.xy].xyz * 2 - 1;
    float linearDepth = LinearDepths[input.Position.xy].x;

    float3 viewPosition = normalize(input.EyeRay) * linearDepth;

    float3 light = float3(0, 0, 0);

    for (uint i = 0; i < NumLights; ++i)
    {
        light += (Lights[i].Color * dot(Lights[i].Direction, viewNormal));
    }

    return float4(light, 0);
}