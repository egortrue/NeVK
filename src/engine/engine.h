#pragma once

// Сторонние библиотеки
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Внутренние библиотеки
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "shaders.h"
#include "textures.h"
#include "models.h"
#include "render/passes/geometry.h"

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

  TexturesManager textures;
  void initTextures();
  void destroyTextures();

  ModelsManager models;
  void initModels();
  void destroyModels();

  VkBuffer vertexBuffer, indexBuffer;
  VkDeviceMemory vertexBufferMemory, indexBufferMemory;
  void createVertexBuffer();
  void createIndexBuffer();

  std::vector<Frame> frames;
  uint32_t currentFrameIndex;
  void initFrames();
  void destroyFrames();

  GeometryPass geometryPass;
  void initGeometryPass();
  void destroyGeometryPass();
};
