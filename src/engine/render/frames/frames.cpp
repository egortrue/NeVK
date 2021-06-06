#include "frames.h"

Frames::Frames(Core::Manager core) {
  this->core = core;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  currentFrameIndex = 0;
  for (uint32_t i = 0; i < core->swapchain.count; ++i) {
    auto frame = new frame_t;
    frame->cmdPool = core->commands->createCommandBufferPool();
    frame->cmdBuffer = core->commands->createCommandBuffer(frame->cmdPool);

    vkCreateFence(core->device, &fenceInfo, nullptr, &frame->drawing);
    vkCreateFence(core->device, &fenceInfo, nullptr, &frame->showing);
    fences.push_back(frame->drawing);
    fences.push_back(frame->showing);

    vkCreateSemaphore(core->device, &semaphoreInfo, nullptr, &frame->imageAvailable);
    vkCreateSemaphore(core->device, &semaphoreInfo, nullptr, &frame->imageRendered);
    semaphores.push_back(frame->imageAvailable);
    semaphores.push_back(frame->imageRendered);

    handlers.push_back(frame);
  }
}

Frames::~Frames() {
  for (auto frame : handlers)
    core->commands->destroyCommandBufferPool(frame->cmdPool);
  for (auto fence : fences)
    vkDestroyFence(core->device, fence, nullptr);
  for (auto semaphore : semaphores)
    vkDestroySemaphore(core->device, semaphore, nullptr);
}

Frames::Instance Frames::getFrame(uint32_t id) {
  return handlers[id];
}

Frames::Instance Frames::getCurrentFrame() {
  return handlers[currentFrameIndex];
}
