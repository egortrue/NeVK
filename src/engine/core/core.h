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
  VkPhysicalDevice physicalDevice;

  std::vector<VkImage> swapchainImages;
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;

 private:
  VkSurfaceKHR surface;
  uint32_t surfaceWidth;
  uint32_t surfaceHeight;

  VkSwapchainKHR swapchain;

  // Extensions
  std::vector<const char*> instanceExtensions;
  const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  // Device options
  VkPhysicalDeviceProperties physicalDeviceProperties;
  VkPhysicalDeviceFeatures physicalDeviceFeatures;
  VkQueue graphicsQueue;
  VkQueue presentQueue;

  // Debug
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
  // Device configuration:

  void createDevice();
  void destroyDevice();

  void choosePhysicalDevice();
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
