#include "engine.h"

Engine::Engine(Window::Manager window) : window(window) {
  initCore();
  initScene();
  initRender();
}

Engine::~Engine() {
  vkDeviceWaitIdle(core->device);
  destroyRender();
  destroyScene();
  destroyCore();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initCore() {
  core = new Core();
  {
    // Расширения экземпляра
    uint32_t extensionsCount = 0;
    std::vector<const char*> extensions;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    for (uint32_t i = 0; i < extensionsCount; i++)
      extensions.push_back(glfwExtensions[i]);
    core->setInstanceExtensions(extensions);
    core->init();

    // Поверхность вывода изображений
    if (glfwCreateWindowSurface(core->instance, window->instance, nullptr, &core->surface.handler) != VK_SUCCESS)
      throw std::runtime_error("ERROR: Failed to create window surface!");
    int width, height;
    glfwGetFramebufferSize(window->instance, &width, &height);
    window->width = width;
    window->height = height;
    core->surface.width = width;
    core->surface.height = height;
    core->configure();
  }

  // Ресурсы устройства и управление ими
  core->resources = new Resources(core);
  core->commands = new Commands(core);
}

void Engine::destroyCore() {
  if (core != nullptr) {
    delete core->commands;
    delete core->resources;
    core->destroy();
    delete core;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initScene() {
  scene = new Scene(core);
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
  render = new Render(window, core, scene);
}

void Engine::destroyRender() {
  if (render != nullptr)
    delete render;
}

Render::Manager Engine::getRender() {
  return render;
}
