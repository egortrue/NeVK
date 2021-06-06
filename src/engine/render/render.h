#pragma once

// Внутренние библиотеки
#include "core.h"
#include "scene.h"

#include "shaders/shaders.h"
#include "frames/frames.h"
#include "passes/graphics/geometry.h"
#include "passes/graphics/postprocessing/fullscreen.h"
#include "passes/graphics/postprocessing/gui.h"

// Стандартные библиотеки
#include <chrono>
#include <string>
#include <vector>

class Render {
 public:
  typedef Render* Manager;

  Render(Window::Manager, Core::Manager, Scene::Manager);
  ~Render();

  void draw();

  void reloadSwapchain();
  void reloadShaders();

  GUI::Pass getInterface();

 private:
  Window::Manager window;
  Core::Manager core;
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
  // Постпроцессинг - улучшение изображения, добавление эффектов

  struct {
    Fullscreen::Pass TAA;  // Temporal Anti-Aliasing
    Fullscreen::Pass TM;   // Tone Maping
  } postprocess;

  void initPostProcess();
  void reinitPostProcess();
  void destroyPostProcess();

  //=========================================================================
  // Проход интерфейса - отрисовка меню управления

  struct {
    GUI::Pass pass;
  } interface;

  void initInterface();
  void reinitInterface();
  void destroyInterface();
};
