#include "fullscreen.h"

void Fullscreen::init(init_t& data) {
  this->core = data.core;
  this->commands = data.commands;
  this->resources = data.resources;
  this->shaders = data.shaders;
  this->shaderName = data.shaderName;

  GraphicsPass::init();
}

void Fullscreen::reload() {
  GraphicsPass::reload();
}

void Fullscreen::resize() {
  updateDescriptorSets();
  GraphicsPass::resize();
}

void Fullscreen::destroy() {
  GraphicsPass::destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Fullscreen::record(record_t& data) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipeline.pass;
  renderPassInfo.framebuffer = framebuffers[data.imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = {targetImage.width, targetImage.height};

  // Заливка цвета вне всех примитивов
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.8f, 0.3f, 0.0f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

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

  // Операция рендера
  vkCmdDraw(data.cmd, 3, 1, 0, 0);

  vkCmdEndRenderPass(data.cmd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

VkVertexInputBindingDescription Fullscreen::getVertexBinding() {
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(float) * 3;
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Fullscreen::getVertexAttributes() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

  VkVertexInputAttributeDescription attributeDescription{};
  attributeDescription.binding = 0;
  attributeDescription.location = 0;
  attributeDescription.offset = 0;
  attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions.emplace_back(attributeDescription);

  return attributeDescriptions;
}

VkPushConstantRange Fullscreen::getPushConstantRange() {
  VkPushConstantRange pushConstant{};
  pushConstant.offset = 0;
  pushConstant.size = sizeof(float);
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  return pushConstant;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Fullscreen::createFramebuffers() {
  framebuffers.resize(targetImage.count);
  for (uint32_t i = 0; i < targetImage.count; ++i) {
    std::vector<VkImageView> attachment = {targetImage.views[i]};
    framebuffers[i] = createFramebuffer(attachment, targetImage.width, targetImage.height);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Fullscreen::createRenderPass() {
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
  // Подпроходы рендера

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Цветовые подключения
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

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

  std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
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

void Fullscreen::createDescriptorSetsLayout() {
  VkDescriptorSetLayoutBinding textureImageLayout{};
  textureImageLayout.binding = 0;
  textureImageLayout.descriptorCount = 1;
  textureImageLayout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  textureImageLayout.pImmutableSamplers = nullptr;
  textureImageLayout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding textureSamplerLayout{};
  textureSamplerLayout.binding = 1;
  textureSamplerLayout.descriptorCount = 1;
  textureSamplerLayout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  textureSamplerLayout.pImmutableSamplers = nullptr;
  textureSamplerLayout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
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

void Fullscreen::createDescriptorSets() {
  descriptorSets.resize(targetImage.count);
  for (size_t i = 0; i < targetImage.count; ++i)
    descriptorSets[i] = resources->createDesciptorSet(descriptorSetsLayout[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Fullscreen::updateDescriptorSets() {
  for (size_t i = 0; i < targetImage.count; ++i) {
    //=========================================================================
    // Инициализация ресурсов

    VkDescriptorImageInfo colorImageInfo{};
    colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colorImageInfo.imageView = colorImageView;

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = colorImageSampler;

    //=========================================================================
    // Запись ресурсов

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &colorImageInfo;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &samplerInfo;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

    vkUpdateDescriptorSets(core->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}
