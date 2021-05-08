#include "core.h"

void Core::init() {
  createInstance();
  createDebugMessenger();
}

void Core::configure() {
  choosePhysicalDevice();
  createDevice();
  createSwapchain();
}

void Core::destroy() {
  destroySwapchain();
  destroySurface();
  destroyDevice();
  destroyDebugMessenger();
  destroyInstance();
}

//=============================================================================

void Core::setExtensions(const std::vector<const char*>& requiredExtensions) {
  // Get available extensions
  uint32_t count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> instanceExtensionsAvailable(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count, instanceExtensionsAvailable.data());

  // Output available extensions
  std::cout << instanceExtensionsAvailable.size() << " available extensions:" << std::endl;
  for (const auto& extension : instanceExtensionsAvailable)
    std::cout << '\t' << extension.extensionName << std::endl;

  // Output required extensions
  std::cout << requiredExtensions.size() << " required extnesions:" << std::endl;
  for (const auto& extension : requiredExtensions)
    std::cout << '\t' << extension << std::endl;

  // TODO: Here should be the intersection of sets "availableExtensions" and "requiredExtensions"
  this->instanceExtensions = requiredExtensions;

  if (enableValidationLayers)
    this->instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  this->instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
}

void Core::setSurface(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
  this->surface = surface;
  this->surfaceWidth = width;
  this->surfaceHeight = height;
}

//=============================================================================

void Core::createInstance() {
  // Validation request
  if (enableValidationLayers && !checkValidationLayerSupport())
    throw std::runtime_error("ERROR: Validation layers requested, but not available!");

  // Application description
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_2;

  appInfo.pEngineName = "NeVK";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  appInfo.pApplicationName = "NeVK Exanmple";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

  // Set instance info
  VkInstanceCreateInfo instanceInfo{};
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pApplicationInfo = &appInfo;
  instanceInfo.enabledExtensionCount = instanceExtensions.size();
  instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

  // Debug messages of instance creation
  if (enableValidationLayers) {
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    populateDebugMessengerCreateInfo(debugCreateInfo);
    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  }

  // Create instance
  if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create instance!");
}

void Core::destroyInstance() {
  if (this->instance != VK_NULL_HANDLE)
    vkDestroyInstance(this->instance, nullptr);
}

//=============================================================================

void Core::createDebugMessenger() {
  if (!enableValidationLayers) return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to set up debug messenger!");
}

void Core::destroyDebugMessenger() {
  if (!enableValidationLayers) return;
  DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
}

//=============================================================================

void Core::destroySurface() {
  vkDestroySurfaceKHR(instance, surface, nullptr);
}

//=============================================================================

void Core::choosePhysicalDevice() {
  // Find the count of GPUs
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0)
    throw std::runtime_error("ERROR: Failed to find GPUs with Vulkan support!");

  // Find all the GPUs
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  // Choose the GPU
  for (const auto& currentDevice : devices) {
    if (isDeviceSuitable(currentDevice)) {
      this->physicalDevice = currentDevice;
      vkGetPhysicalDeviceProperties(this->physicalDevice, &(this->physicalDeviceProperties));
      vkGetPhysicalDeviceFeatures(this->physicalDevice, &(this->physicalDeviceFeatures));
      std::cout << "GPU: " << this->physicalDeviceProperties.deviceName << std::endl;
      break;
    }
  }

  if (this->physicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("ERROR: Failed to find a suitable GPU!");
}

bool Core::isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapchainAdequate = false;
  if (extensionsSupported) {
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
    swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapchainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Core::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> deviceExtensionsAvailable(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, deviceExtensionsAvailable.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : deviceExtensionsAvailable) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

Core::QueueFamilyIndices Core::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // Find queue families index with certain capability
  uint32_t graphicsFamilyIndex = 0;
  for (const auto& queueFamily : queueFamilies) {
    // Check for VK_QUEUE_GRAPHICS_BIT support
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = graphicsFamilyIndex;
    }

    // Check for SURFACE_PRESENT_BIT support
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, graphicsFamilyIndex, surface, &presentSupport);
    if (presentSupport) {
      indices.presentFamily = graphicsFamilyIndex;
    }

    if (indices.isComplete()) break;
    graphicsFamilyIndex++;
  }

  return indices;
}

//=============================================================================

void Core::createDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  {
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    indexingFeatures.pNext = nullptr;

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.pNext = &indexingFeatures;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

    if (indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray) {
      // all set to use unbound arrays of textures
    }
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
  indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
  indexingFeatures.pNext = nullptr;
  indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
  indexingFeatures.runtimeDescriptorArray = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = &indexingFeatures;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create logical device!");

  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void Core::destroyDevice() {
  vkDestroyDevice(device, nullptr);
}

//=============================================================================

void Core::createSwapchain() {
  SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapchainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapchainPresentMode(swapchainSupport.presentModes);
  VkExtent2D extent = chooseSwapchainExtent(swapchainSupport.capabilities);

  uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
  if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create swap chain!");

  vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
  swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

  swapchainImageFormat = surfaceFormat.format;
  swapchainExtent = extent;
}

void Core::destroySwapchain() {
  vkDestroySwapchainKHR(device, swapchain, nullptr);
}

Core::SwapchainSupportDetails Core::querySwapchainSupport(VkPhysicalDevice device) {
  SwapchainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  details.formats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  details.presentModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());

  return details;
}

VkSurfaceFormatKHR Core::chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  for (const auto& availableFormat : availableFormats)
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return availableFormat;

  return availableFormats[0];
}

VkPresentModeKHR Core::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  for (const auto& availablePresentMode : availablePresentModes)
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return availablePresentMode;

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Core::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent;
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, surfaceWidth));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, surfaceHeight));
    return actualExtent;
  }
}