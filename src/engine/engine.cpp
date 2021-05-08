#include "engine.h"

Engine::Engine(GLFWwindow* window) : window(window) {
  initCore();
}

Engine::~Engine() {
  destroyCore();
}

void Engine::initCore() {
  this->core = new Core();

  // GLFW Extensions
  std::vector<const char*> requiredExtensions;
  uint32_t extensionsCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
  for (uint32_t i = 0; i < extensionsCount; i++)
    requiredExtensions.push_back(glfwExtensions[i]);

  core->setExtensions(requiredExtensions);
  core->init();

  // GLFW Rendering Surface
  if (glfwCreateWindowSurface(core->instance, window, nullptr, &core->surface) != VK_SUCCESS) {
    throw std::runtime_error("ERROR: Failed to create window surface!");
  }

  core->configure();
}

void Engine::destroyCore() {
  if (this->core != nullptr) {
    this->core->destroy();
    delete this->core;
  }
}