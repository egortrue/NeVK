#pragma once

#include "core.h"

class Commands {
 private:
  Core* core;

 public:
  Commands(Core*);
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
};
