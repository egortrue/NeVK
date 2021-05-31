#pragma once

// Внутренние библиотеки
#include "window.h"
#include "core.h"
#include "commands.h"
#include "resources.h"
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

  // Менеджер ядра - экземпляры, устройства, поверхности вывода
  Core::Manager core;
  void initCore();
  void destroyCore();

  // Менеджер ресурсов - выделение и уничтожение буферов, изображений
  Resources::Manager resources;
  void initResources();
  void destroyResources();

  // Менеджер команд - генерация и хранение базовых команд устройства
  Commands::Manager commands;
  void initCommands();
  void destroyCommands();

  // Менеджер сцены - загрузка моделей, текстур, хранилище объектов
  Scene::Manager scene;
  void initScene();
  void destroyScene();

  // Менеджер рендеринга - генерация команд для отрисовки
  Render::Manager render;
  void initRender();
  void destroyRender();
};
