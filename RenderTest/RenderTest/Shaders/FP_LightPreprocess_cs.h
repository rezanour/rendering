#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Buffer Definitions: 
//
// cbuffer Constants
// {
//
//   float4x4 WorldToView;              // Offset:    0 Size:    64
//
// }
//
// Resource bind info for Lights
// {
//
//   struct
//   {
//       
//       float3 Position;               // Offset:    0
//       float Radius;                  // Offset:   12
//       float3 Color;                  // Offset:   16
//       uint IsVisible;                // Offset:   28
//
//   } $Element;                        // Offset:    0 Size:    32
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// Lights                                UAV  struct         r/w    0        1
// Constants                         cbuffer      NA          NA    0        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Output
cs_4_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[4], immediateIndexed
dcl_uav_structured u0, 32
dcl_input vThreadGroupID.x
dcl_temps 2
dcl_thread_group 1, 1, 1
ld_structured r0.xyzw, vThreadGroupID.x, l(0), u0.xyzw
mul r1.xyz, r0.yyyy, cb0[1].xyzx
mad r1.xyz, cb0[0].xyzx, r0.xxxx, r1.xyzx
mad r0.xyz, cb0[2].xyzx, r0.zzzz, r1.xyzx
add r0.xyz, r0.xyzx, cb0[3].xyzx
store_structured u0.xyz, vThreadGroupID.x, l(0), r0.xyzx
lt r0.x, r0.z, -r0.w
movc r0.x, r0.x, l(0), l(1)
store_structured u0.x, vThreadGroupID.x, l(28), r0.x
ret 
// Approximately 10 instruction slots used
#endif

const BYTE FP_LightPreprocess_cs[] =
{
     68,  88,  66,  67, 228, 109, 
    201, 164,  90, 155, 167, 161, 
     69, 210, 168, 197, 194,  83, 
    176,  43,   1,   0,   0,   0, 
     32,   4,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    248,   1,   0,   0,   8,   2, 
      0,   0,  24,   2,   0,   0, 
    164,   3,   0,   0,  82,  68, 
     69,  70, 188,   1,   0,   0, 
      2,   0,   0,   0, 112,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
     83,  67,   0,   1,   4,   0, 
    136,   1,   0,   0,  92,   0, 
      0,   0,   6,   0,   0,   0, 
      6,   0,   0,   0,   1,   0, 
      0,   0,  32,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     99,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  76, 105, 103, 104, 
    116, 115,   0,  67, 111, 110, 
    115, 116,  97, 110, 116, 115, 
      0, 171, 171, 171,  99,   0, 
      0,   0,   1,   0,   0,   0, 
    160,   0,   0,   0,  64,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  92,   0, 
      0,   0,   1,   0,   0,   0, 
    212,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0, 184,   0, 
      0,   0,   0,   0,   0,   0, 
     64,   0,   0,   0,   2,   0, 
      0,   0, 196,   0,   0,   0, 
      0,   0,   0,   0,  87, 111, 
    114, 108, 100,  84, 111,  86, 
    105, 101, 119,   0,   3,   0, 
      3,   0,   4,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 236,   0,   0,   0, 
      0,   0,   0,   0,  32,   0, 
      0,   0,   2,   0,   0,   0, 
    120,   1,   0,   0,   0,   0, 
      0,   0,  36,  69, 108, 101, 
    109, 101, 110, 116,   0,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0, 171, 171,   1,   0, 
      3,   0,   1,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  82,  97, 100, 105, 
    117, 115,   0, 171,   0,   0, 
      3,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  67, 111, 108, 111, 
    114,   0,  73, 115,  86, 105, 
    115, 105,  98, 108, 101,   0, 
      0,   0,  19,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 245,   0, 
      0,   0,   0,   1,   0,   0, 
      0,   0,   0,   0,  16,   1, 
      0,   0,  24,   1,   0,   0, 
     12,   0,   0,   0,  40,   1, 
      0,   0,   0,   1,   0,   0, 
     16,   0,   0,   0,  46,   1, 
      0,   0,  56,   1,   0,   0, 
     28,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,   8,   0, 
      0,   0,   4,   0,  72,   1, 
      0,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  54,  46,  51,  46,  57, 
     54,  48,  48,  46,  49,  54, 
     51,  56,  52,   0, 171, 171, 
     73,  83,  71,  78,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  79,  83, 
     71,  78,   8,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  83,  72,  69,  88, 
    132,   1,   0,   0,  64,   0, 
      5,   0,  97,   0,   0,   0, 
    106,   8,   0,   1,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0, 158,   0,   0,   4, 
      0, 224,  17,   0,   0,   0, 
      0,   0,  32,   0,   0,   0, 
     95,   0,   0,   2,  18,  16, 
      2,   0, 104,   0,   0,   2, 
      2,   0,   0,   0, 155,   0, 
      0,   4,   1,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 167,   0,   0,   8, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  10,  16,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  70, 238,  17,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   8, 114,   0,  16,   0, 
      1,   0,   0,   0,  86,   5, 
     16,   0,   0,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 114,   0,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0, 166,  10, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   8, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      3,   0,   0,   0, 168,   0, 
      0,   8, 114, 224,  17,   0, 
      0,   0,   0,   0,  10,  16, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     49,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16, 128, 
     65,   0,   0,   0,   0,   0, 
      0,   0,  55,   0,   0,   9, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   1,   0, 
      0,   0, 168,   0,   0,   8, 
     18, 224,  17,   0,   0,   0, 
      0,   0,  10,  16,   2,   0, 
      1,  64,   0,   0,  28,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    116,   0,   0,   0,  10,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};
