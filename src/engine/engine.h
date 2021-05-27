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

  Core::Manager core;
  void initCore();
  void destroyCore();

  Resources::Manager resources;
  void initResources();
  void destroyResources();

  Commands::Manager commands;
  void initCommands();
  void destroyCommands();

  Shaders::Manager shaders;
  void initShaders();
  void destroyShaders();

  Textures::Manager textures;
  void initTextures();
  void destroyTextures();

  Models::Manager models;
  Models::Instance cube;
  void initModels();
  void destroyModels();

  std::vector<Frame> frames;
  uint32_t currentFrameIndex;
  void initFrames();
  void destroyFrames();

  GeometryPass geometryPass;
  void initGeometryPass();
  void destroyGeometryPass();
};
