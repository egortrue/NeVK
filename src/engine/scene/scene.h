#pragma once

// Внутренние библиотеки
#include "core.h"
#include "objects/object.h"
#include "objects/camera.h"
#include "resources/textures.h"
#include "resources/models.h"

// Стандартные библиотеки
#include <list>
#include <vector>
#include <string>

class Scene {
 public:
  typedef Scene* Manager;

  Scene(Core::Manager);
  ~Scene();

  uint32_t currentObject = 0;
  std::vector<PhysicalObject::Instance> objects;
  void loadObject(const char* model);
  void loadObject(const std::string& model);

  Camera::Manager getCamera();
  Textures::Manager getTextures();

 private:
  Core::Manager core;

  Camera::Manager camera;
  void initCamera();
  void destroyCamera();

  Textures::Manager textures;
  void initTextures();
  void destroyTextures();

  Models::Manager models;
  void initModels();
  void destroyModels();
};
