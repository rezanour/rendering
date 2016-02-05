//
// Input Vertex Types
//

struct Position2DVertex
{
    float2 Position : POSITION;
};

struct PositionNormalVertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};


//
// Light Pre-Pass: GBuffer pass VS output
//

struct GBufferVSOutput
{
    float4 Position : SV_POSITION;
    float3 ViewNormal : NORMAL;
    float LinearDepth : TEXCOORD;
};

struct DLightVSOutput
{
    float4 Position : SV_POSITION;
    float3 EyeRay : TEXCOORD;
};

struct FinalPassVSOutput
{
    float4 Position : SV_POSITION;
    float3 ViewPosition : POSITION;
};


//
// Utility functions
//

float2 PackFloatInto2Channels(float value)
{
    half v = (half)value;
    float2 result;
    result.x = frac(v * 256.f);
    result.x -= result.x / 256.f;
    result.y = frac(v);
    return result;
}

float Unpack2ChannelsIntoFloat(float2 value)
{
    return (value.x / 256.f) + value.y;
}
