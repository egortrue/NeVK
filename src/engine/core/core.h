#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>

#include <vulkan/vulkan.h>
#include "validation.h"

class Core {
 public:
  // Хранилище состояний Vulkan (Экземпляр)
  VkInstance instance;

  // Обработчики устройства
  VkDevice device;                  // Логическое утсройство (Интерфейс)
  VkPhysicalDevice physicalDevice;  // Физическое устройство (GPU)
  VkPhysicalDeviceProperties physicalDeviceProperties;
  VkPhysicalDeviceFeatures physicalDeviceFeatures;

  // Очереди задач
  VkQueue graphicsQueue;  // Очередь работы с графическими командами
  VkQueue presentQueue;   // Очередь работы с поверхностями вывода

  // Конечные изображения для показа (список показа)
  // Требует расширения VK_KHR_swapchain
  std::vector<VkImage> swapchainImages;
  VkSwapchainKHR swapchain;
  VkExtent2D swapchainExtent;
  VkFormat swapchainFormat;

  // Поверхность для отрисовки
  // Требует расширения VK_KHR_surface
  VkSurfaceKHR surface;
  uint32_t surfaceWidth;
  uint32_t surfaceHeight;

 private:
  // Расширения
  std::vector<const char*> instanceExtensions;
  const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  // Устройство для вывода дебаг-информации
  VkDebugUtilsMessengerEXT debugMessenger;

 public:
  void init();
  void configure();
  void destroy();
  void setExtensions(const std::vector<const char*>&);
  void setSurface(VkSurfaceKHR, uint32_t width, uint32_t height);

 private:
  void createInstance();
  void destroyInstance();

  void createDebugMessenger();
  void destroyDebugMessenger();

  void destroySurface();

  //===============================
  // Конфигурация устройства

  void createDevice();
  void destroyDevice();

  void choosePhysicalDevice();
  bool isDeviceSuitable(VkPhysicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice);
  struct QueueFamilyIndices {
    // Объявление переманных без значения, используя std::optional
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

  //===============================
  // Конфигурация списка показа

  void createSwapchain();
  void destroySwapchain();

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
