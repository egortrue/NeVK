#pragma once

// Внутренние библиотеки
#include "graphics.h"
#include "window.h"
#include "scene.h"

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

  struct init_t {
    Core::Manager core;
    Commands::Manager commands;
    Resources::Manager resources;
    Window::Manager window;
    Scene::Manager scene;
  };

  struct record_t {
    VkCommandBuffer cmd;
    uint32_t imageIndex;
  };

  void init(init_t&);
  void record(record_t&);
  void reload() override;
  void resize() override;
  void destroy() override;
  void update(uint32_t index) override;

 private:
  Window::Manager window;
  Scene::Manager scene;

  //=========================================================================
  // ImGUI

 public:
  struct {
    ImGuiIO io;
    ImGuiData data;
    ImGuiInit init;
    ImGuiStyle style;

    bool menuHovered;
  } imgui;

  enum {
    WINDOW_NORESIZE = ImGuiWindowFlags_NoResize,
    WINDOW_NOMOVE = ImGuiWindowFlags_NoMove,
    WINDOW_NODECOR = ImGuiWindowFlags_NoDecoration,
    WINDOW_AUTOSIZE = ImGuiWindowFlags_AlwaysAutoResize,
  };

 private:
  void updateUI();

  //=========================================================================

  void createRenderPass() override;
  void createFramebuffers() override;

  void createDescriptorSetsLayout() override {}
  void createDescriptorSets() override {}
  void updateDescriptorSets() override {}

  VkVertexInputBindingDescription getVertexBinding() override { return {}; }
  std::vector<VkVertexInputAttributeDescription> getVertexAttributes() override { return {}; }
  VkPushConstantRange getPushConstantRange() override { return {}; }
};
