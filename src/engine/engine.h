#pragma once

// Внутренние библиотеки
#include "window.h"
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "shaders.h"
#include "textures.h"
#include "models.h"
#include "camera.h"
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
  Camera::Manager getCamera();

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

  Textures::Manager textures;
  void initTextures();
  void destroyTextures();

  Models::Manager models;
  Models::Instance cube;
  void initModels();
  void destroyModels();

  Camera::Manager camera;
  void initCamera();
  void destroyCamera();

  //=======================

  struct Frame {
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;
    VkFence drawing;
    VkSemaphore available;
  };

  std::vector<Frame> frames;
  uint32_t currentFrameIndex;
  void initFrames();
  void destroyFrames();

  GeometryPass geometryPass;
  void initGeometryPass();
  void destroyGeometryPass();
};
