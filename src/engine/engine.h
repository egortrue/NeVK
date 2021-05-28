#pragma once

// Внутренние библиотеки
#include "window.h"
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "shaders.h"
#include "scene.h"
#include "render/passes/geometry.h"

// Стандартные библиотеки
#include <string>
#include <vector>

class Engine {
 public:
  typedef Engine* Manager;

  Engine(Window::Manager);
  ~Engine();
  void drawFrame();

  Scene::Manager getScene();

 private:
  Window::Manager window;

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

  Scene::Manager scene;
  void initScene();
  void destroyScene();

  //=======================

  struct Frame {
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;
    VkFence drawing;
    VkFence showing;
    VkSemaphore imageAvailable;
    VkSemaphore imageRendered;
  };

  std::vector<Frame> frames;
  uint32_t currentFrameIndex;
  void initFrames();
  void destroyFrames();

  GeometryPass geometryPass;
  void initGeometryPass();
  void destroyGeometryPass();

  void resizeSwapchain();
};
