#include "scene.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Scene::Scene(Core::Manager core, Commands::Manager commands, Resources::Manager resources) {
  this->core = core;
  this->commands = commands;
  this->resources = resources;
  initTextures();
  initModels();
  initCamera();
}

Scene::~Scene() {
  for (auto obj : objects)
    delete obj;
  destroyCamera();
  destroyModels();
  destroyTextures();
}

Textures::Manager Scene::getTextures() {
  return textures;
}

void Scene::loadObject(const char* model) {
  loadObject(std::string(model));
}

void Scene::loadObject(const std::string& model) {
  PhysicalObject::Instance object = new PhysicalObject();
  object->model = models->load(model);
  objects.push_back(object);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::initTextures() {
  textures = new Textures(core, commands, resources);
}

void Scene::destroyTextures() {
  if (textures != nullptr)
    delete textures;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::initModels() {
  models = new Models(commands, resources, textures);
  PhysicalObject::Instance object;
  loadObject("test");
  loadObject("cube");
  object = objects.back();
  object->setPosition({5.0f, 0.0f, 0.0f});
  object->update();
  loadObject("teapot");
  object = objects.back();
  object->setPosition({-5.0f, 0.0f, 0.0f});
  object->update();
  loadObject("tree");
  object = objects.back();
  object->setPosition({0.0f, 0.0f, -5.0f});
  object->update();
}

void Scene::destroyModels() {
  if (models != nullptr)
    delete models;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::initCamera() {
  camera = new Camera();
  camera->projection.fov = 45.0f;
  camera->projection.aspect = 4.0f / 3.0f;
  camera->projection.near = 0.1f;
  camera->projection.far = 256.0f;
  camera->updateProjection();
  camera->setPosition({0, 0, 8});
  camera->updateView();
}

void Scene::destroyCamera() {
  if (camera != nullptr)
    delete camera;
}

Camera::Manager Scene::getCamera() {
  return this->camera;
}
