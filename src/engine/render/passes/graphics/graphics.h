#pragma once

// Внутренние библиотеки
#include "passes/pass.h"

// Стандартные библиотеки
#include <vector>
#include <string>

class GraphicsPass : public Pass {
 public:
  virtual void init();
  virtual void destroy();
  virtual void reload();
  virtual void resize();

  //=========================================================================
  // Обработчики конвейера и прохода рендера

 protected:
  virtual void createPipeline();

  // Опции графического конвейера
  virtual VkVertexInputBindingDescription getVertexBinding() = 0;
  virtual std::vector<VkVertexInputAttributeDescription> getVertexAttributes() = 0;
  virtual VkPushConstantRange getPushConstantRange() = 0;

  //=========================================================================
  // Шейдеры - ядро любого прохода

  VkShaderModule vertexShader = VK_NULL_HANDLE;
  VkShaderModule fragmentShader = VK_NULL_HANDLE;

  void createShaderModules() override;

  //=========================================================================
  // Фреймбуфер - целевой объект графического рендера

 public:
  struct {
    VkFormat format;
    uint32_t width, height;
    std::vector<VkImageView> views;
  } target;

 protected:
  std::vector<VkFramebuffer> framebuffers;

  virtual void createFramebuffers() = 0;
  VkFramebuffer createFramebuffer(std::vector<VkImageView>&, uint32_t width, uint32_t height);

  //=========================================================================
};
