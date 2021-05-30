#include "geometry.h"

void GeometryPass::init(init_t& data) {
  this->core = data.core;
  this->commands = data.commands;
  this->resources = data.resources;
  this->shaders = data.shaders;
  this->shaderName = data.shaderName;

  createDepthImage();
  createUniformDescriptors();
  GraphicsPass::init();
}

void GeometryPass::update(uint32_t index) {
  updateUniformDescriptors(index);
}

void GeometryPass::reload() {
  destroyDepthImage();
  destroyUniformDescriptors();
  createDepthImage();
  createUniformDescriptors();
  GraphicsPass::reload();
}

void GeometryPass::resize() {
  destroyDepthImage();
  createDepthImage();
  GraphicsPass::resize();
}

void GeometryPass::destroy() {
  GraphicsPass::destroy();
  destroyDepthImage();
  destroyUniformDescriptors();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::record(record_t& data) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipeline.pass;
  renderPassInfo.framebuffer = framebuffers[data.imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = {targetImage.width, targetImage.height};

  // Заливка цвета вне всех примитивов
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.2f, 0.3f, 0.3f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  // Новое разрешение вывода
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = (float)targetImage.height;
  viewport.width = (float)targetImage.width;
  viewport.height = -(float)targetImage.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdBeginRenderPass(data.cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  // Подключение конвейера и настройка его динамических частей
  vkCmdBindPipeline(data.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.instance);
  vkCmdSetViewport(data.cmd, 0, 1, &viewport);

  // Подключение множества ресурсов, используемых в конвейере
  vkCmdBindDescriptorSets(data.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptorSets[data.imageIndex], 0, nullptr);

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
  uniformBuffers.resize(targetImage.count);
  uniformBuffersMemory.resize(targetImage.count);

  for (uint32_t i = 0; i < targetImage.count; ++i) {
    resources->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffers[i], uniformBuffersMemory[i]);
  }
}

void GeometryPass::destroyUniformDescriptors() {
  for (uint32_t i = 0; i < targetImage.count; ++i)
    resources->destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
}

void GeometryPass::updateUniformDescriptors(uint32_t imageIndex) {
  // Скопируем данне структуры в памяти устройства
  void* data;
  vkMapMemory(core->device, uniformBuffersMemory[imageIndex], 0, sizeof(uniform_t), 0, &data);
  memcpy(data, &uniform, sizeof(uniform_t));
  vkUnmapMemory(core->device, uniformBuffersMemory[imageIndex]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createDepthImage() {
  depthImage.format = resources->findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  resources->createImage(
      core->swapchainExtent.width,
      core->swapchainExtent.height,
      depthImage.format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      depthImage.instance, depthImage.memory);

  depthImage.view = resources->createImageView(
      depthImage.instance,
      depthImage.format,
      VK_IMAGE_ASPECT_DEPTH_BIT);
}

void GeometryPass::destroyDepthImage() {
  resources->destroyImageView(depthImage.view);
  resources->destroyImage(depthImage.instance, depthImage.memory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createFramebuffers() {
  framebuffers.resize(targetImage.count);
  for (uint32_t i = 0; i < targetImage.count; ++i) {
    std::vector<VkImageView> attachment = {targetImage.views[i], depthImage.view};
    framebuffers[i] = createFramebuffer(attachment, targetImage.width, targetImage.height);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::setVertexBinding() {
  // Описание структур, содержащихся в вершинном буфере
  vertexBindingDescription.binding = 0;                        // Уникальный id
  vertexBindingDescription.stride = sizeof(Models::vertex_t);  // Расстояние между началами структур (размер структуры)
  vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void GeometryPass::setVertexAttributes() {
  // Описание членов структур, содержащихся в вершинном буфере
  VkVertexInputAttributeDescription attributeDescription{};
  attributeDescription.binding = 0;                                    // Уникальный id структуры
  attributeDescription.location = 0;                                   // Уникальный id для каждого члена структуры
  attributeDescription.offset = offsetof(Models::vertex_t, position);  // Смещение от начала структуры
  attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexAttributesDescription.emplace_back(attributeDescription);

  attributeDescription.binding = 0;                              // Уникальный id структуры
  attributeDescription.location = 1;                             // Уникальный id для каждого члена структуры
  attributeDescription.offset = offsetof(Models::vertex_t, uv);  // Смещение от начала структуры
  attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
  vertexAttributesDescription.emplace_back(attributeDescription);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createRenderPass() {
  //=================================================================================
  // Описание цветового подключения - выходного изображения конвейера

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = targetImage.format;

  // Действия при работе с изображением
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  // Действия при работе с трафаретом
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Раскладка изображения
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;               // Не устанавливается автоматически
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // Устанавливается автоматически в конце прохода

  // Мультисэмплинг
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // Число образцов (1 = выкл)

  // Элемент подпрохода
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  //=================================================================================
  // Описание изобржения глубины

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = depthImage.format;

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

  if (vkCreateRenderPass(core->device, &renderPassInfo, nullptr, &pipeline.pass) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create render pass!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createDescriptorSetsLayout() {
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

  VkDescriptorSetLayout layout;
  if (vkCreateDescriptorSetLayout(core->device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create descriptor set layout!");

  descriptorSetsLayout.push_back(layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeometryPass::createDescriptorSets() {
  descriptorSets.resize(targetImage.count);
  for (size_t i = 0; i < targetImage.count; ++i)
    descriptorSets[i] = resources->createDesciptorSet(descriptorSetsLayout[0]);
}

void GeometryPass::updateDescriptorSets() {
  for (size_t i = 0; i < targetImage.count; ++i) {
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
