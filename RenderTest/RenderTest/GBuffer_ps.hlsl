// Fill gbuffer with normal & depth info

#include "Common.hlsli"

Texture2D NormalMap;
SamplerState LinearSampler;

void main(
    GBufferVSOutput input,
    out float4 Normal : SV_TARGET0,
    out float LinearDepth : SV_TARGET1)
{
    // Normal
    float3 N = NormalMap.Sample(LinearSampler, input.TexCoord).xyz;
    if (!any(N))
    {
        N = float3(0.5f, 0.5f, 1.f);
    }

    N = N * 2 - 1;
    N.y *= -1;

    Normal.xyz = mul(N, float3x3(normalize(input.ViewTangent), -normalize(input.ViewBiTangent), normalize(input.ViewNormal)));

    // compress from [-1, 1] range to [0, 1] range
    Normal.xyz = Normal.xyz * 0.5 + 0.5;
    Normal.w = 0.f;

    LinearDepth = input.LinearDepth;
}