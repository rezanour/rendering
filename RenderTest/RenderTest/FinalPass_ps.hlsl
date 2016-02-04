// Combine surface data with prepass lighting info to get final output

#include "Common.hlsli"

Texture2D LightTexture;

float4 main(FinalPassVSOutput input) : SV_TARGET
{
    float4 light = LightTexture[input.Position.xy];
    return float4(light.xyz, 0);
}