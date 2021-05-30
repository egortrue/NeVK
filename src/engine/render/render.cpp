#include "render.h"

Render::Render(Window::Manager window, Core::Manager core,
               Resources::Manager resources, Commands::Manager commands,
               Scene::Manager scene) {
  this->window = window;
  this->core = core;
  this->resources = resources;
  this->commands = commands;
  this->scene = scene;

  initShaders();
  initFrames();
  initGeometry();
  initInterface();
}

Render::~Render() {
  destroyInterface();
  destroyGeometry();
  destroyFrames();
  destroyShaders();
}

void Render::reload() {
  vkDeviceWaitIdle(core->device);
  window->isResized = false;

  // Пересоздадим список показа
  core->destroySwapchain();
  core->createSwapchain();

  // Обновим все проходы рендера
  reloadGeometry();
  reloadInterface();

  // Обновим соотношение сторон для камеры
  auto camera = scene->getCamera();
  camera->projection.aspect = static_cast<float>(window->width) / static_cast<float>(window->height);
  camera->updateProjection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initShaders() {
  shaders = new Shaders(core);
}

void Render::destroyShaders() {
  if (shaders != nullptr)
    delete shaders;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initFrames() {
  frames = new Frames(core, commands);
}

void Render::destroyFrames() {
  if (frames != nullptr)
    delete frames;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initGeometry() {
  geometry = new Geometry();

  geometry->textureImageView = scene->objects.front()->texture->view;
  geometry->textureSampler = scene->objects.front()->texture->sampler;

  // Цель вывода прохода рендера
  geometry->targetImage.count = core->swapchain.count;
  geometry->targetImage.width = core->swapchain.extent.width;
  geometry->targetImage.height = core->swapchain.extent.height;
  geometry->targetImage.format = core->swapchain.format;
  geometry->targetImage.views = resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);

  Geometry::init_t data;
  data.core = core;
  data.resources = resources;
  data.commands = commands;
  data.shaders = shaders;
  data.shaderName = std::string("shaders/geometry.hlsl");
  geometry->init(data);
}

void Render::reloadGeometry() {
  resources->destroyImageViews(geometry->targetImage.views);
  geometry->targetImage.width = core->swapchain.extent.width;
  geometry->targetImage.height = core->swapchain.extent.height;
  geometry->targetImage.views = resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);
  geometry->resize();
}

void Render::destroyGeometry() {
  resources->destroyImageViews(geometry->targetImage.views);
  geometry->destroy();
  delete geometry;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initInterface() {
  interface = new GUI();

  // Цель вывода прохода рендера
  interface->targetImage.count = core->swapchain.count;
  interface->targetImage.width = core->swapchain.extent.width;
  interface->targetImage.height = core->swapchain.extent.height;
  interface->targetImage.format = core->swapchain.format;
  interface->targetImage.views = resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);

  GUI::init_t data;
  data.core = core;
  data.resources = resources;
  data.commands = commands;
  data.window = window;
  data.scene = scene;
  interface->init(data);
}

void Render::reloadInterface() {
  resources->destroyImageViews(interface->targetImage.views);
  interface->targetImage.width = core->swapchain.extent.width;
  interface->targetImage.height = core->swapchain.extent.height;
  interface->targetImage.views = resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);
  interface->resize();
}

void Render::destroyInterface() {
  resources->destroyImageViews(interface->targetImage.views);
  interface->destroy();
  delete interface;
}

GUI::Pass Render::getInterface() {
  return interface;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::draw() {
  auto currentFrame = frames->getCurrentFrame();
  vkWaitForFences(core->device, 1, &currentFrame->drawing, VK_TRUE, UINT64_MAX);

  //=========================================================================
  // Получение изображения из списка показа

  uint32_t swapchainImageIndex;
  VkResult result = vkAcquireNextImageKHR(core->device, core->swapchain.handler, UINT64_MAX, currentFrame->imageAvailable, VK_NULL_HANDLE, &swapchainImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    reload();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("ERROR: Failed to acquire swapchain image!");
  }

  auto targetFrame = frames->getFrame(swapchainImageIndex);

  //=========================================================================
  // Подготовка проходов рендера перед генерацией команд

  // Получим время
  static auto timeGlobalStart = std::chrono::high_resolution_clock::now();
  auto timeGlobal = std::chrono::high_resolution_clock::now();
  float deltaGlobal = std::chrono::duration<double, std::milli>(timeGlobal - timeGlobalStart).count() / 1000.0f;

  static auto timeFrameStart = std::chrono::high_resolution_clock::now();
  auto timeFrame = std::chrono::high_resolution_clock::now();
  float deltaFrame = std::chrono::duration<double, std::milli>(timeFrame - timeFrameStart).count() / 1000.0f;
  timeFrameStart = timeFrame;

  // Получим объекты сцены
  auto object = scene->objects[scene->currentObject];
  object->setPosition({object->transform.position.x, sin(deltaGlobal) / 10, object->transform.position.z});
  object->setRotation({object->transform.rotation.x, deltaGlobal * 10.0f, object->transform.rotation.z});
  object->update();

  auto camera = scene->getCamera();
  camera->update(deltaFrame);

  // Обновим данные прохода рендера
  geometry->uniform.modelViewProj = camera->projectionMatrix * camera->viewMatrix * object->modelMatrix;
  geometry->update(swapchainImageIndex);
  interface->update(swapchainImageIndex);

  //=========================================================================
  // Генерация команд рендера

  VkCommandBuffer cmd = targetFrame->cmdBuffer;
  commands->resetCommandBuffer(cmd);

  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = nullptr;
  cmdBeginInfo.pInheritanceInfo = nullptr;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  // Укажем необходимые вершины для отрисовки
  Geometry::record_t geoData;
  geoData.cmd = cmd;
  geoData.imageIndex = swapchainImageIndex;
  geoData.indicesCount = object->model->verticesCount;
  geoData.indices = object->model->indexBuffer;
  geoData.vertices = object->model->vertexBuffer;
  geometry->record(geoData);

  GUI::record_t guiData;
  guiData.cmd = cmd;
  guiData.imageIndex = swapchainImageIndex;
  interface->record(guiData);

  if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    throw std::runtime_error("ERROR: ailed to record command buffer!");

  //=========================================================================
  // Синхронизация кадров
  if (targetFrame->showing != VK_NULL_HANDLE)
    vkWaitForFences(core->device, 1, &targetFrame->showing, VK_TRUE, UINT64_MAX);
  targetFrame->showing = currentFrame->drawing;

  //=========================================================================
  // Установка команд рендера

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &targetFrame->cmdBuffer;

  // Синхронизация изображения
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &currentFrame->imageAvailable;  // ДО: Ждем, пока не получим изображение
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &currentFrame->imageRendered;  // ПОСЛЕ: Укажем, что команды рендера выставлены в очередь

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
  presentInfo.pSwapchains = &core->swapchain.handler;
  presentInfo.pImageIndices = &swapchainImageIndex;

  result = vkQueuePresentKHR(core->presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->isResized) {
    reload();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("ERROR: Failed to present swapchain image!");
  }

  frames->currentFrameIndex += 1;
  frames->currentFrameIndex %= core->swapchain.count;
}
