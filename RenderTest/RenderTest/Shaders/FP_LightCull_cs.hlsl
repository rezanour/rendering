
cbuffer Constants
{
    uint TileSize;
    uint NumTilesX;
    uint NumLights;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
    float Pad;
};

StructuredBuffer<PointLight> Lights;

struct LightLinkedListNode
{
    uint LightIndex;
    uint NextLight;
};

RWStructuredBuffer<LightLinkedListNode> Nodes;
RWByteAddressBuffer Heads;

void AddLight(uint2 tileCoord, uint lightIndex)
{
    uint iNode = Nodes.IncrementCounter();
    uint headAddress = 4 * ((NumTilesX * tileCoord.y) + tileCoord.x);
    uint oldHead = 0;
    Heads.InterlockedExchange(headAddress, iNode, oldHead);

    LightLinkedListNode node;
    node.LightIndex = lightIndex;
    node.NextLight = oldHead;
    Nodes[iNode] = node;
}

[numthreads(4, 4, 1)]
void main(
    uint3 GroupThreadID : SV_GroupThreadID,
    uint3 GroupID : SV_GroupID,
    uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // TODO: implement culling :)
    for (uint i = 0; i < NumLights; ++i)
    {
        if (GroupThreadID.x == 0 && GroupThreadID.y == 0)
        {
            AddLight(GroupID.xy, i);
        }
    }
}