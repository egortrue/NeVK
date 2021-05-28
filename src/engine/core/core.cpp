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

void Core::setInstanceExtensions(const std::vector<const char*>& requiredExtensions) {
  // Получим доступные расширения экземпляра
  uint32_t count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> instanceExtensionsAvailable(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count, instanceExtensionsAvailable.data());

  // Вывод доступных расширений экземпляра
  // std::cout << instanceExtensionsAvailable.size() << " available instance extensions:" << std::endl;
  // for (const auto& extension : instanceExtensionsAvailable)
  //   std::cout << '\t' << extension.extensionName << std::endl;

  // Инициализация расширений экземпляра
  this->instanceExtensions = requiredExtensions;
  this->instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  if (enableValidationLayers)
    this->instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  // Вывод подключаемых расширений экземпляра
  std::cout << instanceExtensions.size() << " enabled instance extensions:" << std::endl;
  for (const auto& extension : instanceExtensions)
    std::cout << '\t' << extension << std::endl;
}

//=============================================================================

void Core::createInstance() {
  if (enableValidationLayers && !checkValidationLayerSupport())
    throw std::runtime_error("ERROR: Validation layers requested, but not available!");

  // Описание приложения
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_2;

  appInfo.pEngineName = "NeVK";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  appInfo.pApplicationName = "NeVK Example";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

  // Описание экземпляра
  VkInstanceCreateInfo instanceInfo{};
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pApplicationInfo = &appInfo;

  // Подключение расширений экземпляра
  instanceInfo.enabledExtensionCount = instanceExtensions.size();
  instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

  // Режим отладки при работе с экземпляром
  if (enableValidationLayers) {
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();

    // Вывод дебаг-информации о создании экземпляра
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    populateDebugMessengerCreateInfo(debugCreateInfo);
    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  }

  // Создание экземпляра
  if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create instance!");
}

void Core::destroyInstance() {
  if (instance != VK_NULL_HANDLE)
    vkDestroyInstance(instance, nullptr);
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
  if (enableValidationLayers && debugMessenger != VK_NULL_HANDLE)
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
}

//=============================================================================

// Поверхность вывода создается сторонней оконной библиотекой
void Core::destroySurface() {
  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

//=============================================================================

void Core::choosePhysicalDevice() {
  // Поиск устройств с поддержкой Vulkan
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0)
    throw std::runtime_error("ERROR: Failed to find GPUs with Vulkan support!");
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  // Выбор подходящего устройства
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
  // Поддержка определенных очередей задач
  QueueFamilyIndices indices = findQueueFamilies(device);
  if (!indices.isComplete()) return false;

  // Поддержка расширений
  bool extensionsSupported = checkDeviceExtensionSupport(device);
  if (!extensionsSupported) return false;

  // Поддержка работы с поверхностями
  SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
  bool swapchainSupported = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
  if (!swapchainSupported) return false;

  // Поддержка анизатропной фильтрации
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
  if (!supportedFeatures.samplerAnisotropy) return false;

  return true;
}

bool Core::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  // Получим доступные расширения устройства
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> deviceExtensionsAvailable(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, deviceExtensionsAvailable.data());

  // Вывод доступных расширений устройства
  // std::cout << deviceExtensionsAvailable.size() << " available device extensions:" << std::endl;
  // for (const auto& extension : deviceExtensionsAvailable)
  //   std::cout << '\t' << extension.extensionName << std::endl;

  // Проверка поддержки требуемых расширений устройства
  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
  for (const auto& extension : deviceExtensionsAvailable) {
    requiredExtensions.erase(extension.extensionName);
  }

  if (requiredExtensions.empty()) {
    // Вывод подключаемых расширений устройства
    std::cout << deviceExtensions.size() << " enabled device extensions:" << std::endl;
    for (const auto& extension : deviceExtensions)
      std::cout << '\t' << extension << std::endl;
    return true;
  }

  return false;
}

Core::QueueFamilyIndices Core::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  // Получение семейств очередей
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // Поиск семейств очередей с поддержкой определенных функций
  uint32_t familyIndex = 0;
  for (const auto& queueFamily : queueFamilies) {
    // Поиск семейства с поддержкой графических команд
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphicsFamily = familyIndex;

    // Поиск семейства с поддержкой работы с поверхностями вывода
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, familyIndex, surface, &presentSupport);
    if (presentSupport)
      indices.presentFamily = familyIndex;

    if (indices.isComplete())
      break;

    familyIndex++;
  }

  return indices;
}

//=============================================================================

void Core::createDevice() {
  queueFamily = findQueueFamilies(physicalDevice);

  // Описание очередей задач
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {queueFamily.graphicsFamily.value(), queueFamily.presentFamily.value()};
  float queuePriority = 1.0f;
  for (uint32_t queueFamilyIndex : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
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
  }

  // Описание особенностей логического устройства
  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
  indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
  indexingFeatures.pNext = nullptr;
  indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
  indexingFeatures.runtimeDescriptorArray = VK_TRUE;

  // Описание логического устройства
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = &indexingFeatures;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;

  // Подключение расширений логического устройства
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // Режим отладки при работе с логическим устройством
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }

  // Создание логического устройства
  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create logical device!");

  // Получение очередей задач от логического устройства
  vkGetDeviceQueue(device, queueFamily.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, queueFamily.presentFamily.value(), 0, &presentQueue);
}

void Core::destroyDevice() {
  if (device != VK_NULL_HANDLE)
    vkDestroyDevice(device, nullptr);
}

//=============================================================================

void Core::createSwapchain() {
  SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapchainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapchainPresentMode(swapchainSupport.presentModes);
  VkExtent2D extent = chooseSwapchainExtent(swapchainSupport.capabilities);

  // Количество изображений в списке показа / буферизация
  uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
  if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  // Описание списка показа
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  // Описание изображений в списке показа
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // Настройка очередей
  uint32_t queueFamilyIndices[] = {queueFamily.graphicsFamily.value(), queueFamily.presentFamily.value()};
  if (queueFamily.graphicsFamily != queueFamily.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  // Создание списка показа
  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create swap chain!");

  // Получение списка изображений, через которые будет проводится показ
  vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
  swapchainImages.resize(swapchainImageCount);
  vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
  swapchainImages.shrink_to_fit();

  // Сохраним формат и размер изображений
  swapchainFormat = surfaceFormat.format;
  swapchainExtent = extent;
}

void Core::destroySwapchain() {
  if (swapchain != VK_NULL_HANDLE)
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
