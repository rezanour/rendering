// Apply directional light(s) to light buffer

#include "Common.hlsli"

struct DLight
{
    float3 Direction;
    float3 Color;
};

#define MAX_DLIGHTS_PER_PASS 8
DLight DLights[MAX_DLIGHTS_PER_PASS];

Texture2D ViewNormals;
Texture2D LinearDepths;

float4 main(DLightVSOutput input) : SV_TARGET
{
    float3 viewNormal = ViewNormals[input.Position.xy].xyz * 2 - 1;
    float linearDepth = LinearDepths[input.Position.xy].x;

    float3 viewPosition = normalize(input.EyeRay) * linearDepth;

    const float3 lightDir = normalize(float3(1, -1, 1));
    return dot(-lightDir, viewNormal);
}