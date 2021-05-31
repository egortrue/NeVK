// Вход вершинного шейдера
struct VS_INPUT {
    float3 position : POSITION;
    float2 uv;
};

// Вход фрагментного шейдера
struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv;
};

// Константы, задаваемые в начале каждого кадра
struct constants {
    int objectID;
};
[[vk::push_constant]] ConstantBuffer<constants> constValues;


// Ресурсу, привязанные к конвейеру
cbuffer ubo // VkBuffer
{
    float4x4 modelViewProj;
}
Texture2D gTexture[]; // VkImageView
SamplerState gSampler; // VkSampler


[shader("vertex")]
PS_INPUT vertexMain(VS_INPUT vertex)
{
    PS_INPUT data;

    data.position = mul(modelViewProj, float4(vertex.position, 1.0f));
    data.uv = vertex.uv;

    return data;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT data) : SV_TARGET
{
    return float4(gTexture[NonUniformResourceIndex(constValues.objectID)].Sample(gSampler, data.uv));
}
