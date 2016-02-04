// Render geometry for final combining

#include "Common.hlsli"

cbuffer GBufferVSConstants
{
    float4x4 LocalToView;   // local -> world -> View
    float4x4 LocalToProjection; // local -> world -> view -> projection
};

FinalPassVSOutput main(PositionNormalVertex input)
{
    FinalPassVSOutput output;
    output.Position = mul(LocalToProjection, float4(input.Position, 1));
    output.ViewPosition = mul(LocalToView, float4(input.Position, 1)).xyz;
    return output;
}
