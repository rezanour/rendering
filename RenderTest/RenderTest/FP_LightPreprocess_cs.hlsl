
cbuffer Constants
{
    float4x4 WorldToView;
    //float3 LeftNorm;
    //float3 RightNorm;
    //float3 UpNorm;
    //float3 DownNorm;
    //float MaxDist;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
    uint IsVisible;
};

RWStructuredBuffer<PointLight> Lights : register (u0);

[numthreads(1, 1, 1)]
void main(uint3 GroupID : SV_GroupID)
{
    // Each thread processes a single light
    PointLight light = Lights[GroupID.x];

    // Transform into view space
    float3 viewPosition = mul(WorldToView, float4(light.Position, 1)).xyz;
    light.Position = viewPosition;

    // whole frustum visibility culling
    if (light.Position.z < -light.Radius)
    {
        light.IsVisible = 0;
    }
    else
    {
        light.IsVisible = 1;
    }

    Lights[GroupID.x] = light;
}