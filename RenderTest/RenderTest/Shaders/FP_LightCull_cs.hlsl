
#include "../ComputeTileConstants.h"

cbuffer Constants
{
    uint NumTilesX;
    uint NumLights;
    float ProjectionA;
    float ProjectionB;
    float2 ViewportSize;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
    uint IsVisible;
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

float3 RayDirFromPixelCoord(uint2 pixelCoord, float2 viewportSize, float depth)
{
    // convert to [0, 1] range
    float2 coord = float2((float)pixelCoord.x, viewportSize.y - (float)pixelCoord.y);
    coord /= viewportSize;
    // convert to [-1, 1] range
    coord = coord * 2 - 1;
    return normalize(float3(coord.xy, depth));
}

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

#ifndef AVOID_UINT_COMPARE
// To speed up the min/max derivation, we use uint bit
// representation of the values so we can perform interlocked
// ops on them across the thread group. However, I'm not 100%
// convinced that doing min/max on bit representation of IEEE float
// is actually valid in all cases, so we have a fallback path in
// case this doesn't pan out :)
groupshared uint MinDepth = 0xFFFFFFFF;
groupshared uint MaxDepth = 0.f;
#else
groupshared float Depths[TOTAL_PIXELS_PER_GROUP];
#endif

[numthreads(NUM_PIXELS_PER_GROUP_X, NUM_PIXELS_PER_GROUP_Y, 1)]
void main(
    uint3 GroupThreadID : SV_GroupThreadID,
    uint3 GroupID : SV_GroupID,
    uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // Determine which pixel we are, and also top left & bottom right pixels of our group (needed for culling)
    uint2 groupSize = uint2(NUM_PIXELS_PER_GROUP_X, NUM_PIXELS_PER_GROUP_Y);
    uint2 pixelCoord = GroupID.xy * groupSize + GroupThreadID.xy;
    uint2 coordTL = GroupID.xy * groupSize;
    uint2 coordBR = GroupID.xy * groupSize + groupSize;

    // Determine which linear index within the group we are
    uint iStart = GroupThreadID.y * NUM_PIXELS_PER_GROUP_X + GroupThreadID.x;

    // Read post-projection depth from the z buffer
    float depth = DepthBuffer.Load(int3(pixelCoord.xy, 0)).x;

    // Find the local view space rays from origin through top left & bottom right corners
    float3 rayTL = RayDirFromPixelCoord(coordTL, ViewportSize, depth);
    float3 rayBR = RayDirFromPixelCoord(coordBR, ViewportSize, depth);

    // outward facing normal vectors from froxel sides
    float3 cullLeft = float3(-rayTL.z, 0, rayTL.x);
    float3 cullRight = float3(rayBR.z, 0, -rayBR.x);
    float3 cullTop = float3(0, rayTL.z, -rayTL.y);
    float3 cullBottom = float3(0, -rayBR.z, rayBR.y);

    // convert back to linear view space depth
    float linearDepth = ProjectionB / (depth - ProjectionA);

#ifndef AVOID_UINT_COMPARE
    InterlockedMin(MinDepth, asuint(linearDepth));
    InterlockedMax(MaxDepth, asuint(linearDepth));
#else
    Depths[iStart] = linearDepth;
#endif
    GroupMemoryBarrierWithGroupSync();

#ifndef AVOID_UINT_COMPARE
    float minDepth = asfloat(MinDepth);
    float maxDepth = asfloat(MaxDepth);
#else
    float minDepth = Depths[0];
    float maxDepth = Depths[0];
    for (uint iDepth = 1; iDepth < TOTAL_PIXELS_PER_GROUP; ++iDepth)
    {
        if (Depths[iDepth] < minDepth)
            minDepth = Depths[iDepth];
    
        if (Depths[iDepth] > maxDepth)
            maxDepth = Depths[iDepth];
    }
#endif

    for (uint i = iStart; i < NumLights; i += TOTAL_PIXELS_PER_GROUP)
    {
        PointLight light = Lights[i];

        if (light.IsVisible == 0)
        {
            // light completely culled during preprocessing
            continue;
        }

        // The Froxel z extents (in view space) are MinDepth & MaxDepth.
        if (light.Position.z + light.Radius < minDepth ||
            light.Position.z - light.Radius > maxDepth)
        {
            // culled
            continue;
        }

        // cull against left/right side of froxel
        if (dot(light.Position, cullLeft) > light.Radius ||
            dot(light.Position, cullRight) > light.Radius)
        {
            // culled
            continue;
        }

        // cull against top/bottom side of froxel
        if (dot(light.Position, cullTop) > light.Radius ||
            dot(light.Position, cullBottom) > light.Radius)
        {
            // culled
            // TODO: Disabled for now due to bug. Need to debug and see
            // why top/down culling is wrong.
            //continue;
        }

        AddLight(GroupID.xy, i);
    }
}