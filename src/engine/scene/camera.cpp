#include "camera.h"

void Camera::update() {
}

void Camera::setPosition(Position& position) {
  this->position.x = position.x;
  this->position.y = position.y;
  this->position.z = position.z;
  updatePosition();
}

void Camera::setProjection(Projection& projection) {
  this->projection.fov = projection.fov;
  this->projection.aspect = projection.aspect;
  this->projection.near = projection.near;
  this->projection.far = projection.far;
  updateProjection();
}

void Camera::updateProjection() {
  transform.projection = glm::perspective(
      glm::radians(projection.fov),
      projection.aspect,
      projection.near,
      projection.far);
}

void Camera::updatePosition() {
  bool moved = false;

  static auto prevTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  double deltaTime = std::chrono::duration<double, std::milli>(currentTime - prevTime).count() / 1000.0;
  prevTime = currentTime;

  float shift = speed.movement * deltaTime;
  float x = position.x;
  float y = position.y;
  float z = position.z;

  if (move.up) {
    position.y += shift;
    move.up = false;
    moved = true;
  }

  if (move.down) {
    position.y -= shift;
    move.down = false;
    moved = true;
  }

  if (move.left) {
    position.x -= shift;
    move.left = false;
    moved = true;
  }

  if (move.right) {
    position.x += shift;
    move.right = false;
    moved = true;
  }

  if (move.forward) {
    position.z += shift;
    move.forward = false;
    moved = true;
  }

  if (move.back) {
    position.z -= shift;
    move.back = false;
    moved = true;
  }

  if (moved) {
    glm::float3 offset = glm::float3(x - position.x, y - position.y, -(z - position.z));
    transform.view = glm::translate(transform.view, offset);
  }
}
