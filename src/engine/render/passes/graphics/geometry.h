#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "graphics.h"
#include "scene.h"

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
    Scene::Manager scene;
  };

  void init(init_t&);
  void record(record_t&);
  void reload() override;
  void resize() override;
  void destroy() override;
  void update(uint32_t index) override;

  //=========================================================================
  // Конвейер и проход рендера

 private:
  // Состояние входных данных вершин
  VkVertexInputBindingDescription getVertexBinding() override;
  std::vector<VkVertexInputAttributeDescription> getVertexAttributes() override;

  // Константы шейдера
  VkPushConstantRange getPushConstantRange() override;

  void createRenderPass() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 public:
  // ~ ConstantBuffer
  struct constants_t {
    glm::float4x4 objectModel;
    uint32_t objectTexture;
  } instance;

  // ~ cbuffer
  struct uniform_t {
    glm::float4x4 cameraView;
    glm::float4x4 cameraProjection;
    glm::float4x4 model;
  } uniform;

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
