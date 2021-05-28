#include "engine.h"

Engine::Engine(Window::Manager window) : window(window) {
  initCore();
  initResources();
  initCommands();
  initShaders();
  initScene();
  initFrames();
  initGeometryPass();
}

Engine::~Engine() {
  vkDeviceWaitIdle(core->device);
  destroyGeometryPass();
  destroyFrames();
  destroyScene();
  destroyShaders();
  destroyCommands();
  destroyResources();
  destroyCore();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initCore() {
  core = new Core();

  // Расширения экземпляра
  uint32_t extensionsCount = 0;
  std::vector<const char*> extensions;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
  for (uint32_t i = 0; i < extensionsCount; i++)
    extensions.push_back(glfwExtensions[i]);

  core->setInstanceExtensions(extensions);
  core->init();

  // Поверхность вывода изображений
  if (glfwCreateWindowSurface(core->instance, window->instance, nullptr, &core->surface) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create window surface!");
  int width, height;
  glfwGetFramebufferSize(window->instance, &width, &height);
  core->surfaceWidth = static_cast<uint32_t>(width);
  core->surfaceHeight = static_cast<uint32_t>(height);

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

void Engine::initScene() {
  scene = new Scene(core, commands, resources);
  scene->loadObject("misc/models/teapot.obj", "misc/textures/default.png");
}

void Engine::destroyScene() {
  if (scene != nullptr)
    delete scene;
}

Scene::Manager Engine::getScene() {
  return scene;
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
  geometryPass.updateTextureDescriptors(scene->objects.front()->texture->view,
                                        scene->objects.front()->texture->sampler);

  // Цель вывода прохода рендера
  geometryPass.targetImageCount = core->swapchainImageCount;
  geometryPass.targetImageWidth = core->swapchainExtent.width;
  geometryPass.targetImageHeight = core->swapchainExtent.height;
  geometryPass.targetImageFormat = core->swapchainFormat;
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

  // Получим время
  static auto prevTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  float deltaTime = std::chrono::duration<double, std::milli>(currentTime - prevTime).count() / 1000.0;
  prevTime = currentTime;

  auto object = scene->objects.front();
  auto camera = scene->getCamera();
  camera->update(deltaTime);
  geometryPass.updateUniformDescriptors(
      swapchainImageIndex,
      camera->transform.view,
      camera->transform.projection);

  //=========================================================================
  // Начало рендера

  VkCommandBuffer cmdBuffer = frames[swapchainImageIndex].cmdBuffer;
  commands->resetCommandBuffer(cmdBuffer);

  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = nullptr;
  cmdBeginInfo.pInheritanceInfo = nullptr;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);

  GeometryPass::record_t data;
  data.cmd = cmdBuffer;
  data.imageIndex = swapchainImageIndex;
  data.indicesCount = object->model->verticesCount;  // TODO: Сделать индексацию
  data.indices = object->model->indexBuffer;
  data.vertices = object->model->vertexBuffer;

  geometryPass.record(data);

  vkEndCommandBuffer(cmdBuffer);

  //=========================================================================

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
