
#include "../ComputeTileConstants.h"

cbuffer Constants
{
    uint NumTilesX;
    uint NumLights;
    float ProjectionA;
    float ProjectionB;
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

RWStructuredBuffer<LightLinkedListNode> Nodes : register (u0);
RWByteAddressBuffer Heads : register (u1);

Texture2D DepthBuffer : register(t1);

void AddLight(uint2 tileCoord, uint lightIndex)
{
    uint iNode = Nodes.IncrementCounter();
    uint headAddress = 4 * ((NumTilesX * tileCoord.y) + tileCoord.x);   // 4 = sizeof(uint32)
    uint oldHead = 0;
    Heads.InterlockedExchange(headAddress, iNode, oldHead);

    LightLinkedListNode node;
    node.LightIndex = lightIndex;
    node.NextLight = oldHead;
    Nodes[iNode] = node;
}


//groupshared uint MinDepth = 0xFFFFFFFF;
//groupshared uint MaxDepth = 0.f;
groupshared float Depths[TOTAL_PIXELS_PER_GROUP];

[numthreads(NUM_PIXELS_PER_GROUP_X, NUM_PIXELS_PER_GROUP_Y, 1)]
void main(
    uint3 GroupThreadID : SV_GroupThreadID,
    uint3 GroupID : SV_GroupID,
    uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // Determine which pixel we are
    uint2 pixelCoord = GroupID.xy * uint2(NUM_PIXELS_PER_GROUP_X, NUM_PIXELS_PER_GROUP_Y) + GroupThreadID.xy;
    uint iStart = GroupThreadID.y * NUM_PIXELS_PER_GROUP_X + GroupThreadID.x;

    float depth = DepthBuffer.Load(int3(pixelCoord.xy, 0)).x;
    float linearDepth = ProjectionB / (depth - ProjectionA);

    Depths[iStart] = linearDepth;

    GroupMemoryBarrierWithGroupSync();

    float minDepth = Depths[0];
    float maxDepth = Depths[0];
    for (uint iDepth = 1; iDepth < TOTAL_PIXELS_PER_GROUP; ++iDepth)
    {
        if (Depths[iDepth] < minDepth)
            minDepth = Depths[iDepth];
    
        if (Depths[iDepth] > maxDepth)
            maxDepth = Depths[iDepth];
    }

    for (uint i = iStart; i < NumLights; i += TOTAL_PIXELS_PER_GROUP)
    {
        PointLight light = Lights[i];

        // The Froxel z extents (in view space) are MinDepth & MaxDepth.
        // For now, we do a cheezy "slab" cull of just z
        if (light.Position.z + light.Radius < minDepth ||
            light.Position.z - light.Radius > maxDepth)
        {
            // culled
            continue;
        }

        AddLight(GroupID.xy, i);
    }
}