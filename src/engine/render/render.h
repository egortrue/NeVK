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

  struct {
    Geometry::Pass pass;
    struct {
      VkFormat format;
      uint32_t width, height;
      std::vector<VkImageView> views;

      std::vector<VkImage> images;
      std::vector<VkDeviceMemory> memory;
    } data;
  } geometry;

  void initGeometry();
  void reinitGeometry();
  void destroyGeometry();

  void createGeometryData();
  void destroyGeometryData();

  //=========================================================================
  // Проход полноэкранного треугольника - промежуточная стадия

  struct {
    Fullscreen::Pass pass;
    struct {
      VkFormat format;
      uint32_t width, height;
      std::vector<VkImageView> views;
    } data;
  } fullscreen;

  void initFullscreen();
  void reinitFullscreen();
  void destroyFullscreen();

  void createFullscreenData();
  void destroyFullscreenData();

  //=========================================================================
  // Проход интерфейса - отрисовка меню управления

  struct {
    GUI::Pass pass;
    struct {
      VkFormat format;
      uint32_t width, height;
      std::vector<VkImageView> views;
    } data;
  } interface;

  void initInterface();
  void reinitInterface();
  void destroyInterface();

  void createInterfaceData();
  void destroyInterfaceData();
};
