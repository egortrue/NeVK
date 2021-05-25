#include "engine.h"

Engine::Engine(GLFWwindow* window) {
  initWindow(window);
  initCore();
  initResources();
  initCommands();
  initFrames();
  initGeometryPass();
}

Engine::~Engine() {
  destroyFrames();
  destroyCommands();
  destroyResources();
  destroyCore();
  destroyWindow();
}

void Engine::initWindow(GLFWwindow* window) {
  this->window = new Window;
  this->window->instance = window;
}

void Engine::destroyWindow() {
  if (window != nullptr)
    delete window;
}

void Engine::initCore() {
  core = new Core();

  // GLFW - Расширения экземпляра
  uint32_t extensionsCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
  for (uint32_t i = 0; i < extensionsCount; i++)
    window->extensions.push_back(glfwExtensions[i]);

  core->setInstanceExtensions(window->extensions);
  core->init();

  // GLFW - Поверхность вывода изображений
  if (glfwCreateWindowSurface(core->instance, window->instance, nullptr, &window->surface) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create window surface!");
  glfwGetFramebufferSize(window->instance, &window->width, &window->height);

  core->setSurface(window->surface, static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height));
  core->configure();
}

void Engine::destroyCore() {
  if (core != nullptr) {
    core->destroy();
    delete core;
  }
}

void Engine::initResources() {
  resources = new Resources(core);
}

void Engine::destroyResources() {
  if (resources != nullptr)
    delete resources;
}

void Engine::initCommands() {
  commands = new Commands(core);
}

void Engine::destroyCommands() {
  if (commands != nullptr)
    delete commands;
}

void Engine::initFrames() {
  currentFrameIndex = 0;
  frames.resize(core->swapchainImages.size());
  frames.shrink_to_fit();

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (auto& frame : frames) {
    frame.cmdPool = commands->createCommandBufferPool();
    frame.cmdBuffer = commands->createCommandBuffer(frame.cmdPool);

    vkCreateSemaphore(core->device, &semaphoreInfo, nullptr, &frame.available);
    vkCreateFence(core->device, &fenceInfo, nullptr, &frame.drawing);
  }
}

void Engine::destroyFrames() {
  for (auto& frame : frames) {
    commands->destroyCommandBufferPool(frame.cmdPool);
  }
}

VkFormat findSupportedFormat(Core* core, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(core->physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

VkFormat findDepthFormat(Core* core) {
  return findSupportedFormat(
      core,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Engine::initGeometryPass() {
  geometryPass.core = core;
  geometryPass.resources = resources;
  geometryPass.shaderManager = new ShaderManager();
  geometryPass.shaderName = std::string("shaders/geometry.hlsl");
  geometryPass.imageCount = core->swapchainImagesCount;
  geometryPass.imageWidth = core->swapchainExtent.width;
  geometryPass.imageHeight = core->swapchainExtent.height;
  geometryPass.imageFormat = core->swapchainFormat;

  // TODO: тройное копирование векторов
  geometryPass.imageViews = resources->createImageViews(core->swapchainImages,
                                                        core->swapchainFormat,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);

  geometryPass.depthImageFormat = findDepthFormat(core);
  resources->createImage(core->swapchainExtent.width,
                         core->swapchainExtent.height,
                         geometryPass.depthImageFormat,
                         VK_IMAGE_TILING_OPTIMAL,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         geometryPass.depthImage, geometryPass.depthImageMemory);

  geometryPass.depthImageView = resources->createImageView(
      geometryPass.depthImage,
      geometryPass.depthImageFormat,
      VK_IMAGE_ASPECT_DEPTH_BIT);

  resources->createImage(
      core->swapchainExtent.width, core->swapchainExtent.height,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      geometryPass.textureImage, geometryPass.textureImageMemory);

  geometryPass.textureImageView = resources->createImageView(
      geometryPass.textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_ASPECT_COLOR_BIT);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = core->physicalDeviceProperties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  if (vkCreateSampler(core->device, &samplerInfo, nullptr, &geometryPass.textureSampler) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create texture sampler!");

  geometryPass.init();
}

void Engine::drawFrame() {
  Frame& frame = frames[currentFrameIndex];
  vkWaitForFences(core->device, 1, &frame.drawing, VK_TRUE, UINT64_MAX);

  uint32_t swapchainImageIndex;
  VkResult result = vkAcquireNextImageKHR(core->device, core->swapchain, UINT64_MAX, frame.available, VK_NULL_HANDLE, &swapchainImageIndex);

  commands->resetCommandBuffer(frame.cmdBuffer);

  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = nullptr;
  cmdBeginInfo.pInheritanceInfo = nullptr;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(frame.cmdBuffer, &cmdBeginInfo);

  GeometryPass::RecordData data;
  data.cmd = frame.cmdBuffer;
  data.imageIndex = swapchainImageIndex;
  data.indicesCount = 0;
  data.indices = nullptr;
  data.vertices = nullptr;

  geometryPass.record(data);

  vkEndCommandBuffer(frame.cmdBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {frame.available};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &frame.cmdBuffer;

  vkResetFences(core->device, 1, &frame.drawing);
  if (vkQueueSubmit(core->graphicsQueue, 1, &submitInfo, frame.drawing) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to submit draw command buffer!");

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  VkSwapchainKHR swapChains[] = {core->swapchain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &swapchainImageIndex;

  vkQueuePresentKHR(core->presentQueue, &presentInfo);

  currentFrameIndex = (swapchainImageIndex + 1) % core->swapchainImagesCount;
}
