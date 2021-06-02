#pragma once

// Внутренние библиотеки
#include "passes/graphics/graphics.h"

// Стандартные библиотеки
#include <vector>
#include <string>

class Fullscreen : public GraphicsPass {
 public:
  typedef Fullscreen* Pass;

  void init() override;
  void destroy() override;
  void reload() override;
  void resize() override;

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
    uint32_t colorImageIndex;
  } instance;

  std::vector<VkImageView> colorImageViews;  // ~ Texture2D
  VkSampler colorImageSampler;               // ~ SamplerState

 private:
  void createDescriptorLayouts() override;
  void createDescriptorSets() override;
  void updateDescriptorSets() override;

  //=========================================================================
  // Фреймбуфер - целевой объект графического рендера

  void createFramebuffers() override;

  //=========================================================================
};
