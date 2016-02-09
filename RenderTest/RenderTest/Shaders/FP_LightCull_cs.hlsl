
cbuffer Constants
{
    uint RTWidth;
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

void AddLight(uint2 pixelCoord, uint lightIndex)
{
    uint iNode = Nodes.IncrementCounter();
    uint headAddress = 4 * ((RTWidth * pixelCoord.y) + pixelCoord.x);
    uint oldHead = 0;
    Heads.InterlockedExchange(headAddress, iNode, oldHead);

    LightLinkedListNode node;
    node.LightIndex = lightIndex;
    node.NextLight = oldHead;
    Nodes[iNode] = node;
}

static const uint NUM_PIXELS_PER_GROUP_X = 4;
static const uint NUM_PIXELS_PER_GROUP_Y = 4;

[numthreads(NUM_PIXELS_PER_GROUP_X, NUM_PIXELS_PER_GROUP_Y, 1)]
void main(
    uint3 GroupThreadID : SV_GroupThreadID,
    uint3 GroupID : SV_GroupID,
    uint3 DispatchThreadID : SV_DispatchThreadID)
{
    // Determine which pixel we are
    uint2 pixelCoord = GroupID.xy * uint2(NUM_PIXELS_PER_GROUP_X, NUM_PIXELS_PER_GROUP_Y) + GroupThreadID.xy;

    // TODO: implement culling :)
    for (uint i = 0; i < NumLights; ++i)
    {
        AddLight(pixelCoord, i);
    }
}