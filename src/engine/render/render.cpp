#include "render.h"

Render::Render(Window::Manager window, Core::Manager core, Scene::Manager scene) {
  this->window = window;
  this->core = core;
  this->scene = scene;

  initShaders();
  initFrames();
  initGeometry();
  initPostProcess();
  initInterface();
}

Render::~Render() {
  destroyInterface();
  destroyPostProcess();
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
  reinitPostProcess();
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
  postprocess.TAA->reload();
  interface.pass->reload();
}

GUI::Pass Render::getInterface() {
  return interface.pass;
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
  frames = new Frames(core);
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
  geometry.pass->scene = scene;
  geometry.pass->shader.manager = shaders;
  geometry.pass->shader.name = std::string("shaders/geometry.hlsl");

  // Дескрипторы прохода рендера
  scene->getTextures()->getViews(geometry.pass->textureImageViews);
  geometry.pass->textureSampler = core->resources->createImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);

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
  core->resources->destroyImageSampler(geometry.pass->textureSampler);
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
    core->resources->createImage(
        geometry.data.width, geometry.data.height, geometry.data.format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        geometry.data.images[i], geometry.data.memory[i]);
    geometry.data.views[i] = core->resources->createImageView(
        geometry.data.images[i],
        core->swapchain.format,
        VK_IMAGE_ASPECT_COLOR_BIT);
    core->commands->changeImageLayout(
        nullptr,
        geometry.data.images[i],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}

void Render::destroyGeometryData() {
  for (uint32_t i = 0; i < geometry.data.images.size(); ++i) {
    core->resources->destroyImageView(geometry.data.views[i]);
    core->resources->destroyImage(geometry.data.images[i], geometry.data.memory[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initPostProcess() {
  auto origin = new Fullscreen();
  auto TAA = new Fullscreen();

  postprocess.origin = origin;
  postprocess.TAA = TAA;

  std::array<Fullscreen::Pass, 2> passes = {
      postprocess.origin,
      postprocess.TAA,
  };

  for (auto& pass : passes) {
    // Основные параметры
    pass->core = core;
    pass->shader.manager = shaders;

    // Дескрипторы прохода рендера
    pass->colorImageViews = geometry.data.views;
    pass->colorImageSampler = core->resources->createImageSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    // Указание изображений, в которые будет идти результат
    pass->target.width = core->swapchain.extent.width;
    pass->target.height = core->swapchain.extent.height;
    pass->target.format = core->swapchain.format;
    pass->target.views = core->resources->createImageViews(
        core->swapchain.images,
        core->swapchain.format,
        VK_IMAGE_ASPECT_COLOR_BIT);
  }

  origin->shader.name = std::string("shaders/fullscreen.hlsl");
  origin->init();

  TAA->shader.name = std::string("shaders/taa.hlsl");
  TAA->init();
}

void Render::destroyPostProcess() {
  std::array<Fullscreen::Pass, 2> passes = {
      postprocess.origin,
      postprocess.TAA,
  };

  for (auto pass : passes) {
    core->resources->destroyImageSampler(pass->colorImageSampler);
    core->resources->destroyImageViews(pass->target.views);
    pass->destroy();
    delete pass;
  }
}

void Render::reinitPostProcess() {
  std::array<Fullscreen::Pass, 2> passes = {
      postprocess.origin,
      postprocess.TAA,
  };

  for (auto pass : passes) {
    pass->colorImageViews = geometry.data.views;
    pass->target.width = core->swapchain.extent.width;
    pass->target.height = core->swapchain.extent.height;
    pass->target.format = core->swapchain.format;

    core->resources->destroyImageViews(pass->target.views);
    pass->target.views = core->resources->createImageViews(
        core->swapchain.images,
        core->swapchain.format,
        VK_IMAGE_ASPECT_COLOR_BIT);

    pass->resize();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Render::initInterface() {
  interface.pass = new GUI();

  // Основные параметры
  interface.pass->core = core;
  interface.pass->window = window;
  interface.pass->scene = scene;

  // Цель вывода прохода рендера
  interface.pass->target.width = core->swapchain.extent.width;
  interface.pass->target.height = core->swapchain.extent.height;
  interface.pass->target.format = core->swapchain.format;
  interface.pass->target.views = core->resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);

  interface.pass->init();
}

void Render::reinitInterface() {
  interface.pass->target.width = core->swapchain.extent.width;
  interface.pass->target.height = core->swapchain.extent.height;
  interface.pass->target.format = core->swapchain.format;

  core->resources->destroyImageViews(interface.pass->target.views);
  interface.pass->target.views = core->resources->createImageViews(
      core->swapchain.images,
      core->swapchain.format,
      VK_IMAGE_ASPECT_COLOR_BIT);
  interface.pass->resize();
}

void Render::destroyInterface() {
  core->resources->destroyImageViews(interface.pass->target.views);
  interface.pass->destroy();
  delete interface.pass;
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
  core->commands->resetCommandBuffer(cmd);

  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.pNext = nullptr;
  cmdBeginInfo.pInheritanceInfo = nullptr;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  //=========================================================================
  // Генерация команд рендера

  geometry.pass->record(swapchainImageIndex, cmd);

  core->commands->changeImageLayout(
      cmd,
      geometry.data.images[swapchainImageIndex],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  if (!interface.pass->options.taaON)
    postprocess.origin->record(swapchainImageIndex, cmd);
  else
    postprocess.TAA->record(swapchainImageIndex, cmd);
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
