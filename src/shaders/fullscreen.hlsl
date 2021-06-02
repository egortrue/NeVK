// Вход вершинного шейдера
struct VS_INPUT {
    uint id: SV_VertexID;
    float3 position : POSITION;
};

// Вход фрагментного шейдера
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv;
};

// Константы, задаваемые для каждого кадра
struct constants_t {
    int imageIndex;
};
[[vk::push_constant]] ConstantBuffer<constants_t> instance;

// Ресурсу, привязанные к конвейеру
Texture2D images[]; // VkImageView
SamplerState imageSampler; // VkSampler

[shader("vertex")]
PS_INPUT vertexMain(VS_INPUT vertex)
{
    PS_INPUT data;

    data.uv = float2((vertex.id << 1) & 2, vertex.id & 2);
    data.position = float4(data.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

    return data;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT fragment) : SV_TARGET
{
    return float4(images[NonUniformResourceIndex(instance.imageIndex)].Sample(imageSampler, fragment.uv));
    //return float4(gTexture.Sample(gSampler, input.uv)) + float4(gPrevTexture.Sample(gSampler, input.uv));
}
