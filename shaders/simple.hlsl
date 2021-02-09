#include "simpleparams.h"

float3 hsv2rgb(float3 input) {
  float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  float3 p = abs(frac(input.xxx + K.xyz) * 6.0 - K.www);
  return input.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), input.y);
}

struct AssembledVertex {
  float3 position : POSITION;
  float3 normal;
  float2 uv;
  uint materialId;
};

ParameterBlock<Simple> gSimpleParams;
// ParameterBlock<Materials> gMaterials;

struct PS_INPUT {
  float4 pos : SV_POSITION;
  float3 normal;
  float2 uv;
  nointerpolation uint materialId;
};

[shader("vertex")] 
PS_INPUT vertexMain(AssembledVertex av) {
  PS_INPUT ret;
  ret.pos = mul(gSimpleParams.modelViewProj, float4(av.position, 1.0f));
  ret.normal = av.normal;
  ret.uv = av.uv;
  ret.materialId = av.materialId;
  return ret;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET {
  float dotNL = dot(inp.normal, normalize(float3(1, 1, 1)));
  float h = frac(inp.materialId / 100.0);
  float3 color = saturate(dotNL * 0.5 + 0.5) * hsv2rgb(float3(h, 0.5, 0.95)); // gSimpleParams.materials[inp.materialId].diffuse.rgb;
  return float4(color, 1.0);
}
