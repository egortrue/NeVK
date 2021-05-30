#include "engine.h"

Engine::Engine(Window::Manager window) : window(window) {
  initCore();
  initResources();
  initCommands();
  initScene();
  initRender();
}

Engine::~Engine() {
  vkDeviceWaitIdle(core->device);
  destroyRender();
  destroyScene();
  destroyCommands();
  destroyResources();
  destroyCore();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initCore() {
  core = new Core();

  // Расширения экземпляра
  uint32_t extensionsCount = 0;
  std::vector<const char*> extensions;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
  for (uint32_t i = 0; i < extensionsCount; i++)
    extensions.push_back(glfwExtensions[i]);

  core->setInstanceExtensions(extensions);
  core->init();

  // Поверхность вывода изображений
  if (glfwCreateWindowSurface(core->instance, window->instance, nullptr, &core->surface) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create window surface!");
  int width, height;
  glfwGetFramebufferSize(window->instance, &width, &height);
  window->width = width;
  window->height = height;
  core->surfaceWidth = width;
  core->surfaceHeight = height;

  core->configure();
}

void Engine::destroyCore() {
  if (core != nullptr) {
    core->destroy();
    delete core;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initResources() {
  resources = new Resources(core);
}

void Engine::destroyResources() {
  if (resources != nullptr)
    delete resources;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initCommands() {
  commands = new Commands(core, resources);
}

void Engine::destroyCommands() {
  if (commands != nullptr)
    delete commands;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initScene() {
  scene = new Scene(core, commands, resources);
}

void Engine::destroyScene() {
  if (scene != nullptr)
    delete scene;
}

Scene::Manager Engine::getScene() {
  return scene;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initRender() {
  render = new Render(window, core, resources, commands, scene);
}

void Engine::destroyRender() {
  if (render != nullptr)
    delete render;
}

Render::Manager Engine::getRender() {
  return render;
}
