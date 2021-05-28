#pragma once

// Внутренние библиотеки
#include "core.h"
#include "resources.h"

class Commands {
 public:
  typedef Commands* Manager;

 private:
  Core::Manager core;
  Resources::Manager resources;

 public:
  Commands(Core::Manager, Resources::Manager);
  ~Commands();

  //=========================================================================
  // Области выделения командных буферов

  VkCommandPool createCommandBufferPool(bool shortLived = false);
  void resetCommandBufferPool(VkCommandPool);    // Сбросит все буферы, сохранит все ресурсы
  void freeCommandBufferPool(VkCommandPool);     // Сбросит все буферы, освободит все ресурсы (вернёт их системе)
  void destroyCommandBufferPool(VkCommandPool);  // Уничтожет пул и все буферы, освободит все ресурсы (вернёт их системе)

  //=========================================================================
  // Выделенные командные буферы

  VkCommandBuffer createCommandBuffer(VkCommandPool);
  void resetCommandBuffer(VkCommandBuffer);                   // Сбросит буфер, сохранит ресурсы
  void freeCommandBuffer(VkCommandBuffer);                    // Сбросит буфер, освободит ресурсы (вернёт их в пул)
  void destroyCommandBuffer(VkCommandPool, VkCommandBuffer);  // Уничтожет буфер, освободит ресурсы (вернёт их в пул)

  //=========================================================================
  // Единовременные командные буферы

  VkCommandPool singleTimeCommandBufferPool;
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer);

  //=========================================================================
  // Общие операции над ресурсами

  // Копирование в памяти устройства
  void copyBuffer(VkCommandBuffer, VkBuffer src, VkBuffer dst, VkDeviceSize size);
  void copyBufferToImage(VkCommandBuffer, VkBuffer src, VkImage dst, uint32_t width, uint32_t height);

  // Перевод из памяти приложения в память устройства
  void copyDataToBuffer(void* src, VkBuffer dst, VkDeviceSize size);
  void copyDataToImage(void* src, VkImage dst, VkDeviceSize size, uint32_t width, uint32_t height);

  void changeImageLayout(VkCommandBuffer, VkImage, VkImageLayout oldLayout, VkImageLayout newLayout);
};
