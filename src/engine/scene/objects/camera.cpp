#include "camera.h"

void Camera::update(float deltaTime) {
  updatePosition(deltaTime);
  updateDirections();
  updateView();
}

void Camera::updateView() {
  viewMatrix = glm::lookAt(
      transform.position,                    // Позция камеры
      transform.position + direction.front,  // Позиция цели
      direction.upper);
}

void Camera::updateProjection() {
  projectionMatrix = glm::perspective(
      glm::radians(projection.fov),
      projection.aspect,
      projection.near,
      projection.far);
}

void Camera::updateProjection(Projection& projection) {
  this->projection.fov = projection.fov;
  this->projection.aspect = projection.aspect;
  this->projection.near = projection.near;
  this->projection.far = projection.far;
  updateProjection();
}

void Camera::updateRotation(double xpos, double ypos) {
  // Смещение мыши
  double xoffset = xpos - mouse.pos.x;
  double yoffset = mouse.pos.y - ypos;

  // Новые координаты мыши
  mouse.pos.x = xpos;
  mouse.pos.y = ypos;

  // Запишем градуcы поворота
  transform.rotation.y += xoffset * speed.rotation;
  transform.rotation.x += yoffset * speed.rotation;

  // Лимиты поворота
  if (transform.rotation.x > 89.0f)
    transform.rotation.x = 89.0f;
  if (transform.rotation.x < -89.0f)
    transform.rotation.x = -89.0f;

  // Нормализация градусной величины
  transform.rotation.x = transform.rotation.x - 360 * (static_cast<int>(transform.rotation.x) / 360);
  transform.rotation.y = transform.rotation.y - 360 * (static_cast<int>(transform.rotation.y) / 360);
  transform.rotation.z = transform.rotation.z - 360 * (static_cast<int>(transform.rotation.z) / 360);

  updateDirections();
  updateView();
}

void Camera::updateDirections() {
  // Обновим векторы направления движения (вектор вверх всегда константен)
  glm::float3 front;
  front.x = sin(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x));
  front.y = sin(glm::radians(transform.rotation.x));
  front.z = -cos(glm::radians(transform.rotation.y)) * cos(glm::radians(transform.rotation.x));
  direction.front = glm::normalize(front);
  direction.right = glm::normalize(glm::cross(direction.front, direction.upper));
}

void Camera::updatePosition(float deltaTime) {
  if (!(move.left || move.right || move.up || move.down || move.forward || move.back)) {
    return;
  }

  float shift = speed.movement * deltaTime;

  if (move.up)
    transform.position += shift * direction.upper;

  if (move.down)
    transform.position -= shift * direction.upper;

  if (move.right)
    transform.position += shift * direction.right;

  if (move.left)
    transform.position -= shift * direction.right;

  if (move.forward)
    transform.position += shift * direction.front;

  if (move.back)
    transform.position -= shift * direction.front;
}
