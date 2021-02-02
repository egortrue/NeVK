#include "simpleparams.h"

struct AssembledVertex
{
    float3 position : POSITION;
    float3 normal;
    float2 uv;
};

ParameterBlock<Simple> gSimpleParams;

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal;
    float2 uv;
};

[shader("vertex")]
PS_INPUT vertexMain(AssembledVertex av)
{
    PS_INPUT ret;
    ret.pos = mul(gSimpleParams.modelViewProj, float4(av.position, 1.0f));
    ret.normal = av.normal;
    ret.uv = av.uv;
    return ret;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
    float dotNL = dot(inp.normal, normalize(float3(1, 1, 1)));
    float3 color = saturate(dotNL * 0.5 + 0.5);
    return float4(color, 1.0);
}
