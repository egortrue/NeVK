#pragma once

// Внутренние библиотеки
#include "window.h"
#include "core.h"
#include "scene.h"
#include "render.h"

// Стандартные библиотеки
#include <string>
#include <vector>

class Engine {
 public:
  typedef Engine* Manager;

  explicit Engine(Window::Manager);
  ~Engine();

  Scene::Manager getScene();
  Render::Manager getRender();

 private:
  Window::Manager window;

  // Менеджер ядра - устройства, ресурсы, базовые команды
  Core::Manager core;
  void initCore();
  void destroyCore();

  // Менеджер сцены - загрузка моделей, текстур, хранилище объектов
  Scene::Manager scene;
  void initScene();
  void destroyScene();

  // Менеджер рендеринга - генерация команд для отрисовки
  Render::Manager render;
  void initRender();
  void destroyRender();
};
