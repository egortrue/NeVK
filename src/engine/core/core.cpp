#include "core.h"

void Core::init() {
  createInstance();
  createDebugMessenger();
}

void Core::configure() {
  choosePhysicalDevice();
  createLogicalDevice();
}

void Core::destroy() {
  destroyInstance();
  destroyDebugMessenger();
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

//=============================================================================

void Core::createInstance() {
  // Validation request
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("ERROR: Validation layers requested, but not available!");
  }

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

  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("ERROR: Failed to set up debug messenger!");
  }
}

void Core::destroyDebugMessenger() {
  if (!enableValidationLayers) return;
  DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
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

  if (this->physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("ERROR: Failed to find a suitable GPU!");
  }
}

bool Core::isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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

void Core::createLogicalDevice() {
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

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("ERROR: Failed to create logical device!");
  }

  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

//=============================================================================

Core::SwapChainSupportDetails Core::querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;

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