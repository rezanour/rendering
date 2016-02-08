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

struct Basic3DVertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
    float2 TexCoord : TEXCOORD;
};


//
// Light Pre-Pass: GBuffer pass VS output
//

struct GBufferVSOutput
{
    float4 Position : SV_POSITION;
    float3 ViewNormal : NORMAL;
    float3 ViewTangent : TANGENT;
    float3 ViewBiTangent : BITANGENT;
    float2 TexCoord : TEXCOORD0;
    float LinearDepth : TEXCOORD1;
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
    float2 TexCoord : TEXCOORD;
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

#define AVERAGE_MSAA_SAMPLES(tex, pos) ((tex.Load(pos, 0) + tex.Load(pos, 1) + tex.Load(pos, 2) + tex.Load(pos, 3)) * 0.25f)
