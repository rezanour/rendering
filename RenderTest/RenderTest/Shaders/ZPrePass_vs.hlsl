// Zbuffer-only prepass (not RT bound) to quickly generate depth info

#include "Common.hlsli"

cbuffer ZPrePassVSConstants
{
    float4x4 LocalToProjection; // local -> world -> view -> projection
};

float4 main(Basic3DVertex input) : SV_POSITION
{
    return mul(LocalToProjection, float4(input.Position, 1));
}
