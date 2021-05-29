#pragma once

// Сторонние библиотеки
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Стандартные библиотеки
#include <iostream>
#include <string>

class Window {
 public:
  typedef Window* Manager;

  GLFWwindow* instance;
  uint32_t width = 800;
  uint32_t height = 600;
  std::string title = "NeVK Example";
  bool isResized = false;

  struct Callbacks {
    GLFWkeyfun keyboard;
    GLFWmousebuttonfun mouseButtons;
    GLFWcursorposfun mousePos;
    GLFWframebuffersizefun fbResize;
  } callbacks;

  bool isClosed() {
    return glfwWindowShouldClose(instance);
  }

  void setActions() {
    glfwSetKeyCallback(instance, callbacks.keyboard);
    glfwSetMouseButtonCallback(instance, callbacks.mouseButtons);
    glfwSetCursorPosCallback(instance, callbacks.mousePos);
    glfwSetFramebufferSizeCallback(instance, callbacks.fbResize);
  }

  void checkActions() {
    glfwPollEvents();
  }

  Window() {
    // Инициализация GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Монитор
    // nullptr - оконный режим
    // glfwGetPrimaryMonitor() - полноэкранный режим
    GLFWmonitor* monitor = nullptr;

    // Создание окна
    this->instance = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
    if (this->instance == nullptr) {
      glfwTerminate();
      throw std::runtime_error("ERROR: Failed to create GLFW window");
    }
  }

  ~Window() {
    if (this->instance == nullptr)
      glfwDestroyWindow(this->instance);
    glfwTerminate();
  }
};
