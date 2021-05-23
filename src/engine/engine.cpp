#include "engine.h"

Engine::Engine(GLFWwindow* window) {
  initWindow(window);
  initCore();
  initResources();
  initFrames();
}

Engine::~Engine() {
  destroyFrames();
  destroyResources();
  destroyCore();
  destroyWindow();
}

void Engine::initWindow(GLFWwindow* window) {
  this->window = new Window;
  this->window->instance = window;
}

void Engine::destroyWindow() {
  if (window != nullptr)
    delete window;
}

void Engine::initCore() {
  core = new Core();

  // GLFW - Расширения экземпляра
  uint32_t extensionsCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
  for (uint32_t i = 0; i < extensionsCount; i++)
    window->extensions.push_back(glfwExtensions[i]);

  core->setInstanceExtensions(window->extensions);
  core->init();

  // GLFW - Поверхность вывода изображений
  if (glfwCreateWindowSurface(core->instance, window->instance, nullptr, &window->surface) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create window surface!");
  glfwGetFramebufferSize(window->instance, &window->width, &window->height);

  core->setSurface(window->surface, static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height));
  core->configure();
}

void Engine::destroyCore() {
  if (core != nullptr) {
    core->destroy();
    delete core;
  }
}

void Engine::initResources() {
  resources = new Resources(core);
}

void Engine::destroyResources() {
  if (resources != nullptr)
    delete resources;
}

void Engine::initFrames() {
  frames.resize(core->swapchainImages.size());
  frames.shrink_to_fit();
  for (auto& frame : frames) {
    frame.cmdPool = resources->createCommandBufferPool();
    frame.cmdBuffer = resources->createCommandBuffer(frame.cmdPool);
  }
}

void Engine::destroyFrames() {
  for (auto& frame : frames) {
    resources->destroyCommandBufferPool(frame.cmdPool);
  }
}
