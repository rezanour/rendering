// Final rendering pass for Forward+.

#include "Common.hlsli"

cbuffer FinalPassVSConstants
{
    float4x4 LocalToView;       // local -> world -> view
    float4x4 LocalToProjection; // local -> world -> view -> projection
};

FPFinalPassVSOutput main(Basic3DVertex input)
{
    FPFinalPassVSOutput output;
    output.Position = mul(LocalToProjection, float4(input.Position, 1));

    output.ViewPosition = mul(LocalToView, float4(input.Position, 1)).xyz;
    // casting to 3x3 only works if LocalToView has no nonuniform scaling (or other skew, etc...)
    output.ViewNormal = mul((float3x3) LocalToView, input.Normal);
    output.ViewTangent = mul((float3x3) LocalToView, input.Tangent);
    output.ViewBiTangent = mul((float3x3) LocalToView, input.BiTangent);

    output.TexCoord = input.TexCoord;

    return output;
}
