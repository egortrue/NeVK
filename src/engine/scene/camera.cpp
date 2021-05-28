#include "camera.h"

void Camera::update() {
}

void Camera::setPosition(Position& position) {
  this->position.x = position.x;
  this->position.y = position.y;
  this->position.z = position.z;
  updatePosition(1);
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

void Camera::updatePosition(double deltaTime) {
  if (!(move.left || move.right || move.up || move.down || move.forward || move.back))
    return;

  float shift = speed.movement * deltaTime;
  float x = position.x;
  float y = position.y;
  float z = position.z;

  if (move.up)
    position.y += shift;

  if (move.down)
    position.y -= shift;

  if (move.right)
    position.x += shift;

  if (move.left)
    position.x -= shift;

  if (move.forward)
    position.z += shift;

  if (move.back)
    position.z -= shift;

  glm::float3 offset = glm::float3(x - position.x, y - position.y, -(z - position.z));
  transform.view *= glm::translate(glm::float4x4(1.0f), offset);
}
