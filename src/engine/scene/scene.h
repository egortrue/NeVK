#pragma once

// Внутренние библиотеки
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "models.h"
#include "textures.h"
#include "camera.h"

// Стандартные библиотеки
#include <list>
#include <string>

class Object {
 public:
  typedef Object* Instance;

  Models::Instance model;
  Textures::Instance texture;
  glm::float3 position = glm::float3(0, 0, 0);

  struct Rotation {
    float pitch;  // Поворот относительно X
    float yaw;    // Поворот относительно Y
    float roll;   // Поворот относительно Z
  } rotation;

  struct Transform {
    glm::float4x4 model;
  } transform;
};

class Scene {
 public:
  typedef Scene* Manager;

  Scene(Core::Manager, Commands::Manager, Resources::Manager);
  ~Scene();

  Camera::Manager getCamera();

  std::list<Object::Instance> objects;
  void loadObject(const char* model, const char* texture);
  void loadObject(const std::string& model, const std::string& texture);

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
