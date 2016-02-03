
#include "Common.hlsli"

cbuffer LightPrePassVSConstants
{
    float4x4 LocalToWorld;
    float4x4 WorldToView;
    float4x4 WorldToViewToProjection;
};

float4 main(PositionNormalVertex input) : SV_POSITION
{
    return float4(1.f, 1.f, 1.f, 1.f);
}