#pragma once

// Внутренние библиотеки
#include "core.h"

typedef class Resources* ResourcesManager;

class Resources {
 private:
  Core* core;

 public:
  Resources(Core*);
  ~Resources();

  //=========================================================================
  // Характеристики физического устройства

  VkPhysicalDeviceMemoryProperties memoryProperties;
  uint32_t findMemoryTypeIndex(uint32_t type, VkMemoryPropertyFlags);
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  //=========================================================================
  // Области выделения ресурсов

  VkDescriptorPool descriptorPool;
  VkDescriptorPool createDescriptorPool();
  void destroyDescriptorPool(VkDescriptorPool);

  //=========================================================================
  // Буферы - простейшее хранилище неструктурированных данных

  void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
  void destroyBuffer(VkBuffer, VkDeviceMemory);

  //=========================================================================
  // Изображения - хранилище структурированных данных

  void createImage(uint32_t width, uint32_t height, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
  void destroyImage(VkImage, VkDeviceMemory);

  VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags);
  void destroyImageView(VkImageView);

  std::vector<VkImageView> createImageViews(std::vector<VkImage>&, VkFormat, VkImageAspectFlags);
  void destroyImageViews(std::vector<VkImageView>&);

  //=========================================================================
};
