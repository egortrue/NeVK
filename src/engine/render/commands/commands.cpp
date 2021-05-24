#include "commands.h"

Commands::Commands(Core* core) {
  this->core = core;
  this->singleTimeCommandBufferPool = createCommandBufferPool(true);
};

Commands::~Commands() {
  destroyCommandBufferPool(this->singleTimeCommandBufferPool);
}

VkCommandPool Commands::createCommandBufferPool(bool shortLived) {
  VkCommandPoolCreateInfo cmdPoolInfo{};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

  // Все буферы из этого пула будут помещаться в очереди конкретного семейства
  cmdPoolInfo.queueFamilyIndex = core->queueFamily.graphicsFamily.value();

  if (shortLived) {
    // Каждый буфер из этого пула будет не долговечным - может быть сброшен только весь пул
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  } else {
    // Каждый буфер из этого пула будет долговечным - может быть сброшен или перезаписан
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  }

  VkCommandPool cmdPool;
  if (vkCreateCommandPool(core->device, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create graphics command pool!");

  return cmdPool;
}

void Commands::resetCommandBufferPool(VkCommandPool cmdPool) {
  vkResetCommandPool(core->device, cmdPool, 0);
}

void Commands::freeCommandBufferPool(VkCommandPool cmdPool) {
  vkResetCommandPool(core->device, cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

void Commands::destroyCommandBufferPool(VkCommandPool cmdPool) {
  vkDestroyCommandPool(core->device, cmdPool, nullptr);
}

VkCommandBuffer Commands::createCommandBuffer(VkCommandPool cmdPool) {
  VkCommandBufferAllocateInfo cmdBufferInfo{};
  cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdBufferInfo.commandPool = cmdPool;
  cmdBufferInfo.commandBufferCount = 1;

  VkCommandBuffer cmdBuffer;
  if (vkAllocateCommandBuffers(core->device, &cmdBufferInfo, &cmdBuffer) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to allocate command buffer!");

  return cmdBuffer;
}

void Commands::resetCommandBuffer(VkCommandBuffer cmdBuffer) {
  vkResetCommandBuffer(cmdBuffer, 0);
}

void Commands::freeCommandBuffer(VkCommandBuffer cmdBuffer) {
  vkResetCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void Commands::destroyCommandBuffer(VkCommandPool cmdPool, VkCommandBuffer cmdBuffer) {
  vkFreeCommandBuffers(core->device, cmdPool, 1, &cmdBuffer);
}

VkCommandBuffer Commands::beginSingleTimeCommands() {
  VkCommandBuffer cmdBuffer = createCommandBuffer(singleTimeCommandBufferPool);

  VkCommandBufferBeginInfo cmdBufferBeginInfo{};
  cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  // Буфер будет записан, один раз выполнен и затем уничтожен
  cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);
  return cmdBuffer;
}

void Commands::endSingleTimeCommands(VkCommandBuffer cmdBuffer) {
  vkEndCommandBuffer(cmdBuffer);

  VkSubmitInfo cmdBufferSubmitInfo{};
  cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  cmdBufferSubmitInfo.commandBufferCount = 1;
  cmdBufferSubmitInfo.pCommandBuffers = &cmdBuffer;

  vkQueueSubmit(core->graphicsQueue, 1, &cmdBufferSubmitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(core->graphicsQueue);

  destroyCommandBuffer(singleTimeCommandBufferPool, cmdBuffer);
}
