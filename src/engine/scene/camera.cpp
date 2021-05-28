#include "camera.h"

void Camera::update(double deltaTime) {
  updatePosition(deltaTime);
  updateView();
}

void Camera::updateView() {
  transform.view = glm::lookAt(
      position,                    // Позция камеры
      position + direction.front,  // Позиция цели
      direction.upper);
}

void Camera::updateProjection() {
  transform.projection = glm::perspective(
      glm::radians(projection.fov),
      projection.aspect,
      projection.near,
      projection.far);
}

void Camera::rotate(double xpos, double ypos) {
  // Смещение мыши
  double xoffset = xpos - mouse.pos.x;
  double yoffset = mouse.pos.y - ypos;

  // Новые координаты мыши
  mouse.pos.x = xpos;
  mouse.pos.y = ypos;

  // Запишем градуcы поворота
  rotation.yaw += xoffset * speed.rotation;
  rotation.pitch += yoffset * speed.rotation;

  // Лимиты поворота по X
  if (rotation.pitch > 89.0f)
    rotation.pitch = 89.0f;
  if (rotation.pitch < -89.0f)
    rotation.pitch = -89.0f;

  // Обновим векторы направления (вектор вверх всегда константен)
  glm::float3 front;
  front.x = cos(glm::radians(rotation.yaw)) * cos(glm::radians(rotation.pitch));
  front.y = sin(glm::radians(rotation.pitch));
  front.z = sin(glm::radians(rotation.yaw)) * cos(glm::radians(rotation.pitch));
  direction.front = glm::normalize(front);
  direction.right = glm::normalize(glm::cross(direction.front, direction.upper));
}

void Camera::updatePosition(double deltaTime) {
  if (!(move.left || move.right || move.up || move.down || move.forward || move.back))
    return;

  float shift = speed.movement * deltaTime;

  if (move.up)
    position += shift * direction.upper;

  if (move.down)
    position -= shift * direction.upper;

  if (move.right)
    position += shift * direction.right;

  if (move.left)
    position -= shift * direction.right;

  if (move.forward)
    position += shift * direction.front;

  if (move.back)
    position -= shift * direction.front;
}
