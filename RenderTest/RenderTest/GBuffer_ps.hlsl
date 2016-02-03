// Fill gbuffer with normal & depth info

#include "Common.hlsli"

float4 main(GBufferVSOutput input) : SV_TARGET
{
    // TODO: store normal in x & y (since view normal, we can easily recompute z later... BUT requires sqrt so maybe sucks!)
    // TODO: quantize linear depth to 16 bits and pack into z and w channels
    return float4(1.f, 1.f, 1.f, 1.f);
}