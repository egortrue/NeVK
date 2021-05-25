#pragma once

// Внутренние библиотеки
#include "pass.h"

class GeometryPass : public RenderPass {
 public:
  struct RecordData {
    VkCommandBuffer cmd;
    uint32_t imageIndex;
    uint32_t indicesCount;
    VkBuffer indices;
    VkBuffer vertices;
  };

  void init() override;
  void resize() override;
  void destroy() override;
  void record(RecordData&);

 private:
  void createRenderPass() override;
  void createGraphicsPipeline() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 public:
  TexturesManager textures;
  std::string textureName;

 private:
  VkImageView textureImageView;
  VkSampler textureSampler;
  void createTextureDescriptors();
  void destroyTextureDescriptors();

  void createDescriptorSetLayout() override;
  void updateDescriptorSets() override;

  //=========================================================================
  // Наборы изображений, в которые будет идти результат

 public:
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkFormat depthImageFormat;
  VkImageView depthImageView;

 private:
  void createDepthImageFramebuffer();
  void destroyDepthImageFramebuffer();

  void createFramebuffers() override;
};
