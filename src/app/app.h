#pragma once

// Внутренние библиотеки
#include "window.h"
#include "engine.h"

class Application {
 private:
  Window::Manager window;
  Engine::Manager engine;

 public:
  Application() {
    initWindow();
    initEngine();
  }

  ~Application() {
    destroyEngine();
    destroyWindow();
  }

  void run() {
    while (!window->isClosed()) {
      window->checkActions();
      engine->drawFrame();
    }
  }

 private:
  void initWindow() {
    window = new Window();
    glfwSetWindowUserPointer(window->instance, this);
    window->callbacks.keyboard = keyCallback;
    window->callbacks.mousePos = mouseMoveCallback;
    window->callbacks.mouseButtons = mouseButtonCallback;

    window->setActions();
  }

  void destroyWindow() {
    if (window != nullptr)
      delete window;
  }

  void initEngine() {
    engine = new Engine(window);
  }

  void destroyEngine() {
    if (engine != nullptr)
      delete engine;
  }

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto camera = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))->engine->getCamera();
    const bool keyState = ((GLFW_REPEAT == action) || (GLFW_PRESS == action)) ? true : false;
    switch (key) {
      case GLFW_KEY_W:
        camera->move.forward = keyState;
        break;
      case GLFW_KEY_S:
        camera->move.back = keyState;
        break;
      case GLFW_KEY_A:
        camera->move.left = keyState;
        break;
      case GLFW_KEY_D:
        camera->move.right = keyState;
        break;
      case GLFW_KEY_Q:
        camera->move.up = keyState;
        break;
      case GLFW_KEY_E:
        camera->move.down = keyState;
        break;
      default:
        break;
    }
  }

  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto camera = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))->engine->getCamera();

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        double xpos = 0, ypos = 0;
        glfwGetCursorPos(window, &camera->mouse.pos.x, &camera->mouse.pos.y);
        camera->mouse.right = true;
      } else if (action == GLFW_RELEASE) {
        camera->mouse.right = false;
      }
    }
  }

  static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    auto camera = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))->engine->getCamera();

    if (camera->mouse.right)
      camera->rotate(xpos, ypos);
  }
};
