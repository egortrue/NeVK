#include "resources.h"

Resources::Resources(Core* core) {
  vkGetPhysicalDeviceMemoryProperties(core->physicalDevice, &memoryProperties);
  this->core = core;
  this->descriptorPool = createDescriptorPool();
};

Resources::~Resources() {
  destroyDescriptorPool(this->descriptorPool);
}

uint32_t Resources::findMemoryTypeIndex(uint32_t type, VkMemoryPropertyFlags properties) {
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    if ((type & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;

  throw std::runtime_error("ERROR: Failed to find suitable memory type!");
}

VkDescriptorPool Resources::createDescriptorPool() {
  VkDescriptorPoolSize descPoolSizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo descPoolInfo{};
  descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  descPoolInfo.poolSizeCount = sizeof(descPoolSizes) / sizeof(VkDescriptorPoolSize);
  descPoolInfo.pPoolSizes = descPoolSizes;
  descPoolInfo.maxSets = 1000 * descPoolInfo.poolSizeCount;

  VkDescriptorPool descPool;
  if (vkCreateDescriptorPool(core->device, &descPoolInfo, nullptr, &descPool) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create descriptor pool!");

  return descPool;
}

void Resources::destroyDescriptorPool(VkDescriptorPool descPool) {
  vkDestroyDescriptorPool(core->device, descPool, nullptr);
}

void Resources::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
  //===================================================
  // Создание буфера

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;

  // Буфер будет использоваться только одной очередью
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferInfo.queueFamilyIndexCount = 0;
  bufferInfo.pQueueFamilyIndices = nullptr;

  if (vkCreateBuffer(core->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create buffer!");

  //===================================================
  // Выделение памяти, на которую будет опираться буфер

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(core->device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(core->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to allocate buffer memory!");

  vkBindBufferMemory(core->device, buffer, bufferMemory, 0);
}

void Resources::destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory) {
  vkDestroyBuffer(core->device, buffer, nullptr);
  vkFreeMemory(core->device, bufferMemory, nullptr);
}

void Resources::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
  //===================================================
  // Создание изображения

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  // Уровень сжатия изображений (mipmapping)
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  // Изображение будет использоваться только одной очередью
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.queueFamilyIndexCount = 0;
  imageInfo.pQueueFamilyIndices = nullptr;

  imageInfo.format = format;  // Режим отображения данных
  imageInfo.usage = usage;    // Режим использования
  imageInfo.tiling = tiling;  // Режим видимости (VK_IMAGE_TILING_OPTIMAL = GPU | VK_IMAGE_TILING_LINEAR = GPU + CPU)

  // Размер изображения
  // uint32_t limit = core->physicalDeviceProperties.limits.maxImageDimension2D; // Лимит размера
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;

  if (vkCreateImage(core->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create image!");

  //===================================================
  // Выделение памяти, на которую будет опираться изображение

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(core->device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(core->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to allocate image memory!");

  vkBindImageMemory(core->device, image, imageMemory, 0);
}

void Resources::destroyImage(VkImage image, VkDeviceMemory imageMemory) {
  vkDestroyImage(core->device, image, nullptr);
  vkFreeMemory(core->device, imageMemory, nullptr);
}