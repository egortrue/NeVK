#pragma once

// Сторонние библиотеки
#include <glm/gtx/compatibility.hpp>

// Стандартные библиотеки
#include <chrono>

class Camera {
 public:
  typedef Camera* Manager;

  enum Type {
    LOOK_AT,
    FIRST_PERSON
  } type;

  struct Position {
    float x;
    float y;
    float z;
  } position;

  struct Projection {
    float fov;     // Угол обзора
    float aspect;  // Соотношение сторон (4:3, 16:9, ...)
    float near;    // Нижняя граница дальности видимости
    float far;     // Верхняя граница дальности видимости
  } projection;

  struct Transform {
    glm::float4x4 view;        // Соответствует данным позиции
    glm::float4x4 projection;  // Соответствует данным проекции
  } transform;

  struct Speed {
    float movement = 2.5f;
    float rotation = 0.5f;
  } speed;

  struct Move {
    bool left;
    bool right;
    bool up;
    bool down;
    bool forward;
    bool back;
  } move;

  Camera() = default;
  ~Camera() = default;

  void update();
  void updatePosition(double deltaTime);
  void updateProjection();

  void setPosition(Position&);
  void setProjection(Projection&);
};
