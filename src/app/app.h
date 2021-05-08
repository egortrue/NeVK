#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "engine/engine.h"

class Application {
 private:
  GLFWwindow* window;
  Engine* engine;

 public:
  uint32_t WIDTH = 800;
  uint32_t HEIGHT = 600;

  Application() {
    initWindow();
    initEngine();
  }

  ~Application() {
    destroyEngine();
    destroyWindow();
  }

  void run() {
    while (!glfwWindowShouldClose(this->window)) {
      glfwPollEvents();  // Check keyboard and mouse events
    }
  }

 private:
  void initWindow() {
    // GLFW: Initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Monitor
    GLFWmonitor* monitor = nullptr;  // use glfwGetPrimaryMonitor() for fullscreen

    // Window
    this->window = glfwCreateWindow(WIDTH, HEIGHT, "NeVK Example", monitor, nullptr);
    if (this->window == nullptr) {
      glfwTerminate();
      throw std::runtime_error("ERROR: Failed to create GLFW window");
    }
  }

  void destroyWindow() {
    if (this->window == nullptr)
      glfwDestroyWindow(this->window);
    glfwTerminate();
  }

  void initEngine() {
    this->engine = new Engine(window);
  }

  void destroyEngine() {
    if (this->engine != nullptr)
      delete this->engine;
  }
};
