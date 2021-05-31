#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "graphics.h"
#include "textures.h"
#include "models.h"

// Стандартные библиотеки
#include <chrono>
#include <vector>
#include <string>

class Geometry : public GraphicsPass {
 public:
  typedef Geometry* Pass;

  struct init_t {
    Core::Manager core;
    Commands::Manager commands;
    Resources::Manager resources;
    Shaders::Manager shaders;
    std::string shaderName;
  };

  struct record_t {
    VkCommandBuffer cmd;
    uint32_t imageIndex;
    uint32_t objectID;
    VkBuffer indices;
    uint32_t indicesCount;
    VkBuffer vertices;
  };

  void init(init_t&);
  void record(record_t&);
  void reload() override;
  void resize() override;
  void destroy() override;
  void update(uint32_t index) override;

 private:
  Models::Manager models;
  Textures::Manager textures;

  //=========================================================================
  // Конвейер и проход рендера

 private:
  void setVertexBinding() override;
  void setVertexAttributes() override;
  void createRenderPass() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 public:
  struct uniform_t {
    glm::float4x4 modelViewProj;
  } uniform;                                   // ~ cbuffer
  VkSampler textureSampler;                    // ~ SamplerState
  std::vector<VkImageView> textureImageViews;  // ~ Texture2D

 private:
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;

  void createUniformDescriptors();
  void destroyUniformDescriptors();
  void updateUniformDescriptors(uint32_t index);

  void createDescriptorSetsLayout() override;
  void createDescriptorSets() override;
  void updateDescriptorSets() override;

  //=========================================================================
  // Наборы изображений для фреймбуфера

 private:
  struct {
    VkImage instance;
    VkDeviceMemory memory;
    VkFormat format;
    VkImageView view;
  } depthImage;
  void createDepthImage();
  void destroyDepthImage();

  void createFramebuffers() override;
};
