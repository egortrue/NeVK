#include "geometry.h"

void Geometry::init() {
  createDepthImage();
  createUniformDescriptors();
  GraphicsPass::init();
}

void Geometry::update(uint32_t index) {
  updateUniformDescriptors(index);
}

void Geometry::reload() {
  destroyDepthImage();
  destroyUniformDescriptors();
  createDepthImage();
  createUniformDescriptors();
  GraphicsPass::reload();
}

void Geometry::resize() {
  destroyDepthImage();
  createDepthImage();
  GraphicsPass::resize();
}

void Geometry::destroy() {
  GraphicsPass::destroy();
  destroyDepthImage();
  destroyUniformDescriptors();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::record(uint32_t index, VkCommandBuffer cmd) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipeline.pass;
  renderPassInfo.framebuffer = framebuffers[index];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = {target.width, target.height};

  // Заливка цвета вне всех примитивов
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.2f, 0.3f, 0.3f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  // Новое разрешение вывода
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = static_cast<float>(target.height);
  viewport.width = static_cast<float>(target.width);
  viewport.height = -static_cast<float>(target.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  // Подключение конвейера и настройка его динамических частей
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.instance);
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  // Подключение множества ресурсов, используемых в конвейере
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptor.sets[index], 0, nullptr);

  int i = 0;
  for (auto object : scene->objects) {
    // Буферы вершин
    VkBuffer vertexBuffers[] = {object->model->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, object->model->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    instance.objectModel = object->modelMatrix;
    instance.objectTexture = i++;
    vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(instance_t), &instance);

    // Операция рендера
    vkCmdDrawIndexed(cmd, object->model->verticesCount, 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(cmd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::createUniformDescriptors() {
  uint32_t count = target.views.size();
  VkDeviceSize bufferSize = sizeof(uniform_t);
  uniformBuffers.resize(count);
  uniformBuffersMemory.resize(count);

  for (uint32_t i = 0; i < count; ++i) {
    resources->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffers[i], uniformBuffersMemory[i]);
  }
}

void Geometry::destroyUniformDescriptors() {
  for (uint32_t i = 0; i < target.views.size(); ++i)
    resources->destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
}

void Geometry::updateUniformDescriptors(uint32_t imageIndex) {
  // Скопируем данне структуры в памяти устройства
  void* data;
  vkMapMemory(core->device, uniformBuffersMemory[imageIndex], 0, sizeof(uniform_t), 0, &data);
  memcpy(data, &uniform, sizeof(uniform_t));
  vkUnmapMemory(core->device, uniformBuffersMemory[imageIndex]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::createDepthImage() {
  depth.format = resources->findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  resources->createImage(
      core->swapchain.extent.width,
      core->swapchain.extent.height,
      depth.format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      depth.image, depth.memory);

  depth.view = resources->createImageView(
      depth.image,
      depth.format,
      VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Geometry::destroyDepthImage() {
  resources->destroyImageView(depth.view);
  resources->destroyImage(depth.image, depth.memory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::createFramebuffers() {
  framebuffers.resize(target.views.size());
  for (uint32_t i = 0; i < target.views.size(); ++i) {
    std::vector<VkImageView> attachment = {target.views[i], depth.view};
    framebuffers[i] = createFramebuffer(attachment, target.width, target.height);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

VkVertexInputBindingDescription Geometry::getVertexBinding() {
  // Описание структур, содержащихся в вершинном буфере
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;                        // Уникальный id
  bindingDescription.stride = sizeof(Models::vertex_t);  // Расстояние между началами структур (размер структуры)
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Geometry::getVertexAttributes() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

  // Описание членов структур, содержащихся в вершинном буфере
  VkVertexInputAttributeDescription attributeDescription{};
  attributeDescription.binding = 0;                                    // Уникальный id структуры
  attributeDescription.location = 0;                                   // Уникальный id для каждого члена структуры
  attributeDescription.offset = offsetof(Models::vertex_t, position);  // Смещение от начала структуры
  attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions.emplace_back(attributeDescription);

  attributeDescription.binding = 0;                              // Уникальный id структуры
  attributeDescription.location = 1;                             // Уникальный id для каждого члена структуры
  attributeDescription.offset = offsetof(Models::vertex_t, uv);  // Смещение от начала структуры
  attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions.emplace_back(attributeDescription);

  return attributeDescriptions;
}

VkPushConstantRange Geometry::getPushConstantRange() {
  VkPushConstantRange pushConstant{};
  pushConstant.offset = 0;
  pushConstant.size = sizeof(instance_t);
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  return pushConstant;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::createRenderPass() {
  //=================================================================================
  // Описание цветового подключения - выходного изображения конвейера

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = target.format;

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
  depthAttachment.format = depth.format;

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

void Geometry::createDescriptorLayouts() {
  VkDescriptorSetLayoutBinding uniformLayout{};
  uniformLayout.binding = 0;
  uniformLayout.descriptorCount = 1;
  uniformLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniformLayout.pImmutableSamplers = nullptr;
  uniformLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding textureImageLayout{};
  textureImageLayout.binding = 1;
  textureImageLayout.descriptorCount = static_cast<uint32_t>(textureImageViews.size());
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

  descriptor.layouts.push_back(layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::createDescriptorSets() {
  descriptor.sets.resize(target.views.size());
  for (size_t i = 0; i < target.views.size(); ++i)
    descriptor.sets[i] = resources->createDesciptorSet(descriptor.layouts[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Geometry::updateDescriptorSets() {
  for (size_t i = 0; i < target.views.size(); ++i) {
    //=========================================================================
    // Инициализация ресурсов
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(uniform_t);

    std::vector<VkDescriptorImageInfo> imageInfo(textureImageViews.size());
    for (uint32_t textureID = 0; textureID < textureImageViews.size(); ++textureID) {
      imageInfo[textureID].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo[textureID].imageView = textureImageViews[textureID];
    }

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
    descriptorWrites[0].dstSet = descriptor.sets[i];
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorCount = static_cast<uint32_t>(textureImageViews.size());
    descriptorWrites[1].pImageInfo = imageInfo.data();
    descriptorWrites[1].dstSet = descriptor.sets[i];
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &samplerInfo;
    descriptorWrites[2].dstSet = descriptor.sets[i];
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

    vkUpdateDescriptorSets(core->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
