#pragma once

// Внутренние библиотеки
#include "core.h"
#include "resources.h"
#include "commands.h"
#include "shaders.h"
#include "scene.h"
#include "frames/frames.h"
#include "passes/graphics/geometry.h"
#include "passes/graphics/fullscreen.h"
#include "passes/graphics/gui.h"

// Стандартные библиотеки
#include <string>
#include <vector>

class Render {
 public:
  typedef Render* Manager;

  Render(Window::Manager, Core::Manager, Resources::Manager, Commands::Manager, Scene::Manager);
  ~Render();

  void draw();

  void reloadSwapchain();
  void reloadShaders();

  GUI::Pass getInterface();

 private:
  Window::Manager window;
  Core::Manager core;
  Resources::Manager resources;
  Commands::Manager commands;
  Scene::Manager scene;

  //=========================================================================

  // Шейдеры - основа любого прохода рендера
  Shaders::Manager shaders;
  void initShaders();
  void destroyShaders();

  // Кадры - запись собранных команд рендера
  Frames::Manager frames;
  void initFrames();
  void destroyFrames();

  //=========================================================================
  // Проход геометрии - основной механизм отрисовки объектов

  Geometry::Pass geometry;
  void initGeometry();
  void reinitGeometry();
  void destroyGeometry();

  struct {
    VkFormat format;
    uint32_t width, height;
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    std::vector<VkDeviceMemory> memory;
  } geometryData;
  void createGeometryData();
  void destroyGeometryData();

  //=========================================================================
  // Проход полноэкранного треугольника - промежуточная стадия

  Fullscreen::Pass fullscreen;
  void initFullscreen();
  void reinitFullscreen();
  void destroyFullscreen();

  struct {
    VkSampler sampler;
    std::vector<VkImageView> views;
  } fullscreenData;
  void createFullscreenData();
  void destroyFullscreenData();

  //=========================================================================
  // Проход интерфейса - отрисовка меню управления

  GUI::Pass interface;
  void initInterface();
  void reinitInterface();
  void destroyInterface();

  struct {
    std::vector<VkImageView> views;
  } interfaceData;
  void createInterfaceData();
  void destroyInterfaceData();
};
