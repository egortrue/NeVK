#include "commands.h"

Commands::Commands(Core::Manager core, Resources::Manager resources) {
  this->core = core;
  this->resources = resources;
  this->singleTimeCommandBufferPool = createCommandBufferPool(true);
}

Commands::~Commands() {
  destroyCommandBufferPool(this->singleTimeCommandBufferPool);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands::changeImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  barrier.image = image;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("ERROR: Unsupported layout transition!");
  }

  vkCmdPipelineBarrier(
      cmd,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands::copyBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(cmd, src, dst, 1, &copyRegion);
}

void Commands::copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {
      width,
      height,
      1};

  vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands::copyDataToImage(void* src, VkImage dst, VkDeviceSize size, uint32_t width, uint32_t height) {
  // Создание временного буфера на устройстве
  VkBuffer tmpBuffer;
  VkDeviceMemory tmpBufferMemory;
  resources->createBuffer(
      size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      tmpBuffer, tmpBufferMemory);

  // Отображение память GPU на память CPU
  void* commonMemory = nullptr;
  vkMapMemory(core->device, tmpBufferMemory, 0, size, 0, &commonMemory);
  if (commonMemory == nullptr)
    throw std::runtime_error("ERROR: Failed to map memory on image!");

  // Скопируем данные в память устройства
  memcpy(commonMemory, src, static_cast<size_t>(size));
  vkUnmapMemory(core->device, tmpBufferMemory);

  // Копирование данных из буфера в изображение
  VkCommandBuffer cmd = this->beginSingleTimeCommands();
  this->changeImageLayout(cmd, dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  this->copyBufferToImage(cmd, tmpBuffer, dst, width, height);
  this->changeImageLayout(cmd, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  this->endSingleTimeCommands(cmd);

  // Уничтожим временный буфер
  resources->destroyBuffer(tmpBuffer, tmpBufferMemory);
}

void Commands::copyDataToBuffer(void* src, VkBuffer dst, VkDeviceSize size) {
  // Создание временного буфера на устройстве
  VkBuffer tmpBuffer;
  VkDeviceMemory tmpBufferMemory;
  resources->createBuffer(
      size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      tmpBuffer, tmpBufferMemory);

  // Отображение память GPU на память CPU
  void* commonMemory = nullptr;
  vkMapMemory(core->device, tmpBufferMemory, 0, size, 0, &commonMemory);
  if (commonMemory == nullptr)
    throw std::runtime_error("ERROR: Failed to map memory on buffer!");

  // Скопируем данные в память устройства
  memcpy(commonMemory, src, static_cast<size_t>(size));
  vkUnmapMemory(core->device, tmpBufferMemory);

  // Копирование данных в нужный буфер
  VkCommandBuffer cmd = this->beginSingleTimeCommands();
  this->copyBuffer(cmd, tmpBuffer, dst, size);
  this->endSingleTimeCommands(cmd);

  // Уничтожим временный буфер
  resources->destroyBuffer(tmpBuffer, tmpBufferMemory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
