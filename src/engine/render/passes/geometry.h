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
  void destroy() override;
  void resize() override;
  void record(record_t&);

 private:
  void createRenderPass() override;
  void createGraphicsPipeline() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 private:
  // Описание подключаемых ресурсов
  void createDescriptorSetLayout() override;
  void updateDescriptorSets() override;

 public:
  // Текстуры - посутпают извне прохода
  VkImageView textureImageView;
  VkSampler textureSampler;

 private:
  // Буферы
  struct uniform_t {
    glm::float4x4 modelViewProj;
  } uniform;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  void createUniformDescriptors();
  void destroyUniformDescriptors();

 public:
  void updateUniformDescriptor(uint32_t imageIndex, glm::float4x4& modelViewProj);

  //=========================================================================
  // Наборы изображений, в которые будет идти результат

 private:
  // Буфер глубины - создаётся локально
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkFormat depthImageFormat;
  VkImageView depthImageView;
  void createDepthImage();
  void destroyDepthImage();

  void createFramebuffers();
};
