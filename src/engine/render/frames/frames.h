#pragma once

// Внутренние библиотеки
#include "core.h"
#include "commands.h"

// Стандартные библиотеки
#include <vector>

class Frames {
 public:
  typedef Frames* Manager;

  typedef struct frame_t {
    // Задания кадра
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;

    // Синхронизация кадров
    VkFence drawing;
    VkFence showing;

    // Синхронизация внутри кадра
    VkSemaphore imageAvailable;
    VkSemaphore imageRendered;
  } * Instance;

 private:
  Core::Manager core;
  Commands::Manager commands;

  std::vector<Instance> handlers;
  std::vector<VkFence> fences;
  std::vector<VkSemaphore> semaphores;

 public:
  Frames(Core::Manager, Commands::Manager);
  ~Frames();

  uint32_t currentFrameIndex;
  Instance getFrame(uint32_t id);
  Instance getCurrentFrame();
};
