#pragma once

#include "core.h"

class Resources {
 private:
  Core* core;

 public:
  Resources(Core*);
  ~Resources();

  // Память физического устройства
  VkPhysicalDeviceMemoryProperties memoryProperties;
  uint32_t findMemoryTypeIndex(uint32_t type, VkMemoryPropertyFlags);

  // Области выделения ресурсов
  VkDescriptorPool descriptorPool;
  VkDescriptorPool createDescriptorPool();
  void destroyDescriptorPool(VkDescriptorPool);

  // Буферы - простейшее хранилище неструктурированных данных
  void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
  void destroyBuffer(VkBuffer, VkDeviceMemory);

  // Изображения - хранилище структурированных данных
  void createImage(uint32_t width, uint32_t height, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
  void destroyImage(VkImage, VkDeviceMemory);
};
