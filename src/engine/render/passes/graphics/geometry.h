#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "graphics.h"
#include "textures.h"
#include "models.h"

// Стандартные библиотеки
#include <chrono>

class GeometryPass : public GraphicsPass {
 public:
  struct init_t {
    Core::Manager core;
    Commands::Manager commands;
    Resources::Manager resources;
    Shaders::Manager shaders;
    Models::Manager models;
    Textures::Manager textures;

    std::string shaderName;
  };

  struct record_t {
    VkCommandBuffer cmd;
    uint32_t imageIndex;
    uint32_t indicesCount;
    VkBuffer indices;
    VkBuffer vertices;
  };

  void init(init_t&);
  void record(record_t&);
  void reload();
  void resize();
  void destroy();

 private:
  Models::Manager models;
  Textures::Manager textures;

  //=========================================================================
  // Конвейер и проход рендера

  void setVertexBinding() override;
  void setVertexAttributes() override;
  void createRenderPass() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 private:
  void createDescriptorSetsLayout() override;
  void createDescriptorSets() override;
  void updateDescriptorSets() override;

 public:
  // Текстуры
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
  void updateUniformDescriptors(uint32_t imageIndex, glm::float4x4& modelViewProj);

  //=========================================================================
  // Наборы изображений для фреймбуфера

 public:
  // Буферы цвета
  VkFormat colorImageFormat;
  uint32_t colorImageWidth;
  uint32_t colorImageHeight;
  uint32_t colorImageCount;
  std::vector<VkImageView> colorImageViews;

 private:
  // Буфер глубины
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkFormat depthImageFormat;
  VkImageView depthImageView;

  void createDepthImage();
  void destroyDepthImage();

  void createFramebuffers() override;
};
