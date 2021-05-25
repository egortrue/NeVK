#pragma once

// Сторонние библиотеки
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Внутренние библиотеки
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
  std::string textureName;
  void updateDescriptorSets() override;

 private:
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  void createTextureImage();
  void destroyTextureImage();
  void createDescriptorSetLayout() override;

  //void updateUniformBuffer(uint32_t imageIndex, const glm::float4x4& perspective, const glm::float4x4& view);

  //   struct UniformBufferObject {
  //     alignas(16) glm::mat4 modelViewProj;
  //     alignas(16) glm::mat4 worldToView;
  //     alignas(16) glm::mat4 inverseWorldToView;
  //   };
  //   std::vector<VkBuffer> uniformBuffers;
  //   std::vector<VkDeviceMemory> uniformBuffersMemory;
  //void createUniformBuffers();

  //=========================================================================
  // Наборы изображений, в которые будет идти результат

 public:
  // Изображения теста глубины
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkFormat depthImageFormat;
  VkImageView depthImageView;
  void createDepthImage();
  void destroyDepthImage();

 private:
  void createFramebuffers() override;
};
