#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "object.h"

class Camera : public Object {
 public:
  typedef Camera* Manager;

  struct Direction {
    glm::float3 right = glm::float3(1, 0, 0);
    glm::float3 upper = glm::float3(0, 1, 0);
    glm::float3 front = glm::float3(0, 0, -1);
  } direction;

  struct Projection {
    float fov;     // Угол обзора
    float aspect;  // Соотношение сторон (4:3, 16:9, ...)
    float near;    // Нижняя граница дальности видимости
    float far;     // Верхняя граница дальности видимости
  } projection;

  glm::float4x4 viewMatrix;
  glm::float4x4 projectionMatrix;

  struct Speed {
    float movement = 2.5f;
    float rotation = 0.25f;
  } speed;

  struct Move {
    bool left;
    bool right;
    bool up;
    bool down;
    bool forward;
    bool back;
  } move;

  struct Mouse {
    glm::double2 pos;
    bool right;
    bool left;
    bool middle;
  } mouse;

  Camera() = default;
  ~Camera() = default;

  void update(float deltaTime);
  void updatePosition(float deltaTime);
  void updateRotation(double dx, double dy);

  void updateDirections();

  void updateView();
  void updateProjection();
  void updateProjection(Projection&);
};
