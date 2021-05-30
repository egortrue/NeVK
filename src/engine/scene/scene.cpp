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

void Scene::loadObject(const char* model, const char* texture) {
  loadObject(std::string(model), std::string(texture));
}

void Scene::loadObject(const std::string& model, const std::string& texture) {
  PhysicalObject::Instance object = new PhysicalObject();
  object->model = models->loadModel(model);
  object->texture = textures->loadTexture(texture);
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
  models = new Models(commands, resources);
  loadObject("misc/models/teapot.obj", "misc/textures/default.png");
  loadObject("misc/models/cube.obj", "misc/textures/default.png");
  loadObject("misc/models/pine.obj", "misc/textures/default.png");
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
  camera->setPosition({0, 0.5f, 3});
  camera->updateView();
}

void Scene::destroyCamera() {
  if (camera != nullptr)
    delete camera;
}

Camera::Manager Scene::getCamera() {
  return this->camera;
}
