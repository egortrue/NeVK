#pragma once

// Внутренние библиотеки
#include "window.h"
#include "scene.h"

#include "passes/graphics/graphics.h"

// Сторонние библиотеки
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

// Стандартные библиотеки
#include <vector>

typedef ImGui_ImplVulkan_InitInfo ImGuiInit;
typedef ImGui_ImplVulkanH_Window ImGuiData;

class GUI : public GraphicsPass {
 public:
  typedef GUI* Pass;
  Window::Manager window;
  Scene::Manager scene;

  void init() override;
  void destroy() override;
  void reload() override;
  void resize() override;

  void update(uint32_t index);
  void record(uint32_t index, VkCommandBuffer);

  //=========================================================================
  // ImGUI

 public:
  struct {
    bool menuHovered;
    bool taaON;
  } options;

 private:
  struct {
    ImGuiIO io;
    ImGuiData data;
    ImGuiInit init;
    ImGuiStyle style;
  } imgui;

  enum {
    WINDOW_NORESIZE = ImGuiWindowFlags_NoResize,
    WINDOW_NOMOVE = ImGuiWindowFlags_NoMove,
    WINDOW_NODECOR = ImGuiWindowFlags_NoDecoration,
    WINDOW_AUTOSIZE = ImGuiWindowFlags_AlwaysAutoResize,
  };

  void updateUI();

  //=========================================================================
  // Большую часть работы при создании конвейера ImGUI делает сам

  void createRenderPass() override;
  void createFramebuffers() override;

  void createDescriptorLayouts() override {}
  void createDescriptorSets() override {}
  void updateDescriptorSets() override {}

  VkVertexInputBindingDescription getVertexBinding() override { return {}; }
  std::vector<VkVertexInputAttributeDescription> getVertexAttributes() override { return {}; }
  VkPushConstantRange getPushConstantRange() override { return {}; }
};
