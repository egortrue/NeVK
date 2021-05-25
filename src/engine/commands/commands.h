#pragma once

// Внутренние библиотеки
#include "core.h"

typedef class Commands* CommandsManager;

class Commands {
 private:
  CoreManager core;

 public:
  Commands(CoreManager);
  ~Commands();

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

  // Общие операции над ресурсами
  void changeImageLayout(VkCommandBuffer, VkImage, VkImageLayout oldLayout, VkImageLayout newLayout);
  void copyBufferToImage(VkCommandBuffer, VkBuffer, VkImage, uint32_t width, uint32_t height);
};
