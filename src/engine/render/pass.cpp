#include "pass.h"

void RenderPass::reloadShader() {
  vkDeviceWaitIdle(core->device);
  vkDestroyPipeline(core->device, pipeline, nullptr);
  vkDestroyPipelineLayout(core->device, pipelineLayout, nullptr);
  createShaderModules();
  createGraphicsPipeline();
}

void RenderPass::destroy() {
  vkDeviceWaitIdle(core->device);
  for (auto framebuffer : framebuffers)
    vkDestroyFramebuffer(core->device, framebuffer, nullptr);

  vkDestroyPipeline(core->device, pipeline, nullptr);
  vkDestroyPipelineLayout(core->device, pipelineLayout, nullptr);
  vkDestroyRenderPass(core->device, renderPass, nullptr);
  vkDestroyShaderModule(core->device, vertexShader, nullptr);
  vkDestroyShaderModule(core->device, fragmentShader, nullptr);
  vkDestroyDescriptorSetLayout(core->device, descriptorSetLayout, nullptr);
}

void RenderPass::createShaderModules() {
  // Сохраним старые шейдеры, если такие есть
  VkShaderModule oldVS = vertexShader, oldFS = fragmentShader;

  try {
    // Попытка (пере)компиляции новых шейдеров в SPIR-V
    uint32_t vertId = shaders->loadShader(shaderName.c_str(), "vertexMain", SLANG_STAGE_VERTEX);
    uint32_t fragId = shaders->loadShader(shaderName.c_str(), "fragmentMain", SLANG_STAGE_FRAGMENT);

    // Получение SPIR-V
    const char* vertShaderCode = nullptr;
    uint32_t vertShaderCodeSize = 0;
    shaders->getShaderCode(vertId, vertShaderCode, vertShaderCodeSize);
    const char* fragShaderCode = nullptr;
    uint32_t fragShaderCodeSize = 0;
    shaders->getShaderCode(fragId, fragShaderCode, fragShaderCodeSize);

    // Создание модулей
    vertexShader = createModule(vertShaderCode, vertShaderCodeSize);
    fragmentShader = createModule(fragShaderCode, fragShaderCodeSize);

    // Удаление старых модулей
    if (oldVS != VK_NULL_HANDLE && oldFS != VK_NULL_HANDLE) {
      vkDestroyShaderModule(core->device, oldVS, nullptr);
      vkDestroyShaderModule(core->device, oldFS, nullptr);
    }

    std::cout << "Shader \"" << shaderName << "\" was loaded successfully" << std::endl;
  } catch (std::exception& error) {
    // Выведем ошибку компиляции шейдера
    std::cerr << std::endl;
    std::cerr << error.what();

    // Вернём старые модули
    if (oldVS != VK_NULL_HANDLE && oldFS != VK_NULL_HANDLE) {
      vertexShader = oldVS;
      fragmentShader = oldFS;
    } else {
      throw std::runtime_error("ERROR: No shaders loaded");
    }
  }
}

VkShaderModule RenderPass::createModule(const char* code, const uint32_t codeSize) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = codeSize;
  createInfo.pCode = (uint32_t*)code;

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(core->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create shader module!");

  return shaderModule;
}

void RenderPass::createDescriptorSets() {
  // На каждое множество ресурсов - своя раскладка
  // В контексте одного конвейера - у всех одинаковая
  std::vector<VkDescriptorSetLayout> layouts(targetImageCount, descriptorSetLayout);

  // Информация о множестве ресурсов
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

  // Пул, из которого они будут выделены
  allocInfo.descriptorPool = resources->descriptorPool;

  // Количество множеств и их раскладка соответственно
  allocInfo.descriptorSetCount = static_cast<uint32_t>(targetImageCount);
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(targetImageCount);
  if (vkAllocateDescriptorSets(core->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to allocate descriptor sets!");
}

void RenderPass::createFramebuffer(std::vector<VkImageView>& attachment, uint32_t index) {
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = renderPass;

  // Изображения, в которые будет идти результат
  framebufferInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
  framebufferInfo.pAttachments = attachment.data();
  framebufferInfo.width = targetImageWidth;
  framebufferInfo.height = targetImageHeight;
  framebufferInfo.layers = 1;

  if (vkCreateFramebuffer(core->device, &framebufferInfo, nullptr, &framebuffers[index]) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create framebuffer!");
}
