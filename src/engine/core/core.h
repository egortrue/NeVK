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
  VkInstance instance;
  VkDevice device;
  VkSurfaceKHR surface;

 private:
  std::vector<const char*> instanceExtensions;
  const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceProperties physicalDeviceProperties;
  VkPhysicalDeviceFeatures physicalDeviceFeatures;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkDebugUtilsMessengerEXT debugMessenger;

 public:
  void init();
  void configure();
  void destroy();
  void setExtensions(const std::vector<const char*>& requiredExtensions);

 private:
  void createInstance();
  void destroyInstance();

  void createDebugMessenger();
  void destroyDebugMessenger();

  //===============================
  // Device configuration:

  void choosePhysicalDevice();
  void createLogicalDevice();
  void destroyLogicalDevice();
  bool isDeviceSuitable(VkPhysicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice);
  struct QueueFamilyIndices {
    // Make variables without value at all using std::optional
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

  //===============================
  // SwapChain configuration:

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
};
