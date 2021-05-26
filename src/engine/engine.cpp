#include "engine.h"

Engine::Engine(GLFWwindow* window) {
  initWindow(window);
  initCore();
  initResources();
  initCommands();
  initShaders();
  initTextures();
  initModels();
  initFrames();
  initGeometryPass();
}

Engine::~Engine() {
  vkDeviceWaitIdle(core->device);
  destroyGeometryPass();
  destroyFrames();
  destroyModels();
  destroyTextures();
  destroyShaders();
  destroyCommands();
  destroyResources();
  destroyCore();
  destroyWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initWindow(GLFWwindow* window) {
  this->window = new Window;
  this->window->instance = window;
}

void Engine::destroyWindow() {
  if (window != nullptr)
    delete window;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initResources() {
  resources = new Resources(core);
}

void Engine::destroyResources() {
  if (resources != nullptr)
    delete resources;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initCommands() {
  commands = new Commands(core, resources);
}

void Engine::destroyCommands() {
  if (commands != nullptr)
    delete commands;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initShaders() {
  shaders = new Shaders();
}

void Engine::destroyShaders() {
  if (shaders != nullptr)
    delete shaders;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initTextures() {
  textures = new Textures(core, commands, resources);
}

void Engine::destroyTextures() {
  if (textures != nullptr)
    delete textures;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initModels() {
  models = new Models();
  createIndexBuffer();
  createVertexBuffer();
}

void Engine::destroyModels() {
  if (models != nullptr)
    delete models;
}

void Engine::createVertexBuffer() {
  std::vector<float>& vertices = models->cube.vertices;
  resources->createBuffer(
      vertices.size() * sizeof(float),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      vertexBuffer, vertexBufferMemory);

  commands->copyDataToBuffer(vertices.data(), vertexBuffer, vertices.size() * sizeof(float));
}

void Engine::createIndexBuffer() {
  std::vector<uint32_t>& indices = models->cube.indices;
  resources->createBuffer(
      indices.size() * sizeof(uint32_t),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      indexBuffer, indexBufferMemory);

  commands->copyDataToBuffer(indices.data(), indexBuffer, indices.size() * sizeof(uint32_t));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    vkDestroySemaphore(core->device, frame.available, nullptr);
    vkDestroyFence(core->device, frame.drawing, nullptr);
    commands->destroyCommandBufferPool(frame.cmdPool);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initGeometryPass() {
  geometryPass.core = core;
  geometryPass.resources = resources;
  geometryPass.commands = commands;
  geometryPass.shaders = shaders;
  geometryPass.shaderName = std::string("shaders/geometry.hlsl");
  geometryPass.textures = textures;
  geometryPass.textureName = std::string("misc/textures/brickwall.png");

  // Цель вывода прохода рендера
  geometryPass.targetImageCount = core->swapchainImageCount;
  geometryPass.targetImageWidth = core->swapchainExtent.width;
  geometryPass.targetImageHeight = core->swapchainExtent.height;
  geometryPass.targetImageFormat = core->swapchainFormat;

  // TODO: тройное копирование векторов
  geometryPass.targetImageViews = resources->createImageViews(
      core->swapchainImages,
      core->swapchainFormat,
      VK_IMAGE_ASPECT_COLOR_BIT);

  geometryPass.init();
}

void Engine::destroyGeometryPass() {
  resources->destroyImageViews(geometryPass.targetImageViews);
  geometryPass.destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  data.indicesCount = static_cast<uint32_t>(models->cube.indices.size());
  data.indices = indexBuffer;
  data.vertices = vertexBuffer;

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

  currentFrameIndex = (swapchainImageIndex + 1) % core->swapchainImageCount;
}
