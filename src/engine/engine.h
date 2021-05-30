#pragma once

// Внутренние библиотеки
#include "window.h"
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "shaders.h"
#include "scene.h"
#include "render/frames/frames.h"
#include "render/passes/graphics/geometry.h"
#include "render/passes/graphics/gui.h"

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

  Frames::Manager frames;
  void initFrames();
  void destroyFrames();

  //=======================

  GeometryPass* geometryPass;
  void initGeometryPass();
  void destroyGeometryPass();

  GUIPass* guiPass;
  void initGUIPass();
  void destroyGUIPass();

  void resizeSwapchain();
};
