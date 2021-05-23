#pragma once

// Сторонние зависимости
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Внутренние зависимости
#include "core/core.h"
#include "resources/resources.h"
#include "render/commands.h"

// Стандартные зависимости
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
  };

 public:
  Engine(GLFWwindow*);
  ~Engine();

 private:
  Window* window;
  void initWindow(GLFWwindow*);
  void destroyWindow();

  Core* core;
  void initCore();
  void destroyCore();

  Resources* resources;
  void initResources();
  void destroyResources();

  Commands* commands;
  void initCommands();
  void destroyCommands();

  std::vector<Frame> frames;
  void initFrames();
  void destroyFrames();
  void drawFrame();
};
