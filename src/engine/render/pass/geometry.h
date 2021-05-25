#pragma once

#include "pass.h"

class GeometryPass : public RenderPass {
 public:
  void init() override;
  void resize() override;
  void destroy() override;

  struct RecordData {
    VkCommandBuffer cmd;
    uint32_t imageIndex;
    uint32_t indicesCount;
    VkBuffer indices;
    VkBuffer vertices;
  };
  void record(RecordData&);

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
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

 private:
  //   std::vector<VkBuffer> uniformBuffers;
  //   std::vector<VkDeviceMemory> uniformBuffersMemory;

  void createDescriptorSetLayout() override;
  //void createUniformBuffers();

  //=========================================================================
  // Наборы изображений, в которые будет идти результат

 public:
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
  VkFormat depthImageFormat;

 private:
  void createFramebuffers() override;
};
