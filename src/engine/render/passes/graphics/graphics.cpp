#include "graphics.h"

void GraphicsPass::init() {
  setVertexBinding();
  setVertexAttributes();
  Pass::init();
  createFramebuffers();
}

void GraphicsPass::destroy() {
  for (auto framebuffer : framebuffers)
    vkDestroyFramebuffer(core->device, framebuffer, nullptr);
  Pass::destroy();
}

void GraphicsPass::reload() {
  for (auto framebuffer : framebuffers)
    vkDestroyFramebuffer(core->device, framebuffer, nullptr);
  Pass::reload();
  createFramebuffers();
}

void GraphicsPass::resize() {
  for (auto framebuffer : framebuffers)
    vkDestroyFramebuffer(core->device, framebuffer, nullptr);
  Pass::resize();
  createFramebuffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsPass::createShaderModules() {
  // Сохраним старые шейдеры, если такие есть
  VkShaderModule oldVS = vertexShader, oldFS = fragmentShader;

  try {
    // Попытка (пере)компиляции новых шейдеров в SPIR-V
    Shaders::Instance vertexInstance = shaders->loadShader(shaderName, std::string("vertexMain"), SLANG_STAGE_VERTEX);
    Shaders::Instance fragmentInstance = shaders->loadShader(shaderName, std::string("fragmentMain"), SLANG_STAGE_FRAGMENT);

    // Подключение модулей
    vertexShader = vertexInstance->module;
    fragmentShader = fragmentInstance->module;

    // Удаление старых модулей
    if (oldVS != VK_NULL_HANDLE && oldFS != VK_NULL_HANDLE) {
      vkDestroyShaderModule(core->device, oldVS, nullptr);
      vkDestroyShaderModule(core->device, oldFS, nullptr);
    }

    std::cout << "Shader \"" << shaderName << "\" was loaded successfully" << std::endl;

  } catch (std::exception& error) {
    // Выведем ошибку компиляции шейдера
    std::cerr << error.what();

    // Вернём старые модули
    if (oldVS != VK_NULL_HANDLE && oldFS != VK_NULL_HANDLE) {
      vertexShader = oldVS;
      fragmentShader = oldFS;
    } else {
      throw std::runtime_error("ERROR: shader was never loaded: " + shaderName);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

VkFramebuffer GraphicsPass::createFramebuffer(std::vector<VkImageView>& attachment, uint32_t width, uint32_t height) {
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = pipeline.pass;

  // Изображения, в которые будет идти результат
  framebufferInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
  framebufferInfo.pAttachments = attachment.data();
  framebufferInfo.width = width;
  framebufferInfo.height = height;
  framebufferInfo.layers = 1;

  VkFramebuffer framebuffer;
  if (vkCreateFramebuffer(core->device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create framebuffer!");

  return framebuffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsPass::setVertexBinding() {
  vertexBindingDescription.binding = 0;
  vertexBindingDescription.stride = sizeof(float) * 3;
  vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void GraphicsPass::setVertexAttributes() {
  vertexAttributesDescription.clear();
  VkVertexInputAttributeDescription attributeDescription;
  attributeDescription.binding = 0;
  attributeDescription.location = 0;
  attributeDescription.offset = 0;
  attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexAttributesDescription.emplace_back(attributeDescription);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsPass::createPipeline() {
  // Вершинный шейдер - обрабатывает одну вершину за раз
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertexShader;
  vertShaderStageInfo.pName = "main";

  // Фрагментный шейдер - получает растеризованый примитив и выдаёт его цвет
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragmentShader;
  fragShaderStageInfo.pName = "main";

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
      vertShaderStageInfo,
      fragShaderStageInfo,
  };

  //=================================================================================
  // Размещение геометрических данных в памяти

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributesDescription.size());
  vertexInputInfo.pVertexAttributeDescriptions = vertexAttributesDescription.data();

  //=================================================================================
  // Входная сборка - группирование вершин в примитивы

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // Вершины группируются в тройки -> треугольник
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  //=================================================================================
  // Преобразование области вывода

  // Область вывода
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = core->swapchain.extent.height;
  viewport.width = core->swapchain.extent.width;
  viewport.height = -core->swapchain.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Область вывода для теста ножниц
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {core->swapchain.extent.width, core->swapchain.extent.height};

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  //=================================================================================
  // Растеризация - преобразование примитивов для фрагментного шейдера

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  // Полное отключение растеризации
  rasterizer.rasterizerDiscardEnable = VK_FALSE;

  // Режим отрисовки примитивов
  // VK_POLYGON_MODE_FILL --- Полный режим - треугольники, заполненные сплошным цветом
  // VK_POLYGON_MODE_LINE --- Каркасный режим - отрезки, образующие треугольник
  // VK_POLYGON_MODE_POINT --- Точечный режим - несвязанные точки
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

  // Толщина каждого отрезка в пикселах (только для каркасного режима)
  rasterizer.lineWidth = 1.0f;  // 1.0f - обязательно по умолчанию для всех режимов

  // Лицевая сторона полигона (треугольника)
  // VK_FRONT_FACE_CLOCKWISE --- если полигон отрисован по часовой
  // VK_FRONT_FACE_COUNTER_CLOCKWISE --- если полигон отрисован против часовой
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  // Отброс сторон полигонов (треугольника)
  // VK_CULL_MODE_NONE --- ничего не отбрасывать
  // VK_CULL_MODE_FRONT_BIT --- отброс лицевых полигонов
  // VK_CULL_MODE_BACK_BIT --- отброс нелицевых полигонов
  // VK_CULL_MODE_FRONT_AND_BACK --- отброс обоих сторон полигонов
  rasterizer.cullMode = VK_CULL_MODE_NONE;

  // Опции глубины
  rasterizer.depthClampEnable = VK_FALSE;  // Отсечение глубины - заполнение дыр геометрии
  rasterizer.depthBiasEnable = VK_FALSE;   // Смещение глубины

  //=================================================================================
  // Мультисэмплинг - создание образцов для каждого пиксела

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  //=================================================================================
  // Тесты изображений

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  // Тест глубины
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

  // Тест трафарета
  depthStencil.stencilTestEnable = VK_FALSE;

  //=================================================================================
  // Цветовые подключения

  // Смешивание цветов
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

  // Операции между фрагментным шейдером и цветовым подключением
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  //=================================================================================
  // Динамические части конвейера

  std::vector<VkDynamicState> states = {
      VK_DYNAMIC_STATE_VIEWPORT,  // Изменение области вывода - vkCmdSetViewport()
  };

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = states.size();
  dynamicState.pDynamicStates = states.data();

  //=================================================================================
  // Создание конвейера

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetsLayout.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetsLayout.data();

  if (vkCreatePipelineLayout(core->device, &pipelineLayoutInfo, nullptr, &pipeline.layout) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create pipeline layout!");

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = shaderStages.size();
  pipelineInfo.pStages = shaderStages.data();
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipeline.layout;
  pipelineInfo.renderPass = pipeline.pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(core->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.instance) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create graphics pipeline!");
}
