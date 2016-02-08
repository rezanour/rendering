
#include "Common.hlsli"

cbuffer DLightVSConstants
{
    float ClipDistance;
};

DLightVSOutput main(Position2DVertex input)
{
    DLightVSOutput output;
    output.Position = float4(input.Position, 0, 1);
    output.EyeRay = float3(input.Position.xy, ClipDistance);
    return output;
}
