#include "engine.h"

Engine::Engine(GLFWwindow* window) : window(window) {
  initCore();
}

Engine::~Engine() {
  destroyCore();
}

void Engine::initCore() {
  this->core = new Core();

  // GLFW расширения экземпляра
  std::vector<const char*> requiredExtensions;
  uint32_t extensionsCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
  for (uint32_t i = 0; i < extensionsCount; i++)
    requiredExtensions.push_back(glfwExtensions[i]);

  core->setExtensions(requiredExtensions);
  core->init();

  // GLFW поверхность вывода изображений
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(core->instance, window, nullptr, &surface) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create window surface!");
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  core->setSurface(surface, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
  core->configure();
}

void Engine::destroyCore() {
  if (this->core != nullptr) {
    this->core->destroy();
    delete this->core;
  }
}
