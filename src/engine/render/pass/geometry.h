#pragma once

#include "pass.h"

class GeometryPass : public RenderPass {
 public:
  void init() override;
  void resize() override;
  void destroy() override;

 private:
  void createRenderPass() override;
  void createGraphicsPipeline() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 public:
  void updateDescriptorSets() override;
  //void updateUniformBuffer(uint32_t imageIndex, const glm::float4x4& perspective, const glm::float4x4& view);

  // Описание ресурсов
  //   struct UniformBufferObject {
  //     alignas(16) glm::mat4 modelViewProj;
  //     alignas(16) glm::mat4 worldToView;
  //     alignas(16) glm::mat4 inverseWorldToView;
  //   };
  VkImageView textureImageView;
  VkSampler textureSampler;

 private:
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;

  void createDescriptorSetLayout() override;
  void createUniformBuffers();

  //=========================================================================
  // Наборы изображений, в которые будет идти результат

 public:
  VkFormat depthBufferFormat;

 private:
  void createFramebuffers(std::vector<VkImageView>& imageViews, VkImageView& depthImageView);
};
