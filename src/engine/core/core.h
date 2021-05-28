#pragma once

// Сторонние библиотеки
#include <vulkan/vulkan.h>

// Внутренние библиотеки
#include "validation.h"

// Стандартные библиотеки
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>

class Core {
 public:
  typedef Core* Manager;

  void init();
  void configure();
  void destroy();

  //=========================================================================
  // Хранилище состояний Vulkan (Экземпляр)

 public:
  VkInstance instance;
  std::vector<const char*> instanceExtensions;
  void setInstanceExtensions(const std::vector<const char*>&);

 private:
  void createInstance();
  void destroyInstance();

  //=========================================================================
  // Устройства

 public:
  // Физическое устройство (GPU)
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceProperties physicalDeviceProperties;
  VkPhysicalDeviceFeatures physicalDeviceFeatures;

  // Логическое устройство (Интерфейс)
  VkDevice device;
  std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

 private:
  void createDevice();
  void destroyDevice();

  void choosePhysicalDevice();
  bool isDeviceSuitable(VkPhysicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice);

  //=========================================================================
  // Очереди задач

 public:
  VkQueue graphicsQueue;  // Очередь работы с графическими командами
  VkQueue presentQueue;   // Очередь работы с поверхностями вывода
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  } queueFamily;

 private:
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

  //=========================================================================
  // Устройство для вывода отладочной информации

 private:
  VkDebugUtilsMessengerEXT debugMessenger;
  void createDebugMessenger();
  void destroyDebugMessenger();

  //=========================================================================
  // Поверхность для отрисовки. Cоздаётся вне ядра (сторонним API)
  // Требует расширения VK_KHR_surface

 public:
  VkSurfaceKHR surface;
  uint32_t surfaceWidth, surfaceHeight;

 private:
  void destroySurface();

  //=========================================================================
  // Конечные изображения для показа (список показа)
  // Требует расширения VK_KHR_swapchain

 public:
  VkSwapchainKHR swapchain;
  VkExtent2D swapchainExtent;
  VkFormat swapchainFormat;

  // Буферы списка показа
  uint32_t swapchainImageCount;
  std::vector<VkImage> swapchainImages;

  void createSwapchain();
  void destroySwapchain();

 private:
  VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
  struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };
  SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice);
};
