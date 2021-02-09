#pragma once

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

struct Simple
{
    float4x4 modelViewProj;
    Texture2D tex;
    SamplerState gSampler;
    StructuredBuffer<Material> materials;
};
