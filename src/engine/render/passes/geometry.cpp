#include "geometry.h"

void GeometryPass::init() {
  // Подготовка данных вывода
  createDepthImage();

  // Подготовка подкючаемых ресурсов
  createUniformDescriptors();
  createDescriptorSetLayout();
  createDescriptorSets();
  updateDescriptorSets();

  RenderPass::init();
}

void GeometryPass::destroy() {
  RenderPass::destroy();

  destroyDepthImage();
  destroyUniformDescriptors();
}

void GeometryPass::resize() {
  for (auto framebuffer : framebuffers)
    vkDestroyFramebuffer(core->device, framebuffer, nullptr);
  destroyDepthImage();

  vkDestroyPipeline(core->device, pipeline, nullptr);
  vkDestroyPipelineLayout(core->device, pipelineLayout, nullptr);
  vkDestroyRenderPass(core->device, renderPass, nullptr);

  createDepthImage();
  createRenderPass();
  createGraphicsPipeline();
  createFramebuffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::record(record_t& data) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = framebuffers[data.imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = {colorImageWidth, colorImageHeight};

  // Заливка цвета вне всех примитивов
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.2f, 0.3f, 0.3f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  // Новое разрешение вывода
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = (float)colorImageHeight;
  viewport.width = (float)colorImageWidth;
  viewport.height = -(float)colorImageHeight;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdBeginRenderPass(data.cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  // Подключение конвейера и настройка его динамических частей
  vkCmdBindPipeline(data.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdSetViewport(data.cmd, 0, 1, &viewport);
  vkCmdSetLineWidth(data.cmd, 1.0f);

  // Подключение множества ресурсов, используемых в конвейере
  vkCmdBindDescriptorSets(data.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[data.imageIndex], 0, nullptr);

  // Буферы вершин
  VkBuffer vertexBuffers[] = {data.vertices};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(data.cmd, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(data.cmd, data.indices, 0, VK_INDEX_TYPE_UINT32);

  // Операция рендера
  vkCmdDrawIndexed(data.cmd, data.indicesCount, 1, 0, 0, 0);

  vkCmdEndRenderPass(data.cmd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createUniformDescriptors() {
  VkDeviceSize bufferSize = sizeof(uniform_t);
  uniformBuffers.resize(colorImageCount);
  uniformBuffersMemory.resize(colorImageCount);

  for (uint32_t i = 0; i < colorImageCount; ++i) {
    resources->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffers[i], uniformBuffersMemory[i]);
  }
}

void GeometryPass::destroyUniformDescriptors() {
  for (uint32_t i = 0; i < colorImageCount; ++i)
    resources->destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
}

void GeometryPass::updateUniformDescriptor(uint32_t imageIndex, glm::float4x4& modelViewProj) {
  // Обновим данные структуры
  uniform.modelViewProj = modelViewProj;

  // Скопируем данные в память устройства
  void* data;
  vkMapMemory(core->device, uniformBuffersMemory[imageIndex], 0, sizeof(uniform_t), 0, &data);
  memcpy(data, &uniform, sizeof(uniform_t));
  vkUnmapMemory(core->device, uniformBuffersMemory[imageIndex]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createDepthImage() {
  depthImageFormat = resources->findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  resources->createImage(
      core->swapchainExtent.width,
      core->swapchainExtent.height,
      depthImageFormat,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      depthImage, depthImageMemory);

  depthImageView = resources->createImageView(
      depthImage,
      depthImageFormat,
      VK_IMAGE_ASPECT_DEPTH_BIT);
}

void GeometryPass::destroyDepthImage() {
  resources->destroyImageView(depthImageView);
  resources->destroyImage(depthImage, depthImageMemory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createFramebuffers() {
  framebuffers.resize(colorImageCount);
  for (uint32_t i = 0; i < colorImageCount; ++i) {
    std::vector<VkImageView> attachment = {colorImageViews[i], depthImageView};
    createFramebuffer(attachment, i);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createGraphicsPipeline() {
  //=================================================================================
  // Используемые шейдеры
  // - обязательны хотя бы один вершинный и один фрагментный шейдеры

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

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  //=================================================================================
  // Размещение геометрических данных в памяти

  // Описание структур, содержащихся в вершинном буфере
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;                        // Уникальный id
  bindingDescription.stride = sizeof(Models::vertex_t);  // Расстояние между началами структур (размер структуры)
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  // Описание членов структур, содержащихся в вершинном буфере
  VkVertexInputAttributeDescription positionAttributeDescription{};
  positionAttributeDescription.binding = 0;                                    // Уникальный id структуры
  positionAttributeDescription.location = 0;                                   // Уникальный id для каждого члена структуры
  positionAttributeDescription.offset = offsetof(Models::vertex_t, position);  // Смещение от начала структуры
  positionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

  VkVertexInputAttributeDescription uvAttributeDescription{};
  uvAttributeDescription.binding = 0;                              // Уникальный id структуры
  uvAttributeDescription.location = 1;                             // Уникальный id для каждого члена структуры
  uvAttributeDescription.offset = offsetof(Models::vertex_t, uv);  // Смещение от начала структуры
  uvAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;

  std::array<VkVertexInputAttributeDescription, 2> attributesDesc = {
      positionAttributeDescription,
      uvAttributeDescription,
  };

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributesDesc.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributesDesc.data();

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
  viewport.y = (float)colorImageHeight;
  viewport.width = (float)colorImageWidth;
  viewport.height = -(float)colorImageHeight;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Область вывода для теста ножниц
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {colorImageWidth, colorImageHeight};

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

  VkDynamicState states[] = {
      VK_DYNAMIC_STATE_VIEWPORT,    // Изменение области вывода - vkCmdSetViewport()
      VK_DYNAMIC_STATE_LINE_WIDTH,  // Изменение толщины отрезков примитивов - vkCmdSetLineWidth()
  };

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = states;

  //=================================================================================
  // Создание конвейера

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  if (vkCreatePipelineLayout(core->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create pipeline layout!");

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(core->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create graphics pipeline!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createRenderPass() {
  //=================================================================================
  // Описание цветового подключения - выходного изображения конвейера

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = colorImageFormat;

  // Действия при работе с изображением
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  // Действия при работе с трафаретом
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Раскладка изображения
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Не устанавливается автоматически
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Устанавливается автоматически в конце прохода

  // Мультисэмплинг
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // Число образцов (1 = выкл)

  // Элемент подпрохода
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  //=================================================================================
  // Описание изобржения глубины

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = depthImageFormat;

  // Действия при работе с изображением
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Действия при работе с трафаретом
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Раскладка изображения
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                       // Не устанавливается автоматически
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;  // Устанавливается автоматически в конце прохода

  // Мультисэмплинг
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // Число образцов (1 = выкл)

  // Элемент подпрохода
  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  //=================================================================================
  // Подпроходы рендера

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Цветовые подключения
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  // Подключение глубины-трафарета (только одно)
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  //=================================================================================
  // Зависимости подпроходов рендера

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  //=================================================================================
  // Создание прохода рендера

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(core->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create render pass!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uniformLayout{};
  uniformLayout.binding = 0;
  uniformLayout.descriptorCount = 1;
  uniformLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniformLayout.pImmutableSamplers = nullptr;
  uniformLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding textureImageLayout{};
  textureImageLayout.binding = 1;
  textureImageLayout.descriptorCount = 1;
  textureImageLayout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  textureImageLayout.pImmutableSamplers = nullptr;
  textureImageLayout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding textureSamplerLayout{};
  textureSamplerLayout.binding = 2;
  textureSamplerLayout.descriptorCount = 1;
  textureSamplerLayout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  textureSamplerLayout.pImmutableSamplers = nullptr;
  textureSamplerLayout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
      uniformLayout,
      textureImageLayout,
      textureSamplerLayout,
  };

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(core->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create descriptor set layout!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::updateDescriptorSets() {
  for (size_t i = 0; i < colorImageCount; ++i) {
    //=========================================================================
    // Инициализация ресурсов
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(uniform_t);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = textureSampler;

    //=========================================================================
    // Запись ресурсов

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &samplerInfo;
    descriptorWrites[2].dstSet = descriptorSets[i];
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

    vkUpdateDescriptorSets(core->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
