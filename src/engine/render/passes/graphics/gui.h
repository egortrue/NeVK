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

 private:
  void loadFonts();
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

static bool mousePressed[2] = {false, false};

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if (action == GLFW_PRESS && button >= 0 && button < 2)
    mousePressed[button] = true;
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseWheel += static_cast<float>(yoffset);  // Use fractional mouse wheel, 1.0 unit 5 lines.
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  ImGuiIO& io = ImGui::GetIO();
  if (action == GLFW_PRESS)
    io.KeysDown[key] = true;
  if (action == GLFW_RELEASE)
    io.KeysDown[key] = false;
  io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
  io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c) {
  if (c > 0 && c < 0x10000)
    ImGui::GetIO().AddInputCharacter(static_cast<uint16_t>(c));
}
