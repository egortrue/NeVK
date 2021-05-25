#pragma once

// Внутренние библиотеки
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "shaders.h"
#include "textures.h"

// Стандартные библиотеки
#include <array>
#include <string>
#include <vector>

class RenderPass {
 public:
  CoreManager core;
  CommandsManager commands;
  ResourcesManager resources;
  ShadersManager shaders;

  virtual void init() = 0;
  virtual void resize() = 0;
  virtual void destroy();

  //=========================================================================
  // Конвейер и проходы рендера

 protected:
  VkRenderPass renderPass;
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  virtual void createRenderPass() = 0;
  virtual void createGraphicsPipeline() = 0;

  //=========================================================================
  // Шейдеры - подпрограммы конвейера

 public:
  std::string shaderName;
  void reloadShader();

 protected:
  VkShaderModule vertexShader = VK_NULL_HANDLE;
  VkShaderModule fragmentShader = VK_NULL_HANDLE;
  VkShaderModule createModule(const char* code, uint32_t codeSize);
  void createShaderModules();

  //=========================================================================
  // Множества ресурсов, привязанные к конвейеру

 protected:
  std::vector<VkDescriptorSet> descriptorSets;  // Множества ресурсов
  VkDescriptorSetLayout descriptorSetLayout;    // Раскладка каждого множества
  void createDescriptorSets();
  virtual void createDescriptorSetLayout() = 0;
  virtual void updateDescriptorSets() = 0;

  //=========================================================================
  // Наборы изображений, в которые будет идти результат

 public:
  uint32_t targetImageCount;
  uint32_t targetImageWidth, targetImageHeight;
  VkFormat targetImageFormat;
  std::vector<VkImageView> targetImageViews;

 protected:
  std::vector<VkFramebuffer> framebuffers;
  void createFramebuffer(std::vector<VkImageView>&, uint32_t index);
  virtual void createFramebuffers() = 0;
};