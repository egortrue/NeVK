#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "graphics.h"
#include "scene.h"

// Стандартные библиотеки
#include <vector>
#include <string>

class Geometry : public GraphicsPass {
 public:
  typedef Geometry* Pass;
  Scene::Manager scene;

  void init() override;
  void destroy() override;
  void reload() override;
  void resize() override;

  void update(uint32_t index);
  void record(uint32_t index, VkCommandBuffer);

  //=========================================================================
  // Обработчики конвейера и прохода рендера

 private:
  void createRenderPass() override;

  VkVertexInputBindingDescription getVertexBinding() override;
  std::vector<VkVertexInputAttributeDescription> getVertexAttributes() override;
  VkPushConstantRange getPushConstantRange() override;

  //=========================================================================
  // Выделенные ресурсы, привязанные к конвейеру

 public:
  // ~ ConstantBuffer
  struct instance_t {
    glm::float4x4 objectModel;
    uint32_t objectTexture;
  } instance;

  // ~ cbuffer
  struct uniform_t {
    glm::float4x4 cameraView;
    glm::float4x4 cameraProjection;
  } uniform;

  // ~ Texture2D
  std::vector<VkImageView> textureImageViews;

  // ~ SamplerState
  VkSampler textureSampler;

 private:
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;

  void createUniformDescriptors();
  void destroyUniformDescriptors();
  void updateUniformDescriptors(uint32_t index);

  void createDescriptorLayouts() override;
  void createDescriptorSets() override;
  void updateDescriptorSets() override;

  //=========================================================================
  // Фреймбуфер - целевой объект графического рендера

 private:
  void createFramebuffers() override;

  // Изображение для теста глубины
  struct {
    VkImage image;
    VkImageView view;
    VkFormat format;
    VkDeviceMemory memory;
  } depth;
  void createDepthImage();
  void destroyDepthImage();

  //=========================================================================
};
