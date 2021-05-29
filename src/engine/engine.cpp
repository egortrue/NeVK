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
  window->width = width;
  window->height = height;
  core->surfaceWidth = width;
  core->surfaceHeight = height;

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
  frames = new Frames(core, commands);
}

void Engine::destroyFrames() {
  if (frames != nullptr)
    delete frames;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::initGeometryPass() {
  geometryPass.core = core;
  geometryPass.resources = resources;
  geometryPass.commands = commands;
  geometryPass.shaders = shaders;
  geometryPass.shaderName = std::string("shaders/geometry.hlsl");
  geometryPass.textureImageView = scene->objects.front()->texture->view;
  geometryPass.textureSampler = scene->objects.front()->texture->sampler;

  // Цель вывода прохода рендера
  geometryPass.colorImageCount = core->swapchainImageCount;
  geometryPass.colorImageWidth = core->swapchainExtent.width;
  geometryPass.colorImageHeight = core->swapchainExtent.height;
  geometryPass.colorImageFormat = core->swapchainFormat;
  geometryPass.colorImageViews = resources->createImageViews(
      core->swapchainImages,
      core->swapchainFormat,
      VK_IMAGE_ASPECT_COLOR_BIT);

  geometryPass.init();
}

void Engine::destroyGeometryPass() {
  resources->destroyImageViews(geometryPass.colorImageViews);
  geometryPass.destroy();
}

void Engine::resizeSwapchain() {
  vkDeviceWaitIdle(core->device);
  window->isResized = false;

  // Пересоздадим список показа
  core->destroySwapchain();
  core->createSwapchain();

  // Обновим все проходы рендера
  resources->destroyImageViews(geometryPass.colorImageViews);
  geometryPass.colorImageWidth = core->swapchainExtent.width;
  geometryPass.colorImageHeight = core->swapchainExtent.height;
  geometryPass.colorImageViews = resources->createImageViews(
      core->swapchainImages,
      core->swapchainFormat,
      VK_IMAGE_ASPECT_COLOR_BIT);
  geometryPass.resize();

  // Обновим соотношение сторон для камеры
  auto camera = scene->getCamera();
  camera->projection.aspect = static_cast<float>(window->width) / static_cast<float>(window->height);
  camera->updateProjection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::drawFrame() {
  auto currentFrame = frames->getCurrentFrame();
  vkWaitForFences(core->device, 1, &currentFrame->drawing, VK_TRUE, UINT64_MAX);

  //=========================================================================
  // Получение изображения из списка показа

  uint32_t swapchainImageIndex;
  VkResult result = vkAcquireNextImageKHR(core->device, core->swapchain, UINT64_MAX, currentFrame->imageAvailable, VK_NULL_HANDLE, &swapchainImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    resizeSwapchain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("ERROR: Failed to acquire swapchain image!");
  }

  auto targetFrame = frames->getFrame(swapchainImageIndex);

  //=========================================================================
  // Генерация команд рендера

  VkCommandBuffer cmdBuffer = targetFrame->cmdBuffer;
  commands->resetCommandBuffer(cmdBuffer);

  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = nullptr;
  cmdBeginInfo.pInheritanceInfo = nullptr;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);

  // Получим время
  static auto timeGlobalStart = std::chrono::high_resolution_clock::now();
  auto timeGlobal = std::chrono::high_resolution_clock::now();
  float deltaGlobal = std::chrono::duration<double, std::milli>(timeGlobal - timeGlobalStart).count() / 1000.0f;

  static auto timeFrameStart = std::chrono::high_resolution_clock::now();
  auto timeFrame = std::chrono::high_resolution_clock::now();
  float deltaFrame = std::chrono::duration<double, std::milli>(timeFrame - timeFrameStart).count() / 1000.0f;
  timeFrameStart = timeFrame;

  // Получим объекты сцены
  auto object = scene->objects.front();
  object->setPosition({0, sin(deltaGlobal) / 10, 0});
  object->setRotation({0, deltaGlobal * 10.0f, 0});
  object->update();

  auto camera = scene->getCamera();
  camera->update(deltaFrame);

  // Обновим данные прохода рендера
  glm::float4x4 modelViewProj = camera->projectionMatrix * camera->viewMatrix * object->modelMatrix;
  geometryPass.updateUniformDescriptor(swapchainImageIndex, modelViewProj);

  // Укажем необходимые вершины для отрисовки
  GeometryPass::record_t data;
  data.cmd = cmdBuffer;
  data.imageIndex = swapchainImageIndex;
  data.indicesCount = object->model->verticesCount;
  data.indices = object->model->indexBuffer;
  data.vertices = object->model->vertexBuffer;
  geometryPass.record(data);

  if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
    throw std::runtime_error("ERROR: ailed to record command buffer!");
  }

  //=========================================================================
  // Установка команд рендера

  // Синхронизация кадров
  if (targetFrame->showing != VK_NULL_HANDLE)
    vkWaitForFences(core->device, 1, &targetFrame->showing, VK_TRUE, UINT64_MAX);
  targetFrame->showing = currentFrame->drawing;

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &targetFrame->cmdBuffer;

  // Синхронизация изображения
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &currentFrame->imageAvailable;  // Ждем, пока не получим изображение
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &currentFrame->imageRendered;  // Укажем, что команды рендера выставлены в очередь

  vkResetFences(core->device, 1, &currentFrame->drawing);
  if (vkQueueSubmit(core->graphicsQueue, 1, &submitInfo, currentFrame->drawing) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to submit draw command buffer!");

  //=========================================================================
  // Установка изображений показа

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &currentFrame->imageRendered;  // Ждем команды рендера
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &core->swapchain;
  presentInfo.pImageIndices = &swapchainImageIndex;

  result = vkQueuePresentKHR(core->presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->isResized) {
    resizeSwapchain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("ERROR: Failed to present swapchain image!");
  }

  frames->currentFrameIndex += 1;
  frames->currentFrameIndex %= core->swapchainImageCount;
}
