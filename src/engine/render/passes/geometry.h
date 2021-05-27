#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "pass.h"
#include "textures.h"
#include "models.h"

// Стандартные библиотеки
#include <chrono>

class GeometryPass : public RenderPass {
 public:
  Textures::Manager textures;

  struct record_t {
    VkCommandBuffer cmd;
    uint32_t imageIndex;
    uint32_t indicesCount;
    VkBuffer indices;
    VkBuffer vertices;
  };

  void init() override;
  void resize() override;
  void destroy() override;
  void record(record_t&);

 private:
  void createRenderPass() override;
  void createGraphicsPipeline() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 public:
  std::string textureName;
  void updateUniformDescriptors(uint32_t imageIndex);

 private:
  // Буферы
  struct uniform_t {
    glm::float4x4 model;
    glm::float4x4 view;
    glm::float4x4 projection;
  } uniform;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  void createUniformDescriptors();
  void destroyUniformDescriptors();

  // Изображения
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
