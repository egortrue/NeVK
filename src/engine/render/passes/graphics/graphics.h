#pragma once

// Внутренние библиотеки
#include "passes/pass.h"

class GraphicsPass : public Pass {
 protected:
  virtual void init();
  virtual void reload();
  virtual void resize();
  virtual void destroy();

  virtual void createPipeline();

  // Шейдеры
  VkShaderModule vertexShader = VK_NULL_HANDLE;
  VkShaderModule fragmentShader = VK_NULL_HANDLE;
  void createShaderModules() override;

  // Состояние входных данных вершин
  VkVertexInputBindingDescription vertexBindingDescription;
  std::vector<VkVertexInputAttributeDescription> vertexAttributesDescription;
  virtual void setVertexBinding();
  virtual void setVertexAttributes();

  // Цель рендера
  std::vector<VkFramebuffer> framebuffers;
  VkFramebuffer createFramebuffer(std::vector<VkImageView>&, uint32_t width, uint32_t height);
  virtual void createFramebuffers() = 0;
};
