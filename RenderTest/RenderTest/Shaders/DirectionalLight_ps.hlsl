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

#ifdef USE_MSAAx4
Texture2DMS<float4, 4> ViewNormals;
Texture2DMS<float4, 4> LinearDepths;
#else
Texture2D ViewNormals;
Texture2D LinearDepths;
SamplerState LinearSampler;
#endif

float4 main(DLightVSOutput input) : SV_TARGET
{
#ifdef USE_MSAAx4
    float3 viewNormal = AVERAGE_MSAA_SAMPLES(ViewNormals, input.Position.xy).xyz * 2 - 1;
    float linearDepth = AVERAGE_MSAA_SAMPLES(LinearDepths, input.Position.xy).x;
#else
    uint width, height, numLevels;
    ViewNormals.GetDimensions(0, width, height, numLevels);
    float2 size = float2(width, height);
    float3 viewNormal = ViewNormals.Sample(LinearSampler, input.Position.xy / size).xyz * 2 - 1;
    float linearDepth = LinearDepths.Sample(LinearSampler, input.Position.xy / size).x;
#endif

    float3 viewPosition = normalize(input.EyeRay) * linearDepth;

    float3 light = float3(0, 0, 0);

    for (uint i = 0; i < NumLights; ++i)
    {
        light += saturate(Lights[i].Color * dot(Lights[i].Direction, viewNormal));
    }

    return float4(light, 0);
}