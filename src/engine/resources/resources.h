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
  VkCommandPool createCommandBufferPool(bool shortLived = false);
  void resetCommandBufferPool(VkCommandPool);    // Сбросит все буферы, сохранит все ресурсы
  void freeCommandBufferPool(VkCommandPool);     // Сбросит все буферы, освободит все ресурсы (вернёт их системе)
  void destroyCommandBufferPool(VkCommandPool);  // Уничтожет пул и все буферы, освободит все ресурсы (вернёт их системе)

  // Выделенные командные буферы
  VkCommandBuffer createCommandBuffer(VkCommandPool);
  void resetCommandBuffer(VkCommandBuffer);                   // Сбросит буфер, сохранит ресурсы
  void freeCommandBuffer(VkCommandBuffer);                    // Сбросит буфер, освободит ресурсы (вернёт их в пул)
  void destroyCommandBuffer(VkCommandPool, VkCommandBuffer);  // Уничтожет буфер, освободит ресурсы (вернёт их в пул)

  // Единовременные командные буферы
  VkCommandPool singleTimeCommandBufferPool;
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer);

  //=========================================================================
  // Ресурсы (Буферы и Изображения)

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
