#pragma once

// Внутренние библиотеки
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "object.h"
#include "camera.h"

// Стандартные библиотеки
#include <list>
#include <vector>
#include <string>

class Scene {
 public:
  typedef Scene* Manager;

  Scene(Core::Manager, Commands::Manager, Resources::Manager);
  ~Scene();

  uint32_t currentObject = 0;
  std::vector<PhysicalObject::Instance> objects;
  void loadObject(const char* model, const char* texture);
  void loadObject(const std::string& model, const std::string& texture);

  Camera::Manager getCamera();

 private:
  Core::Manager core;
  Commands::Manager commands;
  Resources::Manager resources;

  Textures::Manager textures;
  void initTextures();
  void destroyTextures();

  Models::Manager models;
  void initModels();
  void destroyModels();

  Camera::Manager camera;
  void initCamera();
  void destroyCamera();
};
