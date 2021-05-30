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
      engine->getRender()->draw();
    }
  }

 private:
  void initWindow() {
    window = new Window();

    // Подключение обработчиков пользовательского ввода
    glfwSetWindowUserPointer(window->instance, this);
    window->callbacks.keyboard = keyCallback;
    window->callbacks.mousePos = mouseMoveCallback;
    window->callbacks.mouseButtons = mouseButtonCallback;
    window->callbacks.fbResize = framebufferResizeCallback;
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
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if (action != GLFW_RELEASE) {
      // При использованиии меню срабатывает другой callback
      auto interface = app->engine->getRender()->getInterface();
      if (interface->imgui.menuHovered)
        return;
    }

    auto camera = app->engine->getScene()->getCamera();
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
    auto camera = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))->engine->getScene()->getCamera();
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        glfwGetCursorPos(window, &camera->mouse.pos.x, &camera->mouse.pos.y);
        camera->mouse.right = true;
      } else if (action == GLFW_RELEASE) {
        camera->mouse.right = false;
      }
    }
  }

  static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    auto camera = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))->engine->getScene()->getCamera();
    if (camera->mouse.right)
      camera->updateRotation(xpos, ypos);
  }

  static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->window->isResized = true;

    int width_ = 0, height_ = 0;
    do {
      glfwGetFramebufferSize(window, &width_, &height_);
      glfwWaitEvents();
    } while (width_ == 0 || height_ == 0);

    // Обновим разрешение у главного обработчика окна
    app->window->width = static_cast<uint32_t>(width_);
    app->window->height = static_cast<uint32_t>(height_);
  }
};
