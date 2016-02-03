// Fill gbuffer with normal & depth info

#include "Common.hlsli"

void main(
    GBufferVSOutput input,
    out float4 Normal : SV_TARGET0,
    out float LinearDepth : SV_TARGET1)
{
    // compress from [-1, 1] range to [0, 1] range
    Normal.xyz = input.ViewNormal * 0.5f + 0.5f;
    Normal.w = 0.f;
    LinearDepth = input.LinearDepth;
}