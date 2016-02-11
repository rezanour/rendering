#include "Common.hlsli"

struct DLight
{
    float3 Direction;
    float Pad0;
    float3 Color;
    float Pad1;
};

#define MAX_DLIGHTS 8
cbuffer FinalPSConstants : register (b0)
{
    DLight DLights[MAX_DLIGHTS];
    uint NumDLights;
    uint TileSize;
    uint NumTilesX;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
    float Pad;
};

StructuredBuffer<PointLight> Lights : register (t0);

struct LightLinkedListNode
{
    uint LightIndex;
    uint NextLight;
};

StructuredBuffer<LightLinkedListNode> Nodes : register (t1);
ByteAddressBuffer Heads : register (t2);

Texture2D AlbedoTexture : register (t3);
Texture2D NormalTexture : register (t4);
SamplerState Sampler;

float4 main(FPFinalPassVSOutput input) : SV_TARGET
{
    float4 albedo = AlbedoTexture.Sample(Sampler, input.TexCoord);
    float3 normal = NormalTexture.Sample(Sampler, input.TexCoord).xyz;
    if (!any(normal))
    {
        normal = float3(0.5f, 0.5f, 1.f);
    }

    normal = normal * 2 - 1;
    normal.y *= -1;


    float3x3 tangToView = float3x3(normalize(input.ViewTangent), -normalize(input.ViewBiTangent), normalize(input.ViewNormal));
    normal = normalize(mul(normal, tangToView));

    float3 totalLight = float3(0, 0, 0);

    for (uint i = 0; i < NumDLights; ++i)
    {
        totalLight += saturate(DLights[i].Color * saturate(dot(DLights[i].Direction, normal)));
    }

    uint2 pos = input.Position.xy / TileSize;
    uint iNode = Heads.Load(4 * (pos.y * NumTilesX + pos.x));
    while (iNode != 0xFFFFFFFF)
    {
        LightLinkedListNode node = Nodes[iNode];
        PointLight light = Lights[node.LightIndex];

        float3 toLight = light.Position - input.ViewPosition;
        float3 L = normalize(toLight);
        float dist = length(toLight);
        // simple linear att for now
        float att = 1.f - (dist / light.Radius);
        att = max(att, 0);
        totalLight += light.Color * att * saturate(dot(normal, L));

        iNode = node.NextLight;
    }

    return float4(albedo.xyz * totalLight, albedo.w);
}
