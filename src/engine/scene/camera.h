#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Стандартные библиотеки
#include <chrono>

class Camera {
 public:
  typedef Camera* Manager;

  glm::float3 position = glm::float3(0, 0, 0);

  struct Rotation {
    float pitch;  // Поворот относительно X
    float yaw;    // Поворот относительно Y
    float roll;   // Поворот относительно Z
  } rotation;

  struct Projection {
    float fov;     // Угол обзора
    float aspect;  // Соотношение сторон (4:3, 16:9, ...)
    float near;    // Нижняя граница дальности видимости
    float far;     // Верхняя граница дальности видимости
  } projection;

  struct Transform {
    glm::float4x4 view;
    glm::float4x4 projection;
  } transform;

  struct Direction {
    glm::float3 upper = glm::vec3(0, 1, 0);
    glm::float3 front = glm::vec3(0, 0, -1);
    glm::float3 right = glm::vec3(1, 0, 0);
  } direction;

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

  void update(double deltaTime);
  void updatePosition(double deltaTime);

  void updateView();
  void updateProjection();

  void setProjection(Projection&);

  void rotate(double dx, double dy);
};
