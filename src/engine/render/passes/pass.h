#pragma once

// Внутренние библиотеки
#include "core.h"
#include "shaders/shaders.h"

// Стандартные библиотеки
#include <array>
#include <string>
#include <vector>

class Pass {
 public:
  Core::Manager core;

  virtual void init();
  virtual void destroy();
  virtual void reload();
  virtual void resize();

  //=========================================================================
  // Обработчики конвейера и прохода рендера

 protected:
  struct {
    VkPipeline instance;
    VkPipelineLayout layout;
    VkPipelineCache cache;
    VkRenderPass pass;
  } pipeline;

  virtual void createPipeline() = 0;
  virtual void createRenderPass() = 0;

  //=========================================================================
  // Шейдеры - ядро любого прохода

 public:
  struct {
    std::string name;
    Shaders::Manager manager;
  } shader;

 protected:
  virtual void createShaderModules() = 0;

  //=========================================================================
  // Дескрипторы - обработчики подключаемых ресурсов

  struct {
    std::vector<VkDescriptorSet> sets;
    std::vector<VkDescriptorSetLayout> layouts;
  } descriptor;

  virtual void createDescriptorLayouts() = 0;  // Описание используемых ресурсов
  virtual void createDescriptorSets() = 0;     // Выделение памяти под ресурсы
  virtual void updateDescriptorSets() = 0;     // Подключение данных к выделенной памяти

  //=========================================================================
};
