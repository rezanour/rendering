// Fill gbuffer with normal & depth info

#include "Common.hlsli"

cbuffer GBufferVSConstants
{
    float4x4 LocalToWorld;
    float4x4 WorldToView;
    float4x4 WorldToViewToProjection;
};

GBufferVSOutput main(PositionNormalVertex input)
{
    GBufferVSOutput output;
    output.Position = mul(WorldToViewToProjection, float4(input.Position, 1));
    // casting to 3x3 only works if LocalToWorld and WorldToView have no nonuniform scaling (or other skew, etc...)
    output.ViewNormal = mul((float3x3) WorldToView, mul((float3x3) LocalToWorld, input.Normal));
    output.LinearDepth = output.Position.z / output.Position.w;
    return output;
}
