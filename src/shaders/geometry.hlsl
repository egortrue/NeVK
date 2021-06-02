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

// Константы, задаваемые для каждого объекта
struct constants_t {
    float4x4 objectModel;
    int objectTexture;
};
[[vk::push_constant]] ConstantBuffer<constants_t> instance;


// Ресурсу, привязанные к конвейеру
cbuffer ubo // VkBuffer
{
    float4x4 cameraView;
    float4x4 cameraProjection;
}
Texture2D textures[]; // VkImageView
SamplerState textureSampler; // VkSampler


[shader("vertex")]
PS_INPUT vertexMain(VS_INPUT vertex)
{
    PS_INPUT data;

    float4x4 modelViewProj = mul(cameraProjection, mul(cameraView, instance.objectModel));
    data.position = mul(modelViewProj, float4(vertex.position, 1.0f));
    data.uv = vertex.uv;

    return data;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT data) : SV_TARGET
{
    return float4(textures[NonUniformResourceIndex(instance.objectTexture)].Sample(textureSampler, data.uv));
}
