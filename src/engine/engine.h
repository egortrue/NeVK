#pragma once

// Сторонние библиотеки
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Внутренние библиотеки
#include "core/core.h"
#include "resources/resources.h"
#include "render/commands/commands.h"
#include "render/pass/geometry.h"

// Стандартные библиотеки
#include <array>

class Engine {
  struct Window {
    GLFWwindow* instance;
    VkSurfaceKHR surface;
    int width, height;
    std::vector<const char*> extensions;
  };

  struct Frame {
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;
    VkFence drawing;
    VkSemaphore available;
  };

 public:
  Engine(GLFWwindow*);
  ~Engine();

  void drawFrame();

 private:
  Window* window;
  void initWindow(GLFWwindow*);
  void destroyWindow();

  CoreManager core;
  void initCore();
  void destroyCore();

  ResourcesManager resources;
  void initResources();
  void destroyResources();

  CommandsManager commands;
  void initCommands();
  void destroyCommands();

  ShadersManager shaders;
  void initShaders();
  void destroyShaders();

  std::vector<Frame> frames;
  uint32_t currentFrameIndex;
  void initFrames();
  void destroyFrames();

  GeometryPass geometryPass;
  void initGeometryPass();
  void destroyGeometryPass();
};
