#pragma once

// Внутренние библиотеки
#include "graphics.h"

// Стандартные библиотеки
#include <vector>
#include <string>

class Fullscreen : public GraphicsPass {
 public:
  typedef Fullscreen* Pass;

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
  };

  VkImageView colorImageView;   // ~ Texture2D
  VkSampler colorImageSampler;  // ~ SamplerState

  void init(init_t&);
  void record(record_t&);
  void reload() override;
  void resize() override;
  void destroy() override;
  void update(uint32_t index) override {}

  VkVertexInputBindingDescription getVertexBinding() override;
  std::vector<VkVertexInputAttributeDescription> getVertexAttributes() override;
  VkPushConstantRange getPushConstantRange() override;
  void createRenderPass() override;

  void createDescriptorSetsLayout() override;
  void createDescriptorSets() override;
  void updateDescriptorSets() override;

  void createFramebuffers() override;
};
