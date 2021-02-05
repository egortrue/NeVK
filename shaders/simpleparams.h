#pragma once

struct Material
{
    float4 diffuse;
}

struct Simple
{
    float4x4 modelViewProj;
    Texture2D tex;
    SamplerState gSampler;
    StructuredBuffer<Material> materials;
};
