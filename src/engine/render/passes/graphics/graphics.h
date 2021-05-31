#pragma once

// Внутренние библиотеки
#include "passes/pass.h"

// Стандартные библиотеки
#include <vector>
#include <string>

class GraphicsPass : public Pass {
 protected:
  virtual void init();
  virtual void destroy();
  virtual void reload();
  virtual void resize();
  virtual void update(uint32_t index) = 0;

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

 public:
  // Цель рендера
  struct {
    VkFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t count;
    std::vector<VkImageView> views;
  } targetImage;

 protected:
  std::vector<VkFramebuffer> framebuffers;
  VkFramebuffer createFramebuffer(std::vector<VkImageView>&, uint32_t width, uint32_t height);
  virtual void createFramebuffers() = 0;
};
