#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/core.h"

class Engine {
 private:
  GLFWwindow* window;
  Core* core;

 public:
  Engine(GLFWwindow* window);
  ~Engine();

 private:
  void initCore();
  void destroyCore();
};
