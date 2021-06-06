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

// Ресурсы, привязанные к конвейеру
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
    // Индексы кадров
    int currImageIndex = instance.imageIndex;
    int prevImageIndex = (currImageIndex == 0) ? 1 : currImageIndex - 1;

    // Изображение кадров
    Texture2D currImage = images[NonUniformResourceIndex(currImageIndex)];
    Texture2D prevImage = images[NonUniformResourceIndex(prevImageIndex)];

    // Размер кадра
    uint2 imageDim;
    uint imageLevels;
    currImage.GetDimensions(0, imageDim.x, imageDim.y, imageLevels);

    // Позиция кадра
    int2 currPos = int2(fragment.uv * imageDim);

    // Цвет кадров
    float4 currImageColor = currImage.Load(int3(currPos, 0));
    float4 prevImageColor = prevImage.Load(int3(currPos, 0));

    float3 finalColor = currImageColor.rgb;

    // Смешивание текущего и предыдущего кадров
    // finalColor += prevImageColor.rgb;
    // finalColor /= 2.0f;

    // Субпиксельное дрожание 
    {
        const float step = 1.2f;
        const float2 offset[8] = { 
            {-step, -step}, 
            {-step,  step},
            { step, -step},
            { step,  step}, 
            {-step,  0},
            { step,  0},
            { 0, -step}, 
            { 0,  step},
        };

        // Смешивание всех изображений со сдвигом
        float3 centerColor = currImageColor.rgb;
        for (int i = 0; i < 8; ++i)
        {
            // Смотрим соседние точки
            float3 neighborColor = currImage.Load(int3(currPos + offset[i], 0)).rgb;

            // Смотрим разницу
            float3 diffColor = abs(neighborColor - centerColor);
            if (length(diffColor.yz) < 0.35f) 
                neighborColor = centerColor;
            finalColor += neighborColor;
        }
        finalColor /= 8.0f;
    }


    return float4(finalColor, 1.0f);
    //return currImageColor;
}
