// Combine surface data with prepass lighting info to get final output

#include "Common.hlsli"

#ifdef USE_MSAAx4
Texture2DMS<float4, 4> LightTexture;
#else
Texture2D LightTexture;
#endif

Texture2D AlbedoTexture;
SamplerState LinearSampler;

float4 main(FinalPassVSOutput input) : SV_TARGET
{
#ifdef USE_MSAAx4
    float4 light = AVERAGE_MSAA_SAMPLES(LightTexture, input.Position.xy);
#else
    uint width, height, numLevels;
    LightTexture.GetDimensions(0, width, height, numLevels);
    float4 light = LightTexture.Sample(LinearSampler, input.Position.xy / float2(width, height));
#endif

    float4 albedo = AlbedoTexture.Sample(LinearSampler, input.TexCoord);

    return float4(albedo.xyz * light.xyz, 0);
}