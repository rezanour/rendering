// Fill gbuffer with normal & depth info

#include "Common.hlsli"

cbuffer GBufferVSConstants
{
    float4x4 LocalToView;   // local -> world -> View
    float4x4 LocalToProjection; // local -> world -> view -> projection
};

GBufferVSOutput main(Basic3DVertex input)
{
    GBufferVSOutput output;
    output.Position = mul(LocalToProjection, float4(input.Position, 1));

    // casting to 3x3 only works if LocalToView has no nonuniform scaling (or other skew, etc...)
    output.ViewNormal = mul((float3x3) LocalToView, input.Normal);
    output.ViewTangent = mul((float3x3) LocalToView, input.Tangent);
    output.ViewBiTangent = mul((float3x3) LocalToView, input.BiTangent);

    output.TexCoord = input.TexCoord;

    float4 viewPosition = mul(LocalToView, float4(input.Position, 1));
    output.LinearDepth = viewPosition.z;

    return output;
}
