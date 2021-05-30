#pragma once

// Внутренние библиотеки
#include "core.h"
#include "resources.h"
#include "commands.h"
#include "shaders.h"
#include "scene.h"
#include "frames/frames.h"
#include "passes/graphics/geometry.h"
#include "passes/graphics/gui.h"

class Render {
 public:
  typedef Render* Manager;

  Render(Window::Manager, Core::Manager, Resources::Manager, Commands::Manager, Scene::Manager);
  ~Render();

  void reload();
  void draw();

  GUI::Pass getInterface();

 private:
  Window::Manager window;
  Core::Manager core;
  Resources::Manager resources;
  Commands::Manager commands;
  Scene::Manager scene;

  // Шейдеры - основа любого прохода рендера
  Shaders::Manager shaders;
  void initShaders();
  void destroyShaders();

  // Кадры - запись собранных команд рендера
  Frames::Manager frames;
  void initFrames();
  void destroyFrames();

  // Проход геометрии - основной механизм отрисовки объектов
  Geometry::Pass geometry;
  void initGeometry();
  void reloadGeometry();
  void destroyGeometry();

  // Пользовательский интерфейс - отрисовка меню управления
  GUI::Pass interface;
  void initInterface();
  void reloadInterface();
  void destroyInterface();
};
