#include "object.h"

void Object::setPosition(glm::float3 position) {
  this->transform.position = position;
}

void Object::setRotation(glm::float3 rotation) {
  this->transform.rotation = rotation;
}

void Object::setScale(glm::float3 scale) {
  this->transform.scale = scale;
}

void Object::translate(glm::float3 position) {
  this->transform.position += position;
}

void Object::rotate(glm::float3 rotation) {
  this->transform.rotation += rotation;
}

void Object::scale(glm::float3 scale) {
  this->transform.scale += scale;
}

void Object::update() {
  modelMatrix = glm::translate(glm::mat4(1.0f), transform.position);
  modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  modelMatrix = glm::scale(modelMatrix, transform.scale);
}
