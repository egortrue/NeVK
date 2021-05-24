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
  VkImageView textureImageView;
  VkSampler textureSampler;
  void updateDescriptorSets() override;

 private:
  void createDescriptorSetLayout() override;

  //=========================================================================
  // Uniform-буферы - ресурсы, привязанные к конвейеру

  //   struct UniformBufferObject {
  //     alignas(16) glm::mat4 modelViewProj;
  //     alignas(16) glm::mat4 worldToView;
  //     alignas(16) glm::mat4 inverseWorldToView;
  //   };
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  //void updateUniformBuffer(uint32_t imageIndex, const glm::float4x4& perspective, const glm::float4x4& view);
  void createUniformBuffers();

  //=========================================================================
  // Наборы изображений, в которые будет идти результат
 public:
  VkFormat depthBufferFormat;

 private:
  void createFramebuffers(std::vector<VkImageView>& imageViews, VkImageView& depthImageView);
};
