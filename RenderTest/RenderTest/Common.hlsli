//
// Input Vertex Types
//

struct PositionNormalVertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};


//
// Light Pre-Pass Vertex Shader output
//

struct LightPrePassVSOutput
{
    float4 Position : SV_POSITION;
    float3 ViewNormal : NORMAL;
    float LinearDepth : TEXCOORD;
};
