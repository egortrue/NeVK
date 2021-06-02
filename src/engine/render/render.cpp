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
  initFullscreen();
  initInterface();
}

Render::~Render() {
  destroyInterface();
  destroyFullscreen();
  destroyGeometry();
  destroyFrames();
  destroyShaders();
}

void Render::reloadSwapchain() {
  vkDeviceWaitIdle(core->device);
  window->isResized = false;

  // Пересоздадим список показа
  core->destroySwapchain();
  core->createSwapchain();

  // Обновим данные всех проходов рендера
  reinitGeometry();
  reinitFullscreen();
  reinitInterface();

  // Обновим соотношение сторон для камеры
  auto camera = scene->getCamera();
  camera->projection.aspect = static_cast<float>(window->width) / static_cast<float>(window->height);
  camera->updateProjection();
}

void Render::reloadShaders() {
  vkDeviceWaitIdle(core->device);

  // Перезагрузим все проходы рендера
  geometry.pass->reload();
  fullscreen.pass->reload();
  interface.pass->reload();
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
  geometry.pass = new Geometry();
  createGeometryData();

  // Основные параметры
  geometry.pass->core = core;
  geometry.pass->resources = resources;
  geometry.pass->commands = commands;
  geometry.pass->scene = scene;
  geometry.pass->shader.manager = shaders;
  geometry.pass->shader.name = std::string("shaders/geometry.hlsl");

  // Дескрипторы прохода рендера
  for (auto object : scene->objects)
    geometry.pass->textureImageViews.push_back(object->texture->view);
  geometry.pass->textureSampler = resources->createImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);

  // Цель вывода прохода рендера
  geometry.pass->target.format = geometry.data.format;
  geometry.pass->target.width = geometry.data.width;
  geometry.pass->target.height = geometry.data.height;
  geometry.pass->target.views = geometry.data.views;

  geometry.pass->init();
}

void Render::reinitGeometry() {
  destroyGeometryData();
  createGeometryData();
  geometry.pass->target.format = geometry.data.format;
  geometry.pass->target.width = geometry.data.width;
  geometry.pass->target.height = geometry.data.height;
  geometry.pass->target.views = geometry.data.views;
  geometry.pass->resize();
}

void Render::destroyGeometry() {
  resources->destroyImageSampler(geometry.pass->textureSampler);
  destroyGeometryData();
  geometry.pass->destroy();
  delete geometry.pass;
}

void Render::createGeometryData() {
  geometry.data.width = core->swapchain.extent.width;
  geometry.data.height = core->swapchain.extent.height;
  geometry.data.format = core->swapchain.format;

  uint32_t count = core->swapchain.count;
  geometry.data.images.resize(count);
  geometry.data.memory.resize(count);
  geometry.data.views.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    resources->createImage(
        geometry.data.width, geometry.data.height, geometry.data.format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        geometry.data.images[i], geometry.data.memory[i]);
    geometry.data.views[i] = resources->createImageView(
        geometry.data.images[i],
        core->swapchain.format,
        VK_IMAGE_ASPECT_COLOR_BIT);
    commands->changeImageLayout(
        nullptr,
        geometry.data.images[i],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}

void Render::destroyGeometryData() {
  for (uint32_t i = 0; i < geometry.data.images.size(); ++i) {
    resources->destroyImageView(geometry.data.views[i]);
    resources->destroyImage(geometry.data.images[i], geometry.data.memory[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initFullscreen() {
  fullscreen.pass = new Fullscreen();
  createFullscreenData();

  // Основные параметры
  fullscreen.pass->core = core;
  fullscreen.pass->resources = resources;
  fullscreen.pass->commands = commands;
  fullscreen.pass->shader.manager = shaders;
  fullscreen.pass->shader.name = std::string("shaders/fullscreen.hlsl");

  // Дескрипторы прохода рендера
  fullscreen.pass->colorImageViews = geometry.data.views;
  fullscreen.pass->colorImageSampler = resources->createImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);

  // Указание изображений, в которые будет идти результат
  fullscreen.pass->target.width = fullscreen.data.width;
  fullscreen.pass->target.height = fullscreen.data.height;
  fullscreen.pass->target.format = fullscreen.data.format;
  fullscreen.pass->target.views = fullscreen.data.views;

  fullscreen.pass->init();
}

void Render::destroyFullscreen() {
  resources->destroyImageSampler(fullscreen.pass->colorImageSampler);
  destroyFullscreenData();
  fullscreen.pass->destroy();
  delete fullscreen.pass;
}

void Render::reinitFullscreen() {
  destroyFullscreenData();
  createFullscreenData();
  fullscreen.pass->colorImageViews = geometry.data.views;
  fullscreen.pass->target.width = fullscreen.data.width;
  fullscreen.pass->target.height = fullscreen.data.height;
  fullscreen.pass->target.format = fullscreen.data.format;
  fullscreen.pass->target.views = fullscreen.data.views;
  fullscreen.pass->resize();
}

void Render::createFullscreenData() {
  fullscreen.data.format = core->swapchain.format;
  fullscreen.data.width = core->swapchain.extent.width;
  fullscreen.data.height = core->swapchain.extent.height;
  fullscreen.data.views = resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);
}

void Render::destroyFullscreenData() {
  resources->destroyImageViews(fullscreen.data.views);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initInterface() {
  interface.pass = new GUI();
  createInterfaceData();

  // Основные параметры
  interface.pass->core = core;
  interface.pass->resources = resources;
  interface.pass->commands = commands;
  interface.pass->window = window;
  interface.pass->scene = scene;

  // Цель вывода прохода рендера
  interface.pass->target.width = interface.data.width;
  interface.pass->target.height = interface.data.height;
  interface.pass->target.format = interface.data.format;
  interface.pass->target.views = interface.data.views;

  interface.pass->init();
}

void Render::reinitInterface() {
  destroyInterfaceData();
  createInterfaceData();
  interface.pass->target.width = interface.data.width;
  interface.pass->target.height = interface.data.height;
  interface.pass->target.format = interface.data.format;
  interface.pass->target.views = interface.data.views;
  interface.pass->resize();
}

void Render::destroyInterface() {
  destroyInterfaceData();
  interface.pass->destroy();
  delete interface.pass;
}

void Render::createInterfaceData() {
  interface.data.format = core->swapchain.format;
  interface.data.width = core->swapchain.extent.width;
  interface.data.height = core->swapchain.extent.height;
  interface.data.views = resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);
}

void Render::destroyInterfaceData() {
  resources->destroyImageViews(interface.data.views);
}

GUI::Pass Render::getInterface() {
  return interface.pass;
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
    reloadSwapchain();
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
  object->setRotation({object->transform.rotation.x, deltaGlobal * 100.0f, object->transform.rotation.z});
  object->update();

  auto camera = scene->getCamera();
  camera->update(deltaFrame);

  // Обновим данные прохода рендера
  geometry.pass->uniform.cameraView = camera->viewMatrix;
  geometry.pass->uniform.cameraProjection = camera->projectionMatrix;
  geometry.pass->update(swapchainImageIndex);

  interface.pass->update(swapchainImageIndex);

  //=========================================================================
  // Подготовка буфера команд

  VkCommandBuffer cmd = targetFrame->cmdBuffer;
  commands->resetCommandBuffer(cmd);

  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = nullptr;
  cmdBeginInfo.pInheritanceInfo = nullptr;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  //=========================================================================
  // Генерация команд рендера

  geometry.pass->record(swapchainImageIndex, cmd);

  commands->changeImageLayout(
      cmd,
      geometry.data.images[swapchainImageIndex],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  fullscreen.pass->record(swapchainImageIndex, cmd);
  interface.pass->record(swapchainImageIndex, cmd);

  //=========================================================================
  // Завершение буфера команд

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
    reloadSwapchain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("ERROR: Failed to present swapchain image!");
  }

  frames->currentFrameIndex += 1;
  frames->currentFrameIndex %= core->swapchain.count;
}
