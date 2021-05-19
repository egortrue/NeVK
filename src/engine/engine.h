#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/core.h"

class Engine {
 private:
  Core* core;

  GLFWwindow* window;

 public:
  Engine(GLFWwindow* window);
  ~Engine();

 private:
  void initCore();
  void destroyCore();
};
