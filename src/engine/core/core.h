#pragma once

// Сторонние библиотеки
#include <vulkan/vulkan.h>

// Внутренние библиотеки
#include "validation.h"

// Стандартные библиотеки
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>

class Resources;
class Commands;

class Core {
 public:
  typedef Core* Manager;
  Resources* resources;  // Менеджер ресурсов - выделение и уничтожение буферов, изображений
  Commands* commands;    // Менеджер команд - генерация и хранение базовых команд устройства

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
  struct {
    VkPhysicalDevice handler;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
  } physicalDevice;

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
  struct {
    VkSurfaceKHR handler;
    uint32_t width;
    uint32_t height;
  } surface;

 private:
  void destroySurface();

  //=========================================================================
  // Конечные изображения для показа (список показа)
  // Требует расширения VK_KHR_swapchain

 public:
  struct {
    VkSwapchainKHR handler;
    VkExtent2D extent;
    VkFormat format;
    uint32_t count;
    std::vector<VkImage> images;
  } swapchain;

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

// Внутренние библиотеки
#include "resources/resources.h"
#include "commands/commands.h"
