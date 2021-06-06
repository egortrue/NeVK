#pragma once

// Внутренние библиотеки
#include "models.h"
#include "textures.h"

class Object {
 public:
  struct Transform {
    glm::float3 position = glm::float3(0, 0, 0);
    glm::float3 rotation = glm::float3(0, 0, 0);
    glm::float3 scale = glm::float3(1, 1, 1);
  } transform;

  glm::float4x4 modelMatrix = glm::mat4(1.0f);

  virtual void setPosition(glm::float3 position);
  virtual void setRotation(glm::float3 rotation);
  virtual void setScale(glm::float3 scale);

  virtual void translate(glm::float3 position);
  virtual void rotate(glm::float3 rotation);
  virtual void scale(glm::float3 scale);

  virtual void update();
};

class PhysicalObject : public Object {
 public:
  typedef PhysicalObject* Instance;

  Models::Instance model;
  Textures::Instance textures;
};
