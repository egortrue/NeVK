#pragma once

#include "core.h"

class Resources {
 private:
  Core* core;

 public:
  Resources(Core*);
  ~Resources();

  //=========================================================================
  // Командные буферы

  // Области выделения командных буферов
  VkCommandPool commandBufferPool;
  VkCommandPool createCommandBufferPool();
  void destroyCommandBufferPool(VkCommandPool);

  // Выделенные командные буферы
  VkCommandBuffer createCommandBuffer(VkCommandPool);
  void destroyCommandBuffer(VkCommandPool, VkCommandBuffer);

  //=========================================================================
  // Ресурсы (Буферы и Изображения)

  // Память физического устройства
  VkPhysicalDeviceMemoryProperties memoryProperties;
  uint32_t findMemoryTypeIndex(uint32_t type, VkMemoryPropertyFlags);

  // Области выделения ресурсов
  VkDescriptorPool descriptorBufferPool;
  VkDescriptorPool createDescriptorBufferPool();

  // Буферы - простейшее хранилище неструктурированных данных
  void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
  void destroyBuffer(VkBuffer, VkDeviceMemory);

  // Изображения - хранилище структурированных данных
  void createImage(uint32_t width, uint32_t height, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
  void destroyImage(VkImage, VkDeviceMemory);
};
